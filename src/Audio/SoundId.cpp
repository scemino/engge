#include "Entities/Actor/Actor.hpp"
#include "Engine/Camera.hpp"
#include "Engine/Engine.hpp"
#include "Entities/Entity.hpp"
#include "System/Locator.hpp"
#include "System/Logger.hpp"
#include "Engine/ResourceManager.hpp"
#include "Room/Room.hpp"
#include "Audio/SoundId.hpp"
#include "Audio/SoundManager.hpp"
#include "Scripting/ScriptEngine.hpp"

namespace ng {
SoundId::SoundId(SoundManager &soundManager, SoundDefinition *pSoundDefinition, SoundCategory category)
    : _soundManager(soundManager), _pSoundDefinition(pSoundDefinition), _category(category) {
  _id = Locator<ResourceManager>::get().getSoundId();
}

SoundId::~SoundId() {
  //trace("delete SoundId ({}) {}", (long)this, _pSoundDefinition->getPath());
  stop();
  _pSoundDefinition = nullptr;
}

SoundDefinition *SoundId::getSoundDefinition() {
  return _pSoundDefinition;
}

bool SoundId::isPlaying() const {
  return _sound.getLoop() || _sound.getStatus() == sf::SoundSource::Playing;
}

void SoundId::play(int loopTimes) {
  _loopTimes = loopTimes;
  _pSoundDefinition->load();
  _sound.setBuffer(_pSoundDefinition->_buffer);
  _sound.setLoop(loopTimes == -1);
  updateVolume();
  _sound.play();
}

void SoundId::setVolume(float volume) {
  if (_pSoundDefinition) {
    auto path = _pSoundDefinition->getPath();
//    trace("setVolume({},{})", path, volume);
  }
  _volume = volume;
}

float SoundId::getVolume() const {
  return _volume;
}

void SoundId::stop() {
  auto path = _pSoundDefinition->getPath();
//  trace("stopSoundId({})", path);
  _sound.stop();
}

void SoundId::pause() {
  auto path = _pSoundDefinition->getPath();
  trace("pause sound({})", path);
  _sound.pause();
}

void SoundId::resume() {
  auto path = _pSoundDefinition->getPath();
  trace("resume sound({})", path);
  _sound.play();
}

void SoundId::updateVolume() {
  float entityVolume = 1.f;
  Entity *pEntity = _id ? ScriptEngine::getScriptObjectFromId<Entity>(_id) : nullptr;

  if (pEntity) {
    auto pRoom = _soundManager.getEngine()->getRoom();
    auto at = _soundManager.getEngine()->getCamera().getAt();
    entityVolume = pRoom != pEntity->getRoom() ? 0 : pEntity->getVolume().value_or(1.f);

    if (pRoom == pEntity->getRoom()) {
      auto width = _soundManager.getEngine()->getWindow().getView().getSize().x;
      at.x += width / 2.f;
      auto diff = fabs(at.x - pEntity->getRealPosition().x);
      entityVolume = (1.5f - (diff / width)) / 1.5f;
      if (entityVolume < 0)
        entityVolume = 0;
      float pan = (pEntity->getRealPosition().x - at.x) / (width / 2);
      if (pan > 1.f)
        pan = 1.f;
      if (pan < -1.f)
        pan = -1.f;
      _sound.setPosition({pan, 0.f, pan < 0.f ? -pan - 1.f : pan - 1.f});
    }
  }
  float categoryVolume = 0;
  switch (_category) {
  case SoundCategory::Music:categoryVolume = _soundManager.getMusicVolume();
    break;
  case SoundCategory::Sound:categoryVolume = _soundManager.getSoundVolume();
    break;
  case SoundCategory::Talk:categoryVolume = _soundManager.getTalkVolume();
    break;
  }
  auto masterVolume = _soundManager.getMasterVolume();
  float volume = masterVolume * _volume * categoryVolume * entityVolume;
  _sound.setVolume(volume * 100.f);
}

void SoundId::update(const sf::Time &elapsed) {
  updateVolume();
  if (!isPlaying()) {
    if (_loopTimes > 1) {
      _loopTimes--;
      _sound.play();
    } else {
      auto path = _pSoundDefinition->getPath();
      //trace("Remove sound {} not playing anymore: {}", path, _sound.getStatus());
      _soundManager.stopSound(this);
      return;
    }
  }

  if (!_fade)
    return;

  if (_fade->isElapsed()) {
    _fade.reset();
  } else {
    (*_fade)(elapsed);
  }
}

void SoundId::fadeTo(float volume, const sf::Time &duration) {
  auto get = std::bind(&SoundId::getVolume, this);
  auto set = std::bind(&SoundId::setVolume, this, std::placeholders::_1);
  auto fadeTo = std::make_unique<ChangeProperty<float>>(get, set, volume, duration);
  _fade = std::move(fadeTo);
}

void SoundId::setEntity(int id) {
  _id = id;
}

} // namespace ng
