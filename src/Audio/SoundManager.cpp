#include <memory>
#include <ngf/Audio/AudioSystem.h>
#include <engge/Engine/Engine.hpp>
#include <engge/Engine/EngineSettings.hpp>
#include <engge/Entities/Entity.hpp>
#include <engge/EnggeApplication.hpp>
#include <engge/System/Locator.hpp>
#include <engge/System/Logger.hpp>
#include <engge/Audio/SoundDefinition.hpp>
#include <engge/Audio/SoundId.hpp>
#include <engge/Audio/SoundManager.hpp>

namespace ng {
SoundManager::SoundManager() = default;

std::shared_ptr<SoundId> SoundManager::getSound(size_t index) {
  if (index < 1 || index > m_soundIds.size())
    return nullptr;
  return m_soundIds[index - 1];
}

std::shared_ptr<SoundDefinition> SoundManager::defineSound(const std::string &name) {
  if (!Locator<EngineSettings>::get().hasEntry(name))
    return nullptr;

  auto sound = std::make_shared<SoundDefinition>(name);
  m_sounds.push_back(sound);
  return sound;
}

std::shared_ptr<SoundId> SoundManager::playSound(std::shared_ptr<SoundDefinition> soundDefinition,
                                                 int loopTimes,
                                                 const ngf::TimeSpan &fadeInTime,
                                                 int id) {
  return play(soundDefinition, SoundCategory::Sound, loopTimes, fadeInTime, id);
}

std::shared_ptr<SoundId> SoundManager::playTalkSound(std::shared_ptr<SoundDefinition> soundDefinition,
                                                     int loopTimes,
                                                     const ngf::TimeSpan &fadeInTime,
                                                     int id) {
  return play(soundDefinition, SoundCategory::Talk, loopTimes, fadeInTime, id);
}

std::shared_ptr<SoundId> SoundManager::playMusic(std::shared_ptr<SoundDefinition> soundDefinition,
                                                 int loopTimes,
                                                 const ngf::TimeSpan &fadeInTime) {
  return play(soundDefinition, SoundCategory::Music, loopTimes, fadeInTime);
}

std::shared_ptr<SoundId> SoundManager::play(std::shared_ptr<SoundDefinition> soundDefinition,
                                            SoundCategory category,
                                            int loopTimes,
                                            const ngf::TimeSpan &fadeInTime,
                                            int id) {
  soundDefinition->load();
  auto
      sound = m_pEngine->getApplication()->getAudioSystem().playSound(soundDefinition->m_buffer, loopTimes, fadeInTime);
  auto soundId = std::make_shared<SoundId>(*this, soundDefinition, sound, category, id);
  auto index = sound->get().getChannel();
  if (index == -1) {
    error("cannot play sound no more channel available");
    return nullptr;
  }
  std::string sCategory;
  switch (category) {
  case SoundCategory::Music:sCategory = "music";
    break;
  case SoundCategory::Sound:sCategory = "sound";
    break;
  case SoundCategory::Talk:sCategory = "talk";
    break;
  }
  trace("[{}] loop {} {} {}", index, loopTimes, sCategory, soundDefinition->getPath());
  m_soundIds[index] = soundId;
  return soundId;
}

void SoundManager::stopAllSounds() {
  trace("stopAllSounds");
  for (auto channel : m_pEngine->getApplication()->getAudioSystem()) {
    channel.stop();
  }
  for (auto &soundId : m_soundIds) {
    soundId.reset();
  }
}

void SoundManager::stopSound(std::shared_ptr<SoundDefinition> soundDef) {
  trace("stopSound (sound definition: {})", soundDef->getPath());
  for (size_t i = 0; i < getSize(); i++) {
    auto sound = m_soundIds[i];
    if (sound && soundDef.get()->getId() == sound->getId()) {
      sound->getSoundHandle()->get().stop();
      m_soundIds[i].reset();
    }
  }
}

void SoundManager::setVolume(const SoundDefinition *pSoundDef, float volume) {
  volume = std::clamp(volume, 0.f, 1.f);
  trace("setVolume (sound definition: {})", pSoundDef->getPath());
  for (size_t i = 1; i <= getSize(); i++) {
    auto &&sound = getSound(i);
    if (sound && pSoundDef->getId() == sound->getId()) {
      sound->getSoundHandle()->get().setVolume(volume);
    }
  }
}

void SoundManager::update(const ngf::TimeSpan &elapsed) {
  for (auto &&soundId : m_soundIds) {
    if (soundId) {
      soundId->update(elapsed);
      if (soundId->getSoundHandle()->get().getStatus() == ngf::AudioChannel::Status::Stopped) {
        soundId.reset();
      }
    }
  }
}

void SoundManager::pauseAllSounds() {
  for (auto soundId : m_soundIds) {
    if (soundId) {
      soundId->getSoundHandle()->get().pause();
    }
  }
}

void SoundManager::resumeAllSounds() {
  for (auto &soundId : m_soundIds) {
    if (soundId && soundId->getSoundHandle()->get().getStatus() == ngf::AudioChannel::Status::Paused) {
      soundId->getSoundHandle()->get().play();
    }
  }
}
} // namespace ng
