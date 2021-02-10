#pragma once
#include <cstdint>
#include <ngf/System/TimeSpan.h>

namespace ng {
class Costume;

class BlinkState final {
public:
  explicit BlinkState(Costume &costume);

  void setRate(float min, float max);
  void update(ngf::TimeSpan elapsed);

private:
  Costume &m_costume;
  float m_min{0};
  float m_max{0};
  ngf::TimeSpan m_value;
  int32_t m_state{-1};
  ngf::TimeSpan m_elapsed;
};
}
