#include "ActorWalk.hpp"
#include <engge/Engine/Engine.hpp>
#include <engge/Entities/Actor.hpp>
#include <engge/Entities/Costume.hpp>
#include <engge/Entities/Entity.hpp>
#include <engge/Engine/Sentence.hpp>
#include <engge/Engine/Verb.hpp>
#include <engge/Scripting/ScriptEngine.hpp>
#include "Util/Util.hpp"

namespace ng {
ActorWalk::ActorWalk(Engine &engine, Actor &actor, Entity *pEntity, Sentence *pSentence, const Verb *pVerb)
    : m_engine(engine), m_actor(actor), m_pEntity(pEntity), m_pSentence(pSentence), m_pVerb(pVerb) {
  auto pActor = dynamic_cast<Actor *>(pEntity);
  auto pos = pEntity->getPosition();
  auto usePos = pEntity->getUsePosition().value_or(glm::vec2());
  auto facing = getFacing(pEntity);
  auto destination = pActor ? pos : pos + usePos;
  m_path = m_actor.walkTo(destination, facing);
  int useDist = 0;
  ScriptEngine::rawGet(pEntity, "useDist", useDist);
  useDist = std::max(useDist, 4);
  m_isDestination = distance(m_path.back(), destination) <= static_cast<float>(useDist);
}

Facing ActorWalk::getFacing(const Entity *pEntity) {
  const auto pActor = dynamic_cast<const Actor *>(pEntity);
  if (pActor) {
    return getOppositeFacing(pActor->getCostume().getFacing());
  }
  const auto pObj = dynamic_cast<const Object *>(pEntity);
  return toFacing(pObj->getUseDirection());
}

bool ActorWalk::isElapsed() {
  if (m_done)
    return true;
  auto isWalking = m_actor.isWalking();
  if (isWalking)
    return false;

  m_done = true;
  auto pos = m_actor.getPosition();
  if (m_isDestination)
    return true;

  // to talk to someone, there's no need to reach him
  if (m_pVerb->id == VerbConstants::VERB_TALKTO)
    return true;

  m_pSentence->stop();
  if (m_path.back() != pos)
    return true;

  if (ScriptEngine::rawExists(m_pEntity, "verbCantReach")) {
    ScriptEngine::objCall(m_pEntity, "verbCantReach");
    return true;
  }
  callDefaultObjectVerb();
  return true;
}

void ActorWalk::callDefaultObjectVerb() {
  auto &obj = m_engine.getDefaultObject();
  ScriptEngine::objCall(obj, "verbCantReach", m_pEntity, nullptr);
}
}