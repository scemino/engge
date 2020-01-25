#pragma once
#include "Engine/Function.hpp"

namespace ng
{
class _ShutupFunction : public Function
{
public:
  explicit _ShutupFunction(Engine &engine)
      : _engine(engine)
  {
  }

  bool isElapsed() override { return _done; }

  void operator()(const sf::Time &) override
  {
    if (_done)
      return;
    for (auto &actor : _engine.getActors())
    {
      actor->stopTalking();
    }
    _done = true;
  }

private:
  Engine &_engine;
  bool _done{false};
};
} // namespace ng
