#pragma once
#include <utility>
#include <string>

namespace ng
{
class NGEngineSettings
{
private:
  const std::string _gamePath;

public:
  explicit NGEngineSettings(std::string gamePath)
      : _gamePath(std::move(gamePath))
  {
  }

  const std::string &getGamePath() const { return _gamePath; }
};
} // namespace ng
