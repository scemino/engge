#include "SoundId.h"

namespace ng
{
SoundId::SoundId(SoundDefinition *pSoundDefinition)
    : _pSoundDefinition(pSoundDefinition)
{
}

SoundId::~SoundId()
{
    std::cout << "delete SoundId (" << std::hex << this << ") " << _pSoundDefinition->getPath() << std::endl;
    stop();
    _pSoundDefinition = nullptr;
}

void SoundId::play(bool loop)
{
    _pSoundDefinition->load();
    _sound.setBuffer(_pSoundDefinition->_buffer);
    _sound.setLoop(loop);
    _sound.play();
}

void SoundId::setVolume(float volume)
{
    auto path = _pSoundDefinition->getPath();
    std::cout << "setVolume(" << path << "," << volume << ")" << std::endl;
    _sound.setVolume(volume * 100.f);
}

float SoundId::getVolume() const
{
    return _sound.getVolume() / 100.f;
}

void SoundId::stop()
{
    auto path = _pSoundDefinition->getPath();
    std::cout << "stopSoundId(" << path << ")" << std::endl;
    _sound.stop();
}

void SoundId::update(const sf::Time &elapsed)
{
    if (_fade)
    {
        if(_fade->isElapsed()){
            _fade.reset();
        } else{
            (*_fade)(elapsed);
        }
    }
}

void SoundId::fadeTo(float volume, const sf::Time &duration)
{
    auto get = std::bind(&SoundId::getVolume, this);
    auto set = std::bind(&SoundId::setVolume, this, std::placeholders::_1);
    auto fadeTo = std::make_unique<ChangeProperty<float>>(get, set, volume, duration);
    _fade = std::move(fadeTo);
}
}
