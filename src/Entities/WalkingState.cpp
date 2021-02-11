#include "WalkingState.hpp"
#include <engge/Entities/Actor.hpp>
#include <engge/Entities/Costume.hpp>
#include <engge/Scripting/ScriptEngine.hpp>
#include <engge/System/Logger.hpp>

namespace ng {
void WalkingState::setActor(Actor *pActor) { m_pActor = pActor; }

void WalkingState::setDestination(const std::vector <glm::vec2> &path, std::optional <Facing> facing) {
  m_path = path;
  m_facing = facing;
  m_path.erase(m_path.begin());
  m_pActor->getCostume().setFacing(getFacing());
  m_pActor->getCostume().setWalkState();
  m_isWalking = true;
  m_init = m_pActor->getPosition();
  m_elapsed = ngf::TimeSpan::seconds(0);
  trace("{} go to : {},{}", m_pActor->getName(), m_path[0].x, m_path[0].y);
}

void WalkingState::stop() {
  m_isWalking = false;
  m_pActor->getCostume().setStandState();
  if (ScriptEngine::rawExists(m_pActor, "postWalking")) {
    ScriptEngine::objCall(m_pActor, "postWalking");
  }
}

Facing WalkingState::getFacing() {
  auto pos = m_pActor->getPosition();
  auto dx = m_path[0].x - pos.x;
  auto dy = m_path[0].y - pos.y;
  if (fabs(dx) > fabs(dy))
    return (dx > 0) ? Facing::FACE_RIGHT : Facing::FACE_LEFT;
  return (dy < 0) ? Facing::FACE_FRONT : Facing::FACE_BACK;
}

void WalkingState::update(const ngf::TimeSpan &elapsed) {
  if (!m_isWalking)
    return;

  // update actor position
  m_elapsed += elapsed;
  auto delta = (m_path[0] - m_init);
  auto vSpeed = m_pActor->getWalkSpeed();
  glm::vec2 vDuration;
  vDuration.x = std::abs(delta.x) / vSpeed.x;
  vDuration.y = std::abs(delta.y) / vSpeed.y;
  auto maxDuration = std::max(vDuration.x, vDuration.y);
  auto factor = (2.f * m_elapsed.getTotalSeconds()) / maxDuration;
  auto end = factor >= 1.f;
  auto newPos = end ? m_path[0] : (m_init + factor * delta);
  m_pActor->setPosition(newPos);
  if (!end)
    return;

  // if the actor is arrived to destination
  m_path.erase(m_path.begin());
  // check if we have an other destination
  if (!m_path.empty()) {
    m_pActor->getCostume().setFacing(getFacing());
    m_pActor->getCostume().setWalkState();
    m_init = newPos;
    m_elapsed = ngf::TimeSpan::seconds(0);
    trace("{} go to : {},{}", m_pActor->getName(), m_path[0].x, m_path[0].y);
    return;
  }

  // the actor is arrived to the final destination
  stop();
  trace("Play anim stand");
  if (m_facing.has_value()) {
    m_pActor->getCostume().setFacing(m_facing.value());
  }
  if (ScriptEngine::rawExists(m_pActor, "actorArrived")) {
    ScriptEngine::rawCall(m_pActor, "actorArrived");
  }
}
}