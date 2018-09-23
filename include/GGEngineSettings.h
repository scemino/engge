#pragma once
#include <utility>
#include <string>

namespace gg
{
class GGEngineSettings
{
private:
  const std::string _gamePath;

public:
  explicit GGEngineSettings(std::string gamePath)
      : _gamePath(std::move(gamePath))
  {
  }

  const std::string &getGamePath() const { return _gamePath; }
};
} // namespace gg
