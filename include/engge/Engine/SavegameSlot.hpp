#include <string>
#include <SFML/System/Time.hpp>

#pragma once

namespace ng {
class SavegameSlot {
public:
  int slot{0};
  time_t savetime{};
  sf::Time gametime;
  std::string path;
  bool easyMode{false};

  [[nodiscard]] std::wstring getSaveTimeString() const;
  [[nodiscard]] std::wstring getGameTimeString() const;
};
}
