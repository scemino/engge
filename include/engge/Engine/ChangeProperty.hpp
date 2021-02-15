#pragma once
#include <functional>
#include <ngf/System/TimeSpan.h>
#include <engge/Engine/TimeFunction.hpp>
#include <engge/Engine/Interpolations.hpp>

namespace ng {
template<typename Value>
class ChangeProperty : public TimeFunction {
public:
  ChangeProperty(std::function<Value()> get,
                 std::function<void(const Value &)> set,
                 Value destination,
                 const ngf::TimeSpan &time,
                 InterpolationMethod method = InterpolationMethod::Linear)
      : TimeFunction(time),
        m_get(get),
        m_set(set),
        m_destination(destination),
        m_init(get()),
        m_delta(m_destination - m_init),
        m_current(m_init) {
    m_anim = InterpolationHelper::getInterpolationMethod(method);
    m_isLooping =
        ((method & InterpolationMethod::Looping) | (method & InterpolationMethod::Swing)) != InterpolationMethod::None;
    m_isSwing = (method & InterpolationMethod::Swing) != InterpolationMethod::None;
  }

  void operator()(const ngf::TimeSpan &elapsed) override {
    TimeFunction::operator()(elapsed);
    m_set(m_current);
    if (!isElapsed()) {
      auto t = m_elapsed.getTotalSeconds() / m_time.getTotalSeconds();
      auto f = m_dirForward ? m_anim(t) : 1.f - m_anim(t);
      m_current = m_init + f * m_delta;
      if (m_elapsed >= m_time && m_isLooping) {
        m_elapsed = ngf::TimeSpan::seconds(m_elapsed.getTotalSeconds() - m_time.getTotalSeconds());
        m_dirForward = !m_dirForward;
      }
    }
  }

  bool isElapsed() override {
    if (!m_isLooping)
      return TimeFunction::isElapsed();
    return false;
  }

  void onElapsed() override {
    m_set(m_destination);
  }

private:
  std::function<Value()> m_get;
  std::function<void(const Value &)> m_set;
  Value m_destination;
  Value m_init;
  Value m_delta;
  Value m_current;
  std::function<float(float)> m_anim;
  bool m_isLooping{false};
  bool m_isSwing{false};
  bool m_dirForward{true};
};
}