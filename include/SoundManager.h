#pragma once
#include <array>
#include <memory>
#include <vector>
#include "SoundCategory.h"

namespace ng
{
class Entity;
class Engine;
class SoundDefinition;
class SoundId;

class SoundManager
{
public:
  SoundManager();

  void setEngine(Engine *pEngine) { _pEngine = pEngine; }
  Engine * getEngine() const { return _pEngine; }

  SoundDefinition *defineSound(const std::string &name);
  SoundId *playSound(SoundDefinition *pSoundDefinition, int loopTimes = 1, Entity* pEntity = nullptr);
  SoundId *playTalkSound(SoundDefinition *pSoundDefinition, int loopTimes = 1, Entity* pEntity = nullptr);
  SoundId *playMusic(SoundDefinition *pSoundDefinition, int loopTimes = 1);

  void pauseAllSounds();
  void resumeAllSounds();
  
  void stopAllSounds();
  void stopSound(SoundId *pSound);
  void stopSound(const SoundDefinition *pSoundDefinition);

  void setMasterVolume(float volume) { _masterVolume = volume; }
  float getMasterVolume() const { return _masterVolume; }
  void setSoundVolume(float volume) { _soundVolume = volume; }
  float getSoundVolume() const { return _soundVolume; }
  void setMusicVolume(float volume) { _musicVolume = volume; }
  float getMusicVolume() const { return _musicVolume; }
  void setTalkVolume(float volume) { _talkVolume = volume; }
  float getTalkVolume() const { return _talkVolume; }
  void setVolume(const SoundDefinition *pSoundDefinition, float volume);

  SoundId* getSound(size_t index);
  std::vector<std::unique_ptr<SoundDefinition>>& getSoundDefinitions() { return _sounds; }
  std::array<std::unique_ptr<SoundId>, 32>& getSounds() { return _soundIds; }

  size_t getSize() const { return _soundIds.size(); }

  void update(const sf::Time &elapsed);

private:
  int getSlotIndex();
  SoundId* play(SoundDefinition *pSoundDefinition, SoundCategory category, int loopTimes = 1, Entity* pEntity = nullptr);

private:
  std::vector<std::unique_ptr<SoundDefinition>> _sounds;
  std::array<std::unique_ptr<SoundId>, 32> _soundIds;
  Engine *_pEngine{nullptr};
  float _masterVolume{1};
  float _soundVolume{1};
  float _musicVolume{1};
  float _talkVolume{1};
};
} // namespace ng