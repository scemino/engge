#include "SoundDefinition.h"

namespace ng
{
class SoundId : public Sound
{
public:
  explicit SoundId(SoundDefinition *pSoundDefinition);
  ~SoundId() override;

  void play(bool loop = false);
  void stop();

  void setVolume(float volume);
  float getVolume() const;
  SoundDefinition *getSoundDefinition() { return _pSoundDefinition; }
  bool isPlaying() const { return _sound.getLoop() || _sound.getStatus() == sf::SoundSource::Playing; }
  void fadeTo(float volume, const sf::Time& duration);

  void update(const sf::Time &elapsed);

private:
  SoundDefinition *_pSoundDefinition{nullptr};
  sf::Sound _sound;
  std::unique_ptr<ChangeProperty<float>> _fade;
};
} // namespace ng
