#pragma once
#include "Function.h"

namespace ng
{
class _ExecuteCodeFunction : public Function
{
public:
  _ExecuteCodeFunction(Engine &engine, const std::string &code)
      : _engine(engine), _code(code), _done(false)
  {
  }

  bool isElapsed() override { return _done; }

  void operator()(const sf::Time &elapsed) override
  {
    if (_done)
      return;
    _engine.execute(_code);
    _done = true;
  }

private:
  Engine &_engine;
  std::string _code;
  bool _done;
};
} // namespace ng
