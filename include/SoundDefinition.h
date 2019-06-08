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

class SoundId : public Sound
{
public:
  explicit SoundId(SoundDefinition *pSoundDefinition);
  ~SoundId() override;

  void play(bool loop = false);
  void stop();

  void setVolume(float volume);
  float getVolume() const;
  SoundDefinition *getSoundDefinition() { return _pSoundDefinition; }
  bool isPlaying() const { return _sound.getLoop() || _sound.getStatus() == sf::SoundSource::Playing; }
  void fadeTo(float volume, const sf::Time& duration);

  void update(const sf::Time &elapsed);

private:
  SoundDefinition *_pSoundDefinition{nullptr};
  sf::Sound _sound;
  std::unique_ptr<ChangeProperty<float>> _fade;
};
} // namespace ng