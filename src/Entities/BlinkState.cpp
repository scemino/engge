#include <engge/Entities/BlinkState.hpp>
#include <engge/System/Locator.hpp>
#include <engge/Util/RandomNumberGenerator.hpp>
#include "Util/Util.hpp"

namespace ng {
BlinkState::BlinkState(Costume &costume) : m_costume(costume) {
}

void BlinkState::setRate(float min, float max) {
  m_min = min;
  m_max = max;
  if (min == 0 && max == 0) {
    // blinking is disabled
    m_state = -1;
  } else {
    m_state = ObjectStateConstants::CLOSED;
    m_value = ngf::TimeSpan::seconds(Locator<RandomNumberGenerator>::get().generateFloat(m_min, m_max));
  }
  m_elapsed = ngf::TimeSpan::seconds(0);
  m_costume.setLayerVisible("blink", false);
}

void BlinkState::update(ngf::TimeSpan elapsed) {
  if (m_state == ObjectStateConstants::CLOSED) {
    // wait to blink
    m_elapsed += elapsed;
    if (m_elapsed > m_value) {
      m_state = ObjectStateConstants::OPEN;
      m_costume.setLayerVisible("blink", true);
      m_elapsed = ngf::TimeSpan::seconds(0);
    }
  } else if (m_state == ObjectStateConstants::OPEN) {
    // wait time the eyes are closed
    m_elapsed += elapsed;
    if (m_elapsed > ngf::TimeSpan::seconds(0.2)) {
      m_costume.setLayerVisible("blink", false);
      m_value = ngf::TimeSpan::seconds(Locator<RandomNumberGenerator>::get().generateFloat(m_min, m_max));
      m_elapsed = ngf::TimeSpan::seconds(0);
      m_state = ObjectStateConstants::CLOSED;
    }
  }
}
}
