#include <memory>
#include "engge/Engine/EngineSettings.hpp"
#include "engge/Entities/Entity.hpp"
#include "engge/System/Locator.hpp"
#include "engge/System/Logger.hpp"
#include "engge/Audio/SoundDefinition.hpp"
#include "engge/Audio/SoundId.hpp"
#include "engge/Audio/SoundManager.hpp"

namespace ng {
SoundManager::SoundManager() = default;

SoundId *SoundManager::getSound(size_t index) {
  if (index < 1 || index > m_soundIds.size())
    return nullptr;
  return m_soundIds.at(index - 1).get();
}

int SoundManager::getSlotIndex() {
  for (size_t i = 0; i < m_soundIds.size(); i++) {
    if (m_soundIds.at(i) == nullptr || !m_soundIds.at(i)->isPlaying())
      return i;
  }
  return -1;
}

SoundDefinition *SoundManager::defineSound(const std::string &name) {
  if (!Locator<EngineSettings>::get().hasEntry(name))
    return nullptr;

  auto sound = std::make_unique<SoundDefinition>(name);
  auto pSound = sound.get();
  m_sounds.push_back(std::move(sound));
  return pSound;
}

SoundId *SoundManager::playSound(SoundDefinition *pSoundDefinition, int loopTimes, int id) {
  return play(pSoundDefinition, SoundCategory::Sound, loopTimes, id);
}

SoundId *SoundManager::playTalkSound(SoundDefinition *pSoundDefinition, int loopTimes, int id) {
  return play(pSoundDefinition, SoundCategory::Talk, loopTimes, id);
}

SoundId *SoundManager::playMusic(SoundDefinition *pSoundDefinition, int loopTimes) {
  return play(pSoundDefinition, SoundCategory::Music, loopTimes);
}

SoundId *SoundManager::play(SoundDefinition *pSoundDefinition, SoundCategory category, int loopTimes, int id) {
  auto soundId = std::make_unique<SoundId>(*this, pSoundDefinition, category);
  soundId->setEntity(id);
  auto index = getSlotIndex();
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
  //trace("[{}] loop {} {} {}", index, loopTimes, sCategory, pSoundDefinition->getPath());
  SoundId *pSoundId = soundId.get();
  m_soundIds.at(index) = std::move(soundId);
  pSoundId->play(loopTimes);
  return pSoundId;
}

void SoundManager::stopAllSounds() {
  trace("stopAllSounds");
  for (auto &_soundId : m_soundIds) {
    if (_soundId) {
      _soundId.reset();
    }
  }
}

void SoundManager::stopSound(SoundId *pSound) {
  if (!pSound)
    return;
  for (auto &_soundId : m_soundIds) {
    if (_soundId && _soundId.get() == pSound) {
      _soundId.reset();
      return;
    }
  }
}

void SoundManager::stopSound(const SoundDefinition *pSoundDef) {
  trace("stopSound (sound definition: {})", pSoundDef->getPath());
  for (size_t i = 1; i <= getSize(); i++) {
    auto &&sound = getSound(i);
    if (sound && pSoundDef->getId() == sound->getId()) {
      stopSound(sound);
    }
  }
}

void SoundManager::setVolume(const SoundDefinition *pSoundDef, float volume) {
  volume = std::clamp(volume, 0.f, 1.f);
  trace("setVolume (sound definition: {})", pSoundDef->getPath());
  for (size_t i = 1; i <= getSize(); i++) {
    auto &&sound = getSound(i);
    if (sound && pSoundDef->getId() == sound->getId()) {
      sound->setVolume(volume);
    }
  }
}

void SoundManager::update(const ngf::TimeSpan &elapsed) {
  for (auto &&soundId : m_soundIds) {
    if (soundId) {
      soundId->update(elapsed);
    }
  }
}

void SoundManager::pauseAllSounds() {
  for (auto &&soundId : m_soundIds) {
    if (soundId) {
      soundId->pause();
    }
  }
}

void SoundManager::resumeAllSounds() {
  for (auto &&soundId : m_soundIds) {
    if (soundId) {
      soundId->resume();
    }
  }
}
} // namespace ng
