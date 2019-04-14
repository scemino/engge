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
  
  std::shared_ptr<SoundDefinition> defineSound(const std::string &name);
  std::shared_ptr<SoundId> playSound(std::shared_ptr<SoundDefinition> soundDefinition, bool loop = false);
  std::shared_ptr<SoundId> loopMusic(std::shared_ptr<SoundDefinition> soundDefinition);
  
  void stopAllSounds();
  void stopSound(SoundId &sound);
  void stopSound(std::shared_ptr<SoundDefinition> soundDefinition);
  
  void setVolume(std::shared_ptr<SoundDefinition> soundDefinition, float volume);
  
  std::shared_ptr<SoundId> getSound(void* pSound);
  std::shared_ptr<SoundDefinition> getSoundDefinition(void* pSoundDefinition);
  std::shared_ptr<SoundId> getSound(size_t index);
  
  size_t getSize() const { return _soundIds.size(); }

private:
  int getSlotIndex();

private:
  EngineSettings &_settings;
  std::vector<std::shared_ptr<SoundDefinition>> _sounds;
  std::array<std::shared_ptr<SoundId>, 32> _soundIds;
};
} // namespace ng