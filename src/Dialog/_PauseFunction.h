#pragma once
#include "Function.h"

namespace ng
{
class _PauseFunction : public Function
{
  public:
    _PauseFunction(sf::Time time)
        : _time(time), _done(false)
    {
    }

    bool isElapsed() { return _done && _clock.getElapsedTime() > _time; }

    virtual void operator()()
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
