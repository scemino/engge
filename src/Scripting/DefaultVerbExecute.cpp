#include "DefaultVerbExecute.hpp"
#include <engge/Engine/Engine.hpp>
#include <engge/Engine/Sentence.hpp>
#include <engge/Scripting/ScriptEngine.hpp>
#include "Scripting/ActorWalk.hpp"
#include "Scripting/PostWalk.hpp"
#include "Scripting/ReachAnim.hpp"
#include "Scripting/SetDefaultVerb.hpp"
#include "Scripting/VerbExecuteFunction.hpp"
#include "../Util/Util.hpp"

namespace ng {
DefaultVerbExecute::DefaultVerbExecute(Engine &engine) : m_engine(engine) {}

void DefaultVerbExecute::execute(const Verb *pVerb, Entity *pObject1, Entity *pObject2) {
  getVerb(pObject1, pVerb);
  if (!pVerb)
    return;

  // TODO: do it earlier
  if (pVerb->id == VerbConstants::VERB_TALKTO) {
    pObject2 = m_engine.getCurrentActor();
  }

  if (pVerb->id == VerbConstants::VERB_USE && !pObject2 && useFlags(pObject1))
    return;

  if (pVerb->id == VerbConstants::VERB_GIVE && !pObject2) {
    m_engine.setUseFlag(UseFlag::GiveTo, pObject1);
    return;
  }

  if (callObjectOrActorPreWalk(pVerb->id, pObject1, pObject2))
    return;

  auto pActor = m_engine.getCurrentActor();
  if (!pActor)
    return;

  auto sentence = std::make_unique<Sentence>();
  auto pObjToWalkTo =
      (pVerb->id == VerbConstants::VERB_USE || pVerb->id == VerbConstants::VERB_GIVE) ? pObject2 : pObject1;
  if (needsToWalkTo(pVerb->id, pObjToWalkTo)) {
    auto walk = std::make_unique<ActorWalk>(m_engine, *pActor, pObjToWalkTo, sentence.get(), pVerb);
    sentence->push_back(std::move(walk));
    auto postWalk = std::make_unique<PostWalk>(*sentence, pObject1, pObject2, pVerb->id);
    sentence->push_back(std::move(postWalk));
  }

  if (needsReachAnim(pVerb->id)) {
    if (ScriptEngine::exists(pObject1, pVerb->func.data())) {
      auto reach = std::make_unique<ReachAnim>(*pActor, pObject1);
      sentence->push_back(std::move(reach));
    }
  }

  auto verb = std::make_unique<VerbExecuteFunction>(m_engine, *pActor, pObject1, pObject2, pVerb);
  sentence->push_back(std::move(verb));
  auto setDefaultVerb = std::make_unique<SetDefaultVerb>(m_engine);
  sentence->push_back(std::move(setDefaultVerb));
  m_engine.setSentence(std::move(sentence));
}

bool DefaultVerbExecute::needsToWalkTo(int verbId, Entity *pObj) {
  if (verbId == VerbConstants::VERB_LOOKAT && isFarLook(pObj))
    return false;
  return pObj && !pObj->isInventoryObject();
}

bool DefaultVerbExecute::needsReachAnim(int verbId) {
  return verbId == VerbConstants::VERB_PICKUP || verbId == VerbConstants::VERB_OPEN
      || verbId == VerbConstants::VERB_CLOSE || verbId == VerbConstants::VERB_PUSH
      || verbId == VerbConstants::VERB_PULL
      || verbId == VerbConstants::VERB_USE;
}

bool DefaultVerbExecute::isFarLook(const Entity *pEntity) {
  auto flags = pEntity->getFlags();
  return flags & 8u;
}

bool DefaultVerbExecute::useFlags(Entity *pObject) {
  auto flags = pObject->getFlags();
  UseFlag useFlag;
  if (flags & ObjectFlagConstants::USE_WITH) {
    useFlag = UseFlag::UseWith;
  } else if (flags & ObjectFlagConstants::USE_ON) {
    useFlag = UseFlag::UseOn;
  } else if (flags & ObjectFlagConstants::USE_IN) {
    useFlag = UseFlag::UseIn;
  } else {
    return false;
  }
  m_engine.setUseFlag(useFlag, pObject);
  return true;
}

bool DefaultVerbExecute::callObjectOrActorPreWalk(int verb, Entity *pObj1, Entity *pObj2) {
  auto handled = false;
  auto pActor = m_engine.getCurrentActor();
  if (ScriptEngine::rawExists(pActor, "actorPreWalk")) {
    ScriptEngine::callFunc(handled, pActor, "actorPreWalk", verb, pObj1, pObj2);
    if (handled)
      return true;
  }

  auto *pObj = dynamic_cast<Object *>(pObj1);
  auto functionName = pObj ? "objectPreWalk" : "actorPreWalk";
  if (ScriptEngine::rawExists(pObj1, functionName)) {
    ScriptEngine::callFunc(handled, pObj1, functionName, verb, pObj1, pObj2);
  }
  return handled;
}

void DefaultVerbExecute::getVerb(Entity *pObj, const Verb *&pVerb) {
  int verb;
  if (pVerb) {
    verb = pVerb->id;
    pVerb = m_engine.getHud().getVerb(verb);
    return;
  }
  auto defaultVerb = pObj->getDefaultVerb(VerbConstants::VERB_LOOKAT);
  if (!defaultVerb)
    return;
  verb = defaultVerb;
  pVerb = m_engine.getHud().getVerb(verb);
}
}