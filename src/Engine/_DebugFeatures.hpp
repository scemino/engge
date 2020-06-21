#pragma once

namespace ng {

struct _DebugFeatures {
  inline static bool showHoveredObject{false};
  inline static bool showCursorPosition{false};
  inline static sf::Time _renderTime;
  inline static sf::Time _updateTime;
};

}