#include "ReachAnim.hpp"
#include <engge/Entities/Actor.hpp>
#include <engge/Entities/Entity.hpp>
#include <engge/Entities/Object.hpp>

namespace ng {
ReachAnim::ReachAnim(Actor &actor, Entity *obj)
    : m_actor(actor) {
  auto flags = obj->getFlags();
  if (flags & ObjectFlagConstants::REACH_HIGH) {
    m_reaching = Reaching::High;
  } else if (flags & ObjectFlagConstants::REACH_MED) {
    m_reaching = Reaching::Medium;
  } else if (flags & ObjectFlagConstants::REACH_LOW) {
    m_reaching = Reaching::Low;
  } else {
    m_state = 2;
  }
}

bool ReachAnim::isElapsed() { return m_state == 3; }

void ReachAnim::playReachAnim() {
  m_actor.getCostume().setReachState(m_reaching);
  m_actor.getCostume().getAnimControl().play(false);
}

void ReachAnim::operator()(const ngf::TimeSpan &elapsed) {
  switch (m_state) {
  case 0:playReachAnim();
    m_state = 1;
    break;
  case 1:m_elapsed += elapsed;
    if (m_elapsed.getTotalSeconds() > 0.330)
      m_state = 2;
    break;
  case 2:m_actor.getCostume().setStandState();
    m_state = 3;
    break;
  }
}
}