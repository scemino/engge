#pragma once
#include <string>
#include <vector>
#include "GGPackValue.hpp"

namespace ng {
class SavegameManager {
public:
  static void loadGame(const std::string &path, GGPackValue& hash);
  static void saveGame(const std::string &path, const GGPackValue& hash);
  static int32_t computeHash(const std::vector<char> &data, int32_t size);
};
}