#pragma once
#include <string>
#include <vector>
#include <ngf/IO/GGPackValue.h>

namespace ng {
class SavegameManager {
public:
  static ngf::GGPackValue loadGame(const std::string &path);
  static void saveGame(const std::string &path, const ngf::GGPackValue& hash);
  static int32_t computeHash(const std::vector<char> &data, int32_t size);
};
}