#pragma once
#include "SFML/Audio.hpp"
#include "EngineSettings.h"
#include "SoundDefinition.h"

namespace ng
{
class SoundManager
{
  public:
    explicit SoundManager(EngineSettings &settings);
    std::shared_ptr<SoundDefinition> defineSound(const std::string &name);
    std::shared_ptr<SoundId> playSound(SoundDefinition &soundDefinition, bool loop = false);
    std::shared_ptr<SoundId> loopMusic(SoundDefinition &soundDefinition);
    void stopSound(SoundId &sound);

  private:
    EngineSettings &_settings;
    std::vector<std::shared_ptr<SoundDefinition>> _sounds;
    std::vector<std::shared_ptr<SoundId>> _soundIds;
    sf::Music _music;
};
} // namespace ng