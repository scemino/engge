#include "Actor.h"
#include "Engine.h"
#include "Entity.h"
#include "Room.h"
#include "SoundId.h"
#include "SoundManager.h"

namespace ng
{
SoundId::SoundId(SoundManager &soundManager, SoundDefinition *pSoundDefinition, SoundCategory category)
    : _soundManager(soundManager), _pSoundDefinition(pSoundDefinition), _category(category)
{
}

SoundId::~SoundId()
{
    std::cout << "delete SoundId (" << std::hex << this << ") " << _pSoundDefinition->getPath() << std::endl;
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
    _sound.play();
}

void SoundId::setVolume(float volume)
{
    auto path = _pSoundDefinition->getPath();
    std::cout << "setVolume(" << path << "," << volume << ")" << std::endl;
    _volume = volume;
}

float SoundId::getVolume() const
{
    return _volume;
}

void SoundId::stop()
{
    auto path = _pSoundDefinition->getPath();
    std::cout << "stopSoundId(" << path << ")" << std::endl;
    _sound.stop();
}

void SoundId::update(const sf::Time &elapsed)
{
    float entityVolume = 1.f;
    if (_pEntity)
    {
        auto pActor = _soundManager.getEngine()->getCurrentActor();
        if (pActor)
        {
            if (pActor->getRoom() != _pEntity->getRoom())
                entityVolume = 0;
            if (pActor->getRoom() != _pEntity->getRoom())
                entityVolume = 0;
            else
            {
                auto width = _soundManager.getEngine()->getWindow().getView().getSize().x;
                auto diff = fabs(pActor->getPosition().x - _pEntity->getPosition().x);
                entityVolume = (1.5f - (diff / width)) / 1.5f;
                if (entityVolume < 0)
                    entityVolume = 0;
                float pan = (_pEntity->getPosition().x - pActor->getPosition().x) / (width / 2);
                if (pan > 1.f)
                    pan = 1.f;
                if (pan < -1.f)
                    pan = -1.f;
                _sound.setPosition({pan, 0.f, pan < 0.f ? -pan - 1.f : pan - 1.f});
            }
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
