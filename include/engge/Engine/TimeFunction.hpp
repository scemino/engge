#pragma once
#include <ngf/System/TimeSpan.h>
#include <engge/Engine/Function.hpp>

namespace ng {
class TimeFunction : public Function {
public:
  explicit TimeFunction(const ngf::TimeSpan &time);
  ~TimeFunction() override;

  void operator()(const ngf::TimeSpan &elapsed) override;
  [[nodiscard]] ngf::TimeSpan getElapsed() const;

  bool isElapsed() override;

  virtual void onElapsed();

protected:
  ngf::TimeSpan m_elapsed;
  ngf::TimeSpan m_time;
  bool m_done{false};
};
}