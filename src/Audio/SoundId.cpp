#include "engge/Engine/Camera.hpp"
#include "engge/Entities/Entity.hpp"
#include "engge/System/Locator.hpp"
#include "engge/System/Logger.hpp"
#include "engge/Engine/EntityManager.hpp"
#include "engge/Audio/SoundId.hpp"
#include "engge/Audio/SoundManager.hpp"
#include "engge/Scripting/ScriptEngine.hpp"

namespace ng {
SoundId::SoundId(SoundManager &soundManager,
                 std::shared_ptr<SoundDefinition> soundDefinition,
                 std::shared_ptr<ngf::SoundHandle> sound,
                 SoundCategory category,
                 int entityId)
    : m_soundManager(soundManager), m_soundDefinition(soundDefinition), m_sound(sound), m_category(category),
      m_entityId(entityId) {
  m_id = Locator<EntityManager>::get().getSoundId();
}

SoundId::~SoundId() {
  //trace("delete SoundId ({}) {}", (long)this, _pSoundDefinition->getPath());
  m_sound.reset();
}

void SoundId::updateVolume() {
  float entityVolume = 1.f;
  Entity *pEntity = m_entityId ? EntityManager::getScriptObjectFromId<Entity>(m_entityId) : nullptr;

  if (pEntity) {
    auto pRoom = m_soundManager.getEngine()->getRoom();
    auto at = m_soundManager.getEngine()->getCamera().getAt();
    entityVolume = pRoom != pEntity->getRoom() ? 0 : pEntity->getVolume();

    if (pRoom == pEntity->getRoom()) {
      auto width = m_soundManager.getEngine()->getRoom()->getScreenSize().x;
      auto diff = fabs(at.x - pEntity->getPosition().x);
      entityVolume = (1.5f - (diff / width)) / 1.5f;
      if (entityVolume < 0)
        entityVolume = 0;
      float pan = std::clamp((pEntity->getPosition().x - at.x) / (width / 2), -1.f, 1.f);
      m_sound->get().setPanning(pan);
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
  float volume = masterVolume * categoryVolume * entityVolume;
  m_sound->get().setVolume(volume);
}

void SoundId::update(const ngf::TimeSpan &) {
  updateVolume();
}

bool SoundId::isPlaying() const {
  return m_sound->get().getStatus() == ngf::AudioChannel::Status::Playing;
}

void SoundId::stop(const ngf::TimeSpan &fadeOutTime) {
  return m_sound->get().stop(fadeOutTime);
}
} // namespace ng
