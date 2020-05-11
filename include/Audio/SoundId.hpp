#pragma once
#include "SoundCategory.hpp"
#include "SoundDefinition.hpp"

namespace ng {
class Entity;
class SoundManager;

class SoundId : public Sound {
public:
  explicit SoundId(SoundManager &soundManager, SoundDefinition *pSoundDefinition, SoundCategory category);
  ~SoundId() override;

  void play(int loopTimes);
  void stop();
  void pause();
  void resume();

  void setVolume(float volume);
  [[nodiscard]] float getVolume() const;

  SoundDefinition *getSoundDefinition();
  [[nodiscard]] bool isPlaying() const;
  void fadeTo(float volume, const sf::Time &duration);

  void setEntity(int id);

  void update(const sf::Time &elapsed);

private:
  void updateVolume();

private:
  SoundManager &_soundManager;
  SoundDefinition *_pSoundDefinition{nullptr};
  sf::Sound _sound;
  std::unique_ptr<ChangeProperty<float>> _fade;
  SoundCategory _category;
  float _volume{1.0f};
  int _loopTimes{0};
  int _id{0};
};
} // namespace ng
