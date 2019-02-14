#pragma once
#include "Function.h"

namespace ng
{
class _ShutupFunction : public Function
{
public:
  _ShutupFunction(Engine &engine)
      : _engine(engine), _done(false)
  {
  }

  bool isElapsed() { return _done; }

  virtual void operator()()
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
  bool _done;
};
} // namespace ng
