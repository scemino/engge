#include "VerbExecuteFunction.hpp"
#include <engge/Engine/Verb.hpp>
#include <engge/Entities/Actor.hpp>
#include <engge/Entities/Object.hpp>
#include <engge/Scripting/ScriptEngine.hpp>

namespace ng {
VerbExecuteFunction::VerbExecuteFunction(Engine &engine,
                                         Actor &actor,
                                         Entity *pObject1,
                                         Entity *pObject2,
                                         const Verb *pVerb)
    : m_engine(engine), m_pVerb(pVerb), m_pObject1(pObject1), m_pObject2(pObject2), m_actor(actor) {
}

bool VerbExecuteFunction::isElapsed() { return m_done; }

bool VerbExecuteFunction::needToExecuteVerb() {
  auto executeVerb = true;
  if (m_pVerb->id == VerbConstants::VERB_GIVE && m_pObject2) {
    auto *pActor2 = dynamic_cast<Actor *>(m_pObject2);
    if (!pActor2)
      pActor2 = Entity::getActor(m_pObject2);
    bool selectable = true;
    ScriptEngine::rawGet(pActor2, "selectable", selectable);
    executeVerb = !selectable;
  }
  return executeVerb;
}

bool VerbExecuteFunction::callVerb() {
  if (m_pVerb->id == VerbConstants::VERB_GIVE && ScriptEngine::rawExists(&m_actor, m_pVerb->func.data())) {
    auto handled = false;
    if (ScriptEngine::rawCallFunc(handled, &m_actor, m_pVerb->func.data(), m_pObject1) && handled)
      return true;
  }

  if (!ScriptEngine::rawExists(m_pObject1, m_pVerb->func.data()))
    return false;

  auto count = ScriptEngine::getParameterCount(m_pObject1, m_pVerb->func.data());
  if (count == 2) {
    return ScriptEngine::objCall(m_pObject1, m_pVerb->func.data(), m_pObject2);
  }
  return ScriptEngine::objCall(m_pObject1, m_pVerb->func.data());
}

void VerbExecuteFunction::operator()(const ngf::TimeSpan &) {
  m_done = true;

  if (needToExecuteVerb()) {
    if (callVerb()) {
      onPickup();
      return;
    }
  }

  if (callVerbGive())
    return;

  if (callVerbDefault(m_pObject1))
    return;

  callDefaultObjectVerb();
}

bool VerbExecuteFunction::callVerbGive() {
  if (m_pVerb->id != VerbConstants::VERB_GIVE)
    return false;

  auto *pActor2 = dynamic_cast<Actor *>(m_pObject2);
  if (!pActor2)
    pActor2 = Entity::getActor(m_pObject2);
  ScriptEngine::call("objectGive", m_pObject1, &m_actor, pActor2);

  auto *pObject = dynamic_cast<Object *>(m_pObject1);
  m_actor.giveTo(pObject, pActor2);
  return true;
}

void VerbExecuteFunction::onPickup() {
  if (m_pVerb->id != VerbConstants::VERB_PICKUP)
    return;

  ScriptEngine::call("onPickup", m_pObject1, &m_actor);
}

void VerbExecuteFunction::callDefaultObjectVerb() {
  auto &obj = m_engine.getDefaultObject();
  ScriptEngine::objCall(obj, m_pVerb->func.data(), m_pObject1, m_pObject2);
}

bool VerbExecuteFunction::callVerbDefault(Entity *pEntity) {
  return ScriptEngine::objCall(pEntity, "verbDefault");
}
}