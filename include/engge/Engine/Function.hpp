#pragma once
#include <string>
#include <functional>
#include "engge/System/NonCopyable.hpp"
#include "Interpolations.hpp"
#include <ngf/System/TimeSpan.h>

namespace ng {
class Function : public NonCopyable {
public:
  virtual bool isElapsed() { return true; }
  virtual void operator()(const ngf::TimeSpan &) {}
  virtual ~Function() = default;
};

class TimeFunction : public Function {
protected:
  ngf::TimeSpan _elapsed;
  ngf::TimeSpan _time;
  bool _done{false};

public:
  explicit TimeFunction(const ngf::TimeSpan &time)
      : _time(time) {
  }

  ~TimeFunction() override = default;

  void operator()(const ngf::TimeSpan &elapsed) override {
    _elapsed += elapsed;
  }

  [[nodiscard]] ngf::TimeSpan getElapsed() const { return _elapsed; }

  bool isElapsed() override {
    auto isElapsed = _elapsed > _time;
    if (isElapsed && !_done) {
      _done = true;
      onElapsed();
    }
    return isElapsed;
  }

  virtual void onElapsed() {
  }
};

template<typename Value>
class ChangeProperty : public TimeFunction {
public:
  ChangeProperty(std::function<Value()> get,
                 std::function<void(const Value &)> set,
                 Value destination,
                 const ngf::TimeSpan &time,
                 InterpolationMethod method = InterpolationMethod::Linear)
      : TimeFunction(time),
        _get(get),
        _set(set),
        _destination(destination),
        _init(get()),
        _delta(_destination - _init),
        _current(_init) {
    _anim = InterpolationHelper::getInterpolationMethod(method);
    _isLooping =
        ((method & InterpolationMethod::Looping) | (method & InterpolationMethod::Swing)) != InterpolationMethod::None;
    _isSwing = (method & InterpolationMethod::Swing) != InterpolationMethod::None;
  }

  void operator()(const ngf::TimeSpan &elapsed) override {
    TimeFunction::operator()(elapsed);
    _set(_current);
    if (!isElapsed()) {
      auto t = _elapsed.getTotalSeconds() / _time.getTotalSeconds();
      auto f = _dirForward ? _anim(t) : 1.f - _anim(t);
      _current = _init + f * _delta;
      if (_elapsed >= _time && _isLooping) {
        _elapsed = ngf::TimeSpan::seconds(_elapsed.getTotalSeconds() - _time.getTotalSeconds());
        _dirForward = !_dirForward;
      }
    }
  }

  bool isElapsed() override {
    if (!_isLooping)
      return TimeFunction::isElapsed();
    return false;
  }

  void onElapsed() override {
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
  bool _isLooping{false};
  bool _isSwing{false};
  bool _dirForward{true};
};
} // namespace ng
