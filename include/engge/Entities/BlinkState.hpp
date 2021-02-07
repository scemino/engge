#pragma once
#include <cstdint>
#include <ngf/System/TimeSpan.h>

namespace ng {
class Costume;

class BlinkState {
public:
  explicit BlinkState(Costume &costume);

  void setRate(float min, float max);
  void update(ngf::TimeSpan elapsed);

private:
  Costume &_costume;
  double _min{0};
  double _max{0};
  ngf::TimeSpan _value;
  int32_t _state{-1};
  ngf::TimeSpan _elapsed;
};
}
