#include "Engine/Camera.hpp"
#include "Engine/Cutscene.hpp"
#include "Engine/Engine.hpp"
#include "System/Logger.hpp"
#include "Scripting/ScriptEngine.hpp"

namespace ng {
Cutscene::Cutscene(Engine &engine,
                   HSQUIRRELVM v,
                   HSQOBJECT thread,
                   HSQOBJECT closureObj,
                   HSQOBJECT closureCutsceneOverrideObj,
                   HSQOBJECT envObj)
    : _engine(engine), _v(v), _threadCutscene(thread), _state(0), _closureObj(closureObj),
      _closureCutsceneOverrideObj(closureCutsceneOverrideObj), _envObj(envObj) {
  _engineVm = engine.getVm();
  _hasCutsceneOverride = !sq_isnull(_closureCutsceneOverrideObj);
  _inputState = _engine.getInputState();
  trace("Cutscene with inputState {}", _inputState);
  _engine.setInputActive(false);
  _engine.setInputVerbs(false);

  sq_addref(_engineVm, &_threadCutscene);
  sq_addref(_engineVm, &_closureObj);
  sq_addref(_engineVm, &_closureCutsceneOverrideObj);
  sq_addref(_engineVm, &_envObj);
}

Cutscene::~Cutscene() {
  sq_release(_engineVm, &_threadCutscene);
  sq_release(_engineVm, &_closureObj);
  sq_release(_engineVm, &_closureCutsceneOverrideObj);
  sq_release(_engineVm, &_envObj);
}

HSQUIRRELVM Cutscene::getThread() const {
  return _threadCutscene._unVal.pThread;
}

bool Cutscene::isElapsed() { return _state == 5; }

void Cutscene::cutsceneOverride() {
  if (_hasCutsceneOverride && _state == 1)
    _state = 2;
}

void Cutscene::operator()(const sf::Time &) {
  switch (_state) {
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
  _state = 1;
  trace("start cutscene: {}", _id);
  sq_pushobject(_threadCutscene._unVal.pThread, _closureObj);
  sq_pushobject(_threadCutscene._unVal.pThread, _envObj);
  if (SQ_FAILED(sq_call(_threadCutscene._unVal.pThread, 1, SQFalse, SQTrue))) {
    error("Couldn't call cutscene");
  }
}

void Cutscene::checkEndCutscene() {
  if (ThreadBase::isStopped()) {
    _state = 4;
    trace("end cutscene: {}", _id);
  }
}

void Cutscene::doCutsceneOverride() {
  if (_hasCutsceneOverride) {
    _state = 3;
    trace("start cutsceneOverride: {}", _id);
    sq_pushobject(_threadCutscene._unVal.pThread, _closureCutsceneOverrideObj);
    sq_pushobject(_threadCutscene._unVal.pThread, _envObj);
    if (SQ_FAILED(sq_call(_threadCutscene._unVal.pThread, 1, SQFalse, SQTrue))) {
      error("Couldn't call cutsceneOverride");
    }
    return;
  }
  _state = 4;
}

void Cutscene::checkEndCutsceneOverride() {
  if (ThreadBase::isStopped()) {
    _state = 4;
    trace("end checkEndCutsceneOverride: {}", _id);
  }
}

void Cutscene::endCutscene() {
  _state = 5;
  trace("End cutscene {} with inputState {}", _id, _inputState);
  _engine.setInputState(_inputState);
  _engine.follow(_engine.getCurrentActor());
  auto pThread = ScriptEngine::getThreadFromVm(_v);
  if (pThread)
    pThread->resume();
  pThread = ScriptEngine::getThreadFromId(_id);
  if (pThread)
    pThread->stop();
}

bool Cutscene::isStopped() const {
  return _state == 5;
}

} // namespace ng