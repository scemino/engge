#pragma once
#include "Engine/Function.h"

#include <utility>
#include "Entities/Actor/Actor.h"

namespace ng
{
class _SayFunction : public Function
{
public:
  _SayFunction(Actor &actor, std::string  text)
      : _actor(actor), _text(std::move(text))
  {
  }

  bool isElapsed() override { return _done && !_actor.isTalking(); }

  void operator()(const sf::Time &) override
  {
    if (_done)
      return;
    _actor.say(_text);
    _done = true;
  }

private:
  Actor &_actor;
  std::string _text;
  bool _done{false};
};
} // namespace ng
