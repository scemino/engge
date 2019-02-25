#pragma once
#include <utility>
#include <string>
#include <sstream>
#include "GGPack.h"

namespace ng
{
class EngineSettings
{
private:
  const std::string _gamePath;
  GGPack _pack1;
  GGPack _pack2;

public:
  explicit EngineSettings(std::string gamePath)
      : _gamePath(std::move(gamePath))
  {
    _pack1.open("ThimbleweedPark.ggpack1");
    _pack2.open("ThimbleweedPark.ggpack2");
  }

  bool hasEntry(const std::string &name)
  {
    return _pack1.hasEntry(name) || _pack2.hasEntry(name);
  }

  void readEntry(const std::string &name, std::vector<char> &data)
  {
    if (_pack1.hasEntry(name))
    {
      _pack1.readEntry(name, data);
      return;
    }
    if (_pack2.hasEntry(name))
    {
      _pack2.readEntry(name, data);
    }
  }

  void readEntry(const std::string &name, GGPackValue &hash)
  {
    if (_pack1.hasEntry(name))
    {
      _pack1.readHashEntry(name, hash);
      return;
    }
    if (_pack2.hasEntry(name))
    {
      _pack2.readHashEntry(name, hash);
    }
  }

  const std::string &getGamePath() const { return _gamePath; }
};
} // namespace ng
