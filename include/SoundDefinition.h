#pragma once
#include <iostream>
#include <memory>
#include <string>
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
  ~SoundDefinition() override
  {
    std::cout << "delete SoundDefinition: " << _path << std::endl;
  }

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
  explicit SoundId(const std::shared_ptr<SoundDefinition> &soundDefinition);
  ~SoundId() override;

  void play(bool loop = false);
  void stop();

  void setVolume(float volume);
  float getVolume() const;
  std::shared_ptr<SoundDefinition> getSoundDefinition() { return _soundDefinition; }
  bool isPlaying() const { return _sound.getStatus() == sf::SoundSource::Playing; }

private:
  std::shared_ptr<SoundDefinition> _soundDefinition;
  sf::Sound _sound;
};
} // namespace ng