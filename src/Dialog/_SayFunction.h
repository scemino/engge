#pragma once
#include "Function.h"
#include "Actor.h"

namespace ng
{
class _SayFunction : public Function
{
public:
  _SayFunction(Actor &actor, const int id)
      : _actor(actor), _id(id), _done(false)
  {
  }

  bool isElapsed() override { return _done && _actor.isTalkingIdDone(_id); }

  void operator()(const sf::Time &elapsed) override
  {
    if (_done)
      return;
    _actor.say(_id);
    _done = true;
  }

private:
  Actor &_actor;
  const int _id;
  bool _done;
};
} // namespace ng
