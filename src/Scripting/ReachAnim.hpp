#pragma once
#include <engge/Engine/Function.hpp>
#include <engge/Entities/Costume.hpp>
#include <ngf/System/TimeSpan.h>

namespace ng {
class Actor;
class Entity;

class ReachAnim final : public Function {
public:
  ReachAnim(Actor &actor, Entity *obj);

private:
  bool isElapsed() final;
  void playReachAnim();
  void operator()(const ngf::TimeSpan &elapsed) final;

private:
  int32_t m_state{0};
  Actor &m_actor;
  Reaching m_reaching{Reaching::Medium};
  ngf::TimeSpan m_elapsed;
};
}