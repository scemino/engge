#pragma once
#include "Function.h"

namespace ng
{
class _PauseFunction : public Function
{
  public:
    explicit _PauseFunction(sf::Time time)
        : _time(time), _done(false)
    {
    }

    bool isElapsed() override { return _done && _clock.getElapsedTime() > _time; }

    void operator()(const sf::Time &elapsed) override
    {
        if (_done)
            return;
        _clock.restart();
        _done = true;
    }

  private:
    sf::Clock _clock;
    const sf::Time _time;
    bool _done;
};
} // namespace ng
