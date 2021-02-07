#include <engge/Entities/BlinkState.hpp>
#include <engge/System/Locator.hpp>
#include <engge/Util/RandomNumberGenerator.hpp>
#include "Util/Util.hpp"

namespace ng {
BlinkState::BlinkState(Costume &costume) : _costume(costume) {
}

void BlinkState::setRate(float min, float max) {
  _min = min;
  _max = max;
  if (min == 0 && max == 0) {
    // blinking is disabled
    _state = -1;
  } else {
    _state = 0;
    _value = ngf::TimeSpan::seconds(Locator<RandomNumberGenerator>::get().generateFloat(_min, _max));
  }
  _elapsed = ngf::TimeSpan::seconds(0);
  _costume.setLayerVisible("blink", false);
}

void BlinkState::update(ngf::TimeSpan elapsed) {
  if (_state == ObjectStateConstants::CLOSED) {
    // wait to blink
    _elapsed += elapsed;
    if (_elapsed > _value) {
      _state = 1;
      _costume.setLayerVisible("blink", true);
      _elapsed = ngf::TimeSpan::seconds(0);
    }
  } else if (_state == ObjectStateConstants::OPEN) {
    // wait time the eyes are closed
    _elapsed += elapsed;
    if (_elapsed > ngf::TimeSpan::seconds(0.2)) {
      _costume.setLayerVisible("blink", false);
      _value = ngf::TimeSpan::seconds(Locator<RandomNumberGenerator>::get().generateFloat(_min, _max));
      _elapsed = ngf::TimeSpan::seconds(0);
      _state = 0;
    }
  }
}
}
