#pragma once
#include <array>
#include <memory>
#include <vector>
#include "SoundCategory.hpp"
#include "SoundId.hpp"
#include "SoundDefinition.hpp"

namespace ng {
class Entity;
class Engine;
class SoundDefinition;
class SoundId;

class SoundManager {
public:
  SoundManager();

  void setEngine(Engine *pEngine) { m_pEngine = pEngine; }
  [[nodiscard]] Engine *getEngine() const { return m_pEngine; }

  SoundDefinition *defineSound(const std::string &name);
  SoundId *playSound(SoundDefinition *pSoundDefinition, int loopTimes = 1, int id = 0);
  SoundId *playTalkSound(SoundDefinition *pSoundDefinition, int loopTimes = 1, int id = 0);
  SoundId *playMusic(SoundDefinition *pSoundDefinition, int loopTimes = 1);

  void pauseAllSounds();
  void resumeAllSounds();

  void stopAllSounds();
  void stopSound(SoundId *pSound);
  void stopSound(const SoundDefinition *pSoundDefinition);

  void setMasterVolume(float volume) { m_masterVolume = std::clamp(volume, 0.f, 1.f); }
  [[nodiscard]] float getMasterVolume() const { return m_masterVolume; }
  void setSoundVolume(float volume) { m_soundVolume = std::clamp(volume, 0.f, 1.f); }
  [[nodiscard]] float getSoundVolume() const { return m_soundVolume; }
  void setMusicVolume(float volume) { m_musicVolume = std::clamp(volume, 0.f, 1.f); }
  [[nodiscard]] float getMusicVolume() const { return m_musicVolume; }
  void setTalkVolume(float volume) { m_talkVolume = std::clamp(volume, 0.f, 1.f); }
  [[nodiscard]] float getTalkVolume() const { return m_talkVolume; }
  void setVolume(const SoundDefinition *pSoundDefinition, float volume);

  SoundId *getSound(size_t index);
  std::vector<std::unique_ptr<SoundDefinition>> &getSoundDefinitions() { return m_sounds; }
  std::array<std::unique_ptr<SoundId>, 32> &getSounds() { return m_soundIds; }

  void setSoundHover(SoundDefinition *pSound) { m_pSoundHover = pSound; }
  [[nodiscard]] SoundDefinition *getSoundHover() const { return m_pSoundHover; }

  [[nodiscard]] size_t getSize() const { return m_soundIds.size(); }

  void update(const ngf::TimeSpan &elapsed);

private:
  int getSlotIndex();
  SoundId *play(SoundDefinition *pSoundDefinition,
                SoundCategory category,
                int loopTimes = 1,
                int id = 0);

private:
  std::vector<std::unique_ptr<SoundDefinition>> m_sounds;
  std::array<std::unique_ptr<SoundId>, 32> m_soundIds;
  Engine *m_pEngine{nullptr};
  float m_masterVolume{1};
  float m_soundVolume{1};
  float m_musicVolume{1};
  float m_talkVolume{1};
  SoundDefinition *m_pSoundHover{nullptr};
};
} // namespace ng