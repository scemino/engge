#pragma once
#include <string>
#include "GGPack.h"

namespace ng
{
class EngineSettings
{
private:
  GGPack _pack1;
  GGPack _pack2;

public:
  EngineSettings();

  bool hasEntry(const std::string &name);
  void readEntry(const std::string &name, std::vector<char> &data);
  void readEntry(const std::string &name, GGPackValue &hash);
};
} // namespace ng
