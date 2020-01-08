#pragma once
#include "Function.h"

namespace ng
{
class _WaitWhileFunction: public Function
{
  public:
    explicit _WaitWhileFunction(Engine& engine, const std::string& condition)
        : _engine(engine), _condition(condition)
    {
    }

    bool isElapsed() override { return _done; }

    void operator()(const sf::Time &elapsed) override
    {
        if (_done)
            return;
        _done = !_engine.executeCondition(_condition);
    }

  private:
    Engine& _engine;
    std::string _condition;
    bool _done{false};
};
}