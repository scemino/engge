#pragma once
#include <iostream>
#include <memory>
#include <string>
#include "SFML/Audio.hpp"
#include "EngineSettings.h"
#include "Function.h"

namespace ng
{
class SoundId;

class Sound
{
public:
  virtual ~Sound() {}
};

class SoundDefinition : public Sound
{
  friend class SoundId;

public:
  explicit SoundDefinition(std::string path);
  ~SoundDefinition() override;

  void setSettings(EngineSettings &settings);
  const std::string &getPath() const { return _path; };

private:
  void load();

private:
  EngineSettings *_pSettings;
  std::string _path;
  bool _isLoaded;
  sf::SoundBuffer _buffer;
};
} // namespace ng