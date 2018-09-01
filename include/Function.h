#pragma once
#include <string>
#include "SFML/System.hpp"
#include "NonCopyable.h"

namespace gg
{
class Function : public NonCopyable
{
public:
  virtual bool isElapsed() { return true; }
  virtual void operator()() {}
  virtual ~Function() {}
};

class TimeFunction : public Function
{
protected:
  sf::Clock _clock;
  sf::Time _time;

public:
  TimeFunction(const sf::Time &time)
      : _time(time)
  {
  }
  virtual ~TimeFunction() {}

  bool isElapsed() override
  {
    return _clock.getElapsedTime() > _time;
  }
};
} // namespace gg
