#include <engge/Engine/Camera.hpp>
#include <engge/Engine/Cutscene.hpp>
#include <engge/Engine/Engine.hpp>
#include <engge/System/Logger.hpp>
#include <engge/Scripting/ScriptEngine.hpp>
#include <engge/Engine/EntityManager.hpp>

namespace ng {
Cutscene::Cutscene(Engine &engine,
                   HSQUIRRELVM v,
                   HSQOBJECT thread,
                   HSQOBJECT closureObj,
                   HSQOBJECT closureCutsceneOverrideObj,
                   HSQOBJECT envObj)
    : m_engine(engine), m_v(v), m_threadCutscene(thread), m_state(0), m_closureObj(closureObj),
      m_closureCutsceneOverrideObj(closureCutsceneOverrideObj), m_envObj(envObj) {
  auto engineVm = ScriptEngine::getVm();
  m_hasCutsceneOverride = !sq_isnull(m_closureCutsceneOverrideObj);
  m_inputState = m_engine.getInputState();
  trace("Cutscene with inputState {}", m_inputState);
  m_engine.setInputActive(false);
  m_engine.setInputVerbs(false);

  sq_addref(engineVm, &m_threadCutscene);
  sq_addref(engineVm, &m_closureObj);
  sq_addref(engineVm, &m_closureCutsceneOverrideObj);
  sq_addref(engineVm, &m_envObj);
}

Cutscene::~Cutscene() {
  auto engineVm = ScriptEngine::getVm();
  sq_release(engineVm, &m_threadCutscene);
  sq_release(engineVm, &m_closureObj);
  sq_release(engineVm, &m_closureCutsceneOverrideObj);
  sq_release(engineVm, &m_envObj);
}

HSQUIRRELVM Cutscene::getThread() const {
  return m_threadCutscene._unVal.pThread;
}

std::string Cutscene::getName() const {
  return "cutscene";
}

bool Cutscene::isElapsed() { return m_state == 5; }

void Cutscene::cutsceneOverride() {
  if (m_hasCutsceneOverride && m_state == 1)
    m_state = 2;
}

void Cutscene::operator()(const ngf::TimeSpan &) {
  switch (m_state) {
  case 0:trace("startCutscene");
    startCutscene();
    break;
  case 1:checkEndCutscene();
    break;
  case 2:trace("doCutsceneOverride");
    doCutsceneOverride();
    break;
  case 3:trace("checkEndCutsceneOverride");
    checkEndCutsceneOverride();
    break;
  case 4:trace("endCutscene");
    endCutscene();
    break;
  case 5:return;
  }
}

void Cutscene::startCutscene() {
  m_state = 1;
  trace("start cutscene: {}", m_id);
  sq_pushobject(m_threadCutscene._unVal.pThread, m_closureObj);
  sq_pushobject(m_threadCutscene._unVal.pThread, m_envObj);
  if (SQ_FAILED(sq_call(m_threadCutscene._unVal.pThread, 1, SQFalse, SQTrue))) {
    error("Couldn't call cutscene");
  }
}

void Cutscene::checkEndCutscene() {
  if (ThreadBase::isStopped()) {
    m_state = 4;
    trace("end cutscene: {}", m_id);
  }
}

void Cutscene::doCutsceneOverride() {
  if (m_hasCutsceneOverride) {
    m_state = 3;
    trace("start cutsceneOverride: {}", m_id);
    sq_pushobject(m_threadCutscene._unVal.pThread, m_closureCutsceneOverrideObj);
    sq_pushobject(m_threadCutscene._unVal.pThread, m_envObj);
    if (SQ_FAILED(sq_call(m_threadCutscene._unVal.pThread, 1, SQFalse, SQTrue))) {
      error("Couldn't call cutsceneOverride");
    }
    return;
  }
  m_state = 4;
}

void Cutscene::checkEndCutsceneOverride() {
  if (ThreadBase::isStopped()) {
    m_state = 4;
    trace("end checkEndCutsceneOverride: {}", m_id);
  }
}

void Cutscene::endCutscene() {
  m_state = 5;
  trace("End cutscene {} with inputState {}", m_id, m_inputState);
  m_engine.setInputState(m_inputState);
  m_engine.follow(m_engine.getCurrentActor());
  ScriptEngine::call("onCutsceneEnded");
  auto pThread = EntityManager::getThreadFromVm(m_v);
  if (pThread)
    pThread->resume();
  pThread = EntityManager::getThreadFromId(m_id);
  if (pThread)
    pThread->stop();
}

bool Cutscene::isStopped() const {
  return m_state == 5;
}

} // namespace ng