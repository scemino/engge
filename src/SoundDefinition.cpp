#include <utility>

#include "SoundDefinition.h"

namespace ng
{

SoundDefinition::SoundDefinition(std::string path)
    : _pSettings(nullptr), _path(std::move(path)), _isLoaded(false)
{
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

SoundId::SoundId(std::shared_ptr<SoundDefinition> soundDefinition)
    : _soundDefinition(std::move(soundDefinition))
{
}

SoundId::~SoundId()
{
    std::cout << "delete SoundId" << std::endl;
    stop();
}

void SoundId::play(bool loop)
{
    _soundDefinition->load();
    _sound.setBuffer(_soundDefinition->_buffer);
    _sound.setLoop(loop);
    _sound.play();
}

void SoundId::setVolume(float volume)
{
    auto path = _soundDefinition->getPath();
    std::cout << "setVolume(" << path << "," << volume << ")" << std::endl;
    _sound.setVolume(volume * 100.f);
}

float SoundId::getVolume() const
{
    return _sound.getVolume() / 100.f;
}

void SoundId::stop()
{
    auto path = _soundDefinition->getPath();
    std::cout << "stopSoundId(" << path << ")" << std::endl;
    _sound.stop();
}

} // namespace ng
