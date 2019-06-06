#include <utility>

#include "SoundDefinition.h"

namespace ng
{

SoundDefinition::SoundDefinition(std::string path)
    : _pSettings(nullptr), _path(std::move(path)), _isLoaded(false)
{
}

SoundDefinition::~SoundDefinition()
  {
    std::cout << "delete SoundDefinition " << _path << " " << std::hex << this << std::endl;
  }

void SoundDefinition::setSettings(EngineSettings &settings)
{
    _pSettings = &settings;
}

void SoundDefinition::load()
{
    if (_isLoaded)
        return;
    std::vector<char> buffer;
    _pSettings->readEntry(_path, buffer);
    _isLoaded = _buffer.loadFromMemory(buffer.data(), buffer.size());
    if (!_isLoaded)
    {
        std::cerr << "Can't load the sound " << _path << std::endl;
    }
}

SoundId::SoundId(SoundDefinition *pSoundDefinition)
    : _pSoundDefinition(pSoundDefinition)
{
}

SoundId::~SoundId()
{
    std::cout << "delete SoundId " << std::hex << this << std::endl;
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

} // namespace ng
