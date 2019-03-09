#pragma once
#include <string>
#include <functional>
#include "SFML/System.hpp"
#include "NonCopyable.h"
#include "Interpolations.h"

namespace ng
{
class Function : public NonCopyable
{
public:
  virtual bool isElapsed() { return true; }
  virtual void operator()(const sf::Time &elapsed) {}
  virtual ~Function() = default;
};

class TimeFunction : public Function
{
protected:
  sf::Time _elapsed;
  sf::Time _time;
  std::function<void()> _function;

public:
  explicit TimeFunction(const sf::Time &time)
      : _time(time), _function([]() {})
  {
  }

  ~TimeFunction() override = default;

  void operator()(const sf::Time &elapsed) override
  {
    _elapsed += elapsed;
  }

  bool isElapsed() override
  {
    auto isElapsed = _elapsed > _time;
    if (isElapsed)
    {
      onElapsed();
      _function();
    }
    return isElapsed;
  }

  virtual void onElapsed()
  {
  }

  void callWhenElapsed(std::function<void()> function) { _function = function; }
};

template <typename Value>
class ChangeProperty : public TimeFunction
{
public:
  ChangeProperty(std::function<Value()> get, std::function<void(const Value &)> set, Value destination, const sf::Time &time, std::function<float(float)> anim = Interpolations::linear, bool isLooping = false)
      : TimeFunction(time),
        _get(get),
        _set(set),
        _destination(destination),
        _init(get()),
        _delta(_destination - _init),
        _current(_init),
        _anim(anim),
        _isLooping(isLooping)
  {
  }

  void operator()(const sf::Time &elapsed) override
  {
    TimeFunction::operator()(elapsed);
    _set(_current);
    if (!isElapsed())
    {
      auto t = _elapsed.asSeconds() / _time.asSeconds();
      auto f = _anim(t);
      _current = _init + f * _delta;
    }
  }

  bool isElapsed() override
  {
    if (!_isLooping)
      return TimeFunction::isElapsed();
    return false;
  }

  void onElapsed() override
  {
    _set(_destination);
  }

private:
  std::function<Value()> _get;
  std::function<void(const Value &)> _set;
  Value _destination;
  Value _init;
  Value _delta;
  Value _current;
  std::function<float(float)> _anim;
  bool _isLooping;
};
} // namespace ng
