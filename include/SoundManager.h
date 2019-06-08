#pragma once
#include <array>
#include <memory>
#include <vector>

namespace ng
{
class EngineSettings;
class SoundDefinition;
class SoundId;

class SoundManager
{
public:
  explicit SoundManager(EngineSettings &settings);
  
  SoundDefinition* defineSound(const std::string &name);
  SoundId* playSound(SoundDefinition* pSoundDefinition, bool loop = false);
  SoundId* loopMusic(SoundDefinition* pSoundDefinition);
  
  void stopAllSounds();
  void stopSound(SoundId* pSound);
  void stopSound(const SoundDefinition* pSoundDefinition);
  
  void setVolume(const SoundDefinition* pSoundDefinition, float volume);
  
  SoundId* getSound(size_t index);
  SoundId* getSoundFromId(Sound* id);
  SoundDefinition *getSoundDefinitionFromId(Sound* id);
  
  size_t getSize() const { return _soundIds.size(); }

  void update(const sf::Time &elapsed);

private:
  int getSlotIndex();

private:
  EngineSettings &_settings;
  std::vector<std::unique_ptr<SoundDefinition>> _sounds;
  std::array<std::unique_ptr<SoundId>, 32> _soundIds;
};
} // namespace ng