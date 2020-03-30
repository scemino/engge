#include "Room/RoomScaling.hpp"

namespace ng {

void RoomScaling::setTrigger(const std::string &trigger) {
  _trigger = trigger;
}

float RoomScaling::getScaling(float yPos) const {
  if (_scalings.empty())
    return 1.0f;
  for (size_t i = 0; i < _scalings.size(); i++) {
    const auto &scaling = _scalings[i];
    if (yPos < scaling.yPos) {
      if (i == 0)
        return _scalings[i].scale;
      auto prevScaling = _scalings[i - 1];
      auto dY = scaling.yPos - prevScaling.yPos;
      auto dScale = scaling.scale - prevScaling.scale;
      auto p = (yPos - prevScaling.yPos) / dY;
      auto scale = prevScaling.scale + (p * dScale);
      return scale;
    }
  }
  return _scalings[_scalings.size() - 1].scale;
}

std::vector<Scaling> &RoomScaling::getScalings() {
  return _scalings;
}

const std::string &RoomScaling::getName() const {
  return _trigger;
}

} // namespace ng