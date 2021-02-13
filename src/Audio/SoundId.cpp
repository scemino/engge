#include "engge/Engine/Camera.hpp"
#include "engge/Entities/Entity.hpp"
#include "engge/System/Locator.hpp"
#include "engge/System/Logger.hpp"
#include "engge/Engine/EntityManager.hpp"
#include "engge/Audio/SoundId.hpp"
#include "engge/Audio/SoundManager.hpp"
#include "engge/Scripting/ScriptEngine.hpp"

namespace ng {
SoundId::SoundId(SoundManager &soundManager, SoundDefinition *pSoundDefinition, SoundCategory category)
    : m_soundManager(soundManager), m_pSoundDefinition(pSoundDefinition), m_category(category) {
  m_id = Locator<EntityManager>::get().getSoundId();
}

SoundId::~SoundId() {
  //trace("delete SoundId ({}) {}", (long)this, _pSoundDefinition->getPath());
  stop();
  m_pSoundDefinition = nullptr;
}

SoundDefinition *SoundId::getSoundDefinition() {
  return m_pSoundDefinition;
}

bool SoundId::isPlaying() const {
  return m_sound.getLoop() || m_sound.getStatus() == sf::SoundSource::Playing;
}

void SoundId::play(int loopTimes) {
  m_loopTimes = loopTimes;
  m_pSoundDefinition->load();
  m_sound.setBuffer(m_pSoundDefinition->m_buffer);
  m_sound.setLoop(loopTimes == -1);
  updateVolume();
  m_sound.play();
}

void SoundId::setVolume(float volume) {
//  if (_pSoundDefinition) {
//    auto path = _pSoundDefinition->getPath();
//    trace("setVolume({},{})", path, volume);
//  }
  m_volume = volume;
}

float SoundId::getVolume() const {
  return m_volume;
}

void SoundId::stop() {
//  auto path = _pSoundDefinition->getPath();
//  trace("stopSoundId({})", path);
  m_loopTimes = 0;
  m_sound.stop();
}

void SoundId::pause() {
  auto path = m_pSoundDefinition->getPath();
  trace("pause sound({})", path);
  m_sound.pause();
}

void SoundId::resume() {
  auto path = m_pSoundDefinition->getPath();
  trace("resume sound({})", path);
  m_sound.play();
}

void SoundId::updateVolume() {
  float entityVolume = 1.f;
  Entity *pEntity = m_entityId ? EntityManager::getScriptObjectFromId<Entity>(m_entityId) : nullptr;

  if (pEntity) {
    auto pRoom = m_soundManager.getEngine()->getRoom();
    auto at = m_soundManager.getEngine()->getCamera().getAt();
    entityVolume = pRoom != pEntity->getRoom() ? 0 : pEntity->getVolume().value_or(1.f);

    if (pRoom == pEntity->getRoom()) {
      auto width = m_soundManager.getEngine()->getRoom()->getScreenSize().x;
      auto diff = fabs(at.x - pEntity->getPosition().x);
      entityVolume = (1.5f - (diff / width)) / 1.5f;
      if (entityVolume < 0)
        entityVolume = 0;
      float pan = (pEntity->getPosition().x - at.x) / (width / 2);
      if (pan > 1.f)
        pan = 1.f;
      if (pan < -1.f)
        pan = -1.f;
      m_sound.setPosition({pan, 0.f, pan < 0.f ? -pan - 1.f : pan - 1.f});
    }
  }
  float categoryVolume = 0;
  switch (m_category) {
  case SoundCategory::Music:categoryVolume = m_soundManager.getMusicVolume();
    break;
  case SoundCategory::Sound:categoryVolume = m_soundManager.getSoundVolume();
    break;
  case SoundCategory::Talk:categoryVolume = m_soundManager.getTalkVolume();
    break;
  }
  auto masterVolume = m_soundManager.getMasterVolume();
  float volume = masterVolume * m_volume * categoryVolume * entityVolume;
  m_sound.setVolume(volume * 100.f);
}

void SoundId::update(const ngf::TimeSpan &elapsed) {
  updateVolume();
  if (!isPlaying()) {
    if (m_loopTimes > 1) {
      m_loopTimes--;
      m_sound.play();
    } else {
//      auto path = _pSoundDefinition->getPath();
//      trace("Remove sound {} not playing anymore: {}", path, _sound.getStatus());
      m_soundManager.stopSound(this);
      return;
    }
  }

  if (!m_fade)
    return;

  if (m_fade->isElapsed()) {
    m_fade.reset();
  } else {
    (*m_fade)(elapsed);
  }
}

void SoundId::fadeTo(float volume, const ngf::TimeSpan &duration) {
  const auto get = [this] { return getVolume(); };
  const auto set = [this](float v) { setVolume(v); };
  auto fadeTo = std::make_unique<ChangeProperty<float>>(get, set, volume, duration);
  m_fade = std::move(fadeTo);
}

void SoundId::setEntity(int id) {
  m_entityId = id;
}

} // namespace ng
