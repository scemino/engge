#pragma once

namespace gg
{
class GGEngineSettings
{
private:
  const char *_gamePath;

public:
  GGEngineSettings(const char *gamePath)
      : _gamePath(gamePath)
  {
  }

  const char *getGamePath() const { return _gamePath; }
};
} // namespace gg
