#include <utility>

#pragma once
#include "Engine/Function.h"

namespace ng
{
class _ExecuteCodeFunction : public Function
{
public:
  _ExecuteCodeFunction(Engine &engine, std::string code)
      : _engine(engine), _code(std::move(code))
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
  bool _done{false};
};
} // namespace ng
