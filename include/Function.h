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
  virtual ~Function() = default;
};

class TimeFunction : public Function
{
protected:
  sf::Clock _clock;
  sf::Time _time;
  std::function<void()> _function;

public:
  explicit TimeFunction(const sf::Time &time)
      : _time(time), _function([](){})
  {
  }

  ~TimeFunction() override = default;

  bool isElapsed() override
  {
    auto isElapsed = _clock.getElapsedTime() > _time;
    if(isElapsed) _function();
    return isElapsed;
  }

  void callWhenElapsed(std::function<void()> function) { _function = function; }
};
} // namespace gg
