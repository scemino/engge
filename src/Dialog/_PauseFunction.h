#pragma once
#include "Engine/Function.h"

namespace ng
{
class _PauseFunction : public Function
{
  public:
    explicit _PauseFunction(sf::Time time)
        : _time(time)
    {
    }

    bool isElapsed() override { return _done && _clock.getElapsedTime() > _time; }

    void operator()(const sf::Time &) override
    {
        if (_done)
            return;
        _clock.restart();
        _done = true;
    }

  private:
    sf::Clock _clock;
    const sf::Time _time;
    bool _done{false};
};
} // namespace ng
