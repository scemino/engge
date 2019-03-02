#pragma once
#include <string>
#include <iostream>
#include "SFML/Audio.hpp"
#include "EngineSettings.h"

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
  explicit SoundDefinition(const std::string &path);
  ~SoundDefinition() {}

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

class SoundId : public Sound
{
public:
  explicit SoundId(SoundDefinition &soundDefinition);
  ~SoundId();

  void play(bool loop = false);
  void stop();

  void setVolume(float volume);
  float getVolume() const;
  SoundDefinition &getSoundDefinition() { return _soundDefinition; }
  bool isPlaying() const { return _sound.getStatus() == sf::SoundSource::Playing; }

private:
  SoundDefinition &_soundDefinition;
  sf::Sound _sound;
};
} // namespace ng