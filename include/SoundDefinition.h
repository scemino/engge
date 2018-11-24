#pragma once
#include <string>
#include <iostream>
#include "SFML/Audio.hpp"

namespace gg
{
class SoundId;

class SoundDefinition
{
  friend class SoundId;

public:
  SoundDefinition(const std::string &path);

  const std::string &getPath() const { return _path; };

private:
  void load();

private:
  std::string _path;
  bool _isLoaded;
  sf::SoundBuffer _buffer;
};

class SoundId
{
public:
  SoundId(SoundDefinition &soundDefinition);
  ~SoundId();

  void play(bool loop = false);
  void stop();

  void setVolume(float volume);
  float getVolume() const;

private:
  SoundDefinition &_soundDefinition;
  sf::Sound _sound;
};
}