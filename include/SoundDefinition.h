#pragma once
#include <string>
#include <iostream>
#include "SFML/Audio.hpp"

namespace ng
{
class SoundId;

class SoundDefinition
{
    friend class SoundId;

  public:
    explicit SoundDefinition(const std::string &path);

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
    explicit SoundId(SoundDefinition &soundDefinition);
    ~SoundId();

    void play(bool loop = false);
    void stop();

    void setVolume(float volume);
    float getVolume() const;
    bool isPlaying() const { return _sound.getStatus() == sf::SoundSource::Playing; }

  private:
    SoundDefinition &_soundDefinition;
    sf::Sound _sound;
};
} // namespace ng