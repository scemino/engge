#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Parsers/GGPack.hpp"

namespace ng {
class EngineSettings {
private:
  std::vector<std::unique_ptr<GGPack>> _packs;

public:
  EngineSettings();
  ~EngineSettings() = default;

  void loadPacks();

  bool hasEntry(const std::string &name);
  void readEntry(const std::string &name, std::vector<char> &data);
  void readEntry(const std::string &name, GGPackValue &hash);
  void getEntries(std::vector<std::string> &entries);
};
} // namespace ng
