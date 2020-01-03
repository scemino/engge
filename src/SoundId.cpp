#include "Actor.h"
#include "Camera.h"
#include "Engine.h"
#include "Entity.h"
#include "Locator.h"
#include "Logger.h"
#include "ResourceManager.h"
#include "Room.h"
#include "SoundId.h"
#include "SoundManager.h"

namespace ng
{
SoundId::SoundId(SoundManager &soundManager, SoundDefinition *pSoundDefinition, SoundCategory category)
    : _soundManager(soundManager), _pSoundDefinition(pSoundDefinition), _category(category)
{
    _id = Locator::getResourceManager().getSoundId();
}

SoundId::~SoundId()
{
    trace("delete SoundId ({}) {}", (long)this, _pSoundDefinition->getPath());
    stop();
    _pSoundDefinition = nullptr;
}

SoundDefinition *SoundId::getSoundDefinition()
{
    return _pSoundDefinition;
}

bool SoundId::isPlaying() const
{
    return _sound.getLoop() || _sound.getStatus() == sf::SoundSource::Playing;
}

void SoundId::play(int loopTimes)
{
    _loopTimes = loopTimes;
    _pSoundDefinition->load();
    _sound.setBuffer(_pSoundDefinition->_buffer);
    _sound.setLoop(loopTimes == -1);
    updateVolume();
    _sound.play();
}

void SoundId::setVolume(float volume)
{
    if(_pSoundDefinition) {
        auto path = _pSoundDefinition->getPath();
        trace("setVolume({},{})", path, volume);
    }
    _volume = volume;
}

float SoundId::getVolume() const
{
    return _volume;
}

void SoundId::stop()
{
    auto path = _pSoundDefinition->getPath();
    trace("stopSoundId({})", path);
    _sound.stop();
}

void SoundId::pause()
{
    auto path = _pSoundDefinition->getPath();
    trace("pause sound({})", path);
    _sound.pause();
}

void SoundId::resume()
{
    auto path = _pSoundDefinition->getPath();
    trace("resume sound({})", path);
    _sound.play();
}

void SoundId::updateVolume()
{
    float entityVolume = 1.f;
    if (_pEntity)
    {
        entityVolume = _pEntity->getVolume();
        auto pRoom = _soundManager.getEngine()->getRoom();
        auto at = _soundManager.getEngine()->getCamera().getAt();
        
        if (pRoom != _pEntity->getRoom())
            entityVolume = 0;
        else
        {
            auto width = _soundManager.getEngine()->getWindow().getView().getSize().x;
            at.x += width / 2.f;
            auto diff = fabs(at.x - _pEntity->getRealPosition().x);
            entityVolume = (1.5f - (diff / width)) / 1.5f;
            if (entityVolume < 0)
                entityVolume = 0;
            float pan = (_pEntity->getRealPosition().x - at.x) / (width / 2);
            if (pan > 1.f)
                pan = 1.f;
            if (pan < -1.f)
                pan = -1.f;
            _sound.setPosition({pan, 0.f, pan < 0.f ? -pan - 1.f : pan - 1.f});
        }
    }
    float categoryVolume = 0;
    switch (_category)
    {
    case SoundCategory::Music:
        categoryVolume = _soundManager.getMusicVolume();
        break;
    case SoundCategory::Sound:
        categoryVolume = _soundManager.getSoundVolume();
        break;
    case SoundCategory::Talk:
        categoryVolume = _soundManager.getTalkVolume();
        break;
    }
    auto masterVolume = _soundManager.getMasterVolume();
    float volume = masterVolume * _volume * categoryVolume * entityVolume;
    _sound.setVolume(volume * 100.f);
}

void SoundId::update(const sf::Time &elapsed)
{
    updateVolume();
    if (!isPlaying())
    {
        if (_loopTimes > 1)
        {
            _loopTimes--;
            _sound.play();
        }
        else
        {
            _soundManager.stopSound(this);
            return;
        }
    }

    if (!_fade)
        return;

    if (_fade->isElapsed())
    {
        _fade.reset();
    }
    else
    {
        (*_fade)(elapsed);
    }
}

void SoundId::fadeTo(float volume, const sf::Time &duration)
{
    auto get = std::bind(&SoundId::getVolume, this);
    auto set = std::bind(&SoundId::setVolume, this, std::placeholders::_1);
    auto fadeTo = std::make_unique<ChangeProperty<float>>(get, set, volume, duration);
    _fade = std::move(fadeTo);
}

void SoundId::setEntity(Entity *pEntity)
{
    _pEntity = pEntity;
}

} // namespace ng
