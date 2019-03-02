#pragma once
#include <vector>
#include <array>
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
  void stopAllSounds();
  void stopSound(SoundId &sound);
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