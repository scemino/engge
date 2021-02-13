#pragma once
#include "SoundCategory.hpp"
#include "SoundDefinition.hpp"

namespace ng {
class Entity;
class SoundManager;

class SoundId final : public Sound {
public:
  explicit SoundId(SoundManager &soundManager, SoundDefinition *pSoundDefinition, SoundCategory category);
  ~SoundId() final;

  void play(int loopTimes);
  void stop();
  void pause();
  void resume();

  void setVolume(float volume);
  [[nodiscard]] float getVolume() const;

  SoundDefinition *getSoundDefinition();
  [[nodiscard]] bool isPlaying() const;
  [[nodiscard]] int getLoopTimes() const { return m_loopTimes; }
  [[nodiscard]] SoundCategory getSoundCategory() const { return m_category; }
  void fadeTo(float volume, const ngf::TimeSpan &duration);

  void setEntity(int id);

  void update(const ngf::TimeSpan &elapsed);

private:
  void updateVolume();

private:
  SoundManager &m_soundManager;
  SoundDefinition *m_pSoundDefinition{nullptr};
  sf::Sound m_sound;
  std::unique_ptr<ChangeProperty<float>> m_fade;
  SoundCategory m_category;
  float m_volume{1.0f};
  int m_loopTimes{0};
  int m_entityId{0};
};
} // namespace ng
