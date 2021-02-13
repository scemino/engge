#include "engge/Room/RoomScaling.hpp"

namespace ng {

void RoomScaling::setTrigger(const std::string &trigger) {
  m_trigger = trigger;
}

float RoomScaling::getScaling(float yPos) const {
  if (m_scalings.empty())
    return 1.0f;
  for (size_t i = 0; i < m_scalings.size(); i++) {
    const auto &scaling = m_scalings[i];
    if (yPos < scaling.yPos) {
      if (i == 0)
        return m_scalings[i].scale;
      auto prevScaling = m_scalings[i - 1];
      auto dY = scaling.yPos - prevScaling.yPos;
      auto dScale = scaling.scale - prevScaling.scale;
      auto p = (yPos - prevScaling.yPos) / dY;
      auto scale = prevScaling.scale + (p * dScale);
      return scale;
    }
  }
  return m_scalings[m_scalings.size() - 1].scale;
}

std::vector<Scaling> &RoomScaling::getScalings() {
  return m_scalings;
}

const std::string &RoomScaling::getName() const {
  return m_trigger;
}

} // namespace ng