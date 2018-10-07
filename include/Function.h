#pragma once
#include <string>
#include "SFML/System.hpp"
#include "NonCopyable.h"
#include "Interpolations.h"

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
      : _time(time), _function([]() {})
  {
  }

  ~TimeFunction() override = default;

  bool isElapsed() override
  {
    auto isElapsed = _clock.getElapsedTime() > _time;
    if (isElapsed)
      _function();
    return isElapsed;
  }

  void callWhenElapsed(std::function<void()> function) { _function = function; }
};

template <typename Value>
class ChangeProperty : public TimeFunction
{
public:
  ChangeProperty(std::function<Value()> get, std::function<void(const Value &)> set, Value destination, const sf::Time &time, std::function<float(float)> anim = Interpolations::linear)
      : TimeFunction(time),
        _get(get),
        _set(set),
        _destination(destination),
        _init(get()),
        _delta(_destination - _init),
        _current(_init),
        _anim(anim)
  {
  }

  void operator()() override
  {
    _set(_current);
    if (!isElapsed())
    {
      auto t = _clock.getElapsedTime().asSeconds() / _time.asSeconds();
      auto f = _anim(t);
      _current = _init + f * _delta;
    }
  }

private:
  std::function<Value()> _get;
  std::function<void(const Value &)> _set;
  Value _destination;
  Value _init;
  Value _delta;
  Value _current;
  std::function<float(float)> _anim;
};
} // namespace gg
