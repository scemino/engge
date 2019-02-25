#include "SoundDefinition.h"

namespace ng
{

SoundDefinition::SoundDefinition(const std::string &path)
    : _pSettings(nullptr), _path(path), _isLoaded(false)
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
        std::cerr << "Can't load the sound" << _path << std::endl;
    }
}

SoundId::SoundId(SoundDefinition &soundDefinition)
    : _soundDefinition(soundDefinition)
{
}

SoundId::~SoundId()
{
    stop();
}

void SoundId::play(bool loop)
{
    _soundDefinition.load();
    _sound.setBuffer(_soundDefinition._buffer);
    _sound.setLoop(loop);
    _sound.play();
}

void SoundId::setVolume(float volume)
{
    std::cout << "setVolume(" << volume << ")" << std::endl;
    _sound.setVolume(volume * 100.f);
}

float SoundId::getVolume() const
{
    return _sound.getVolume() / 100.f;
}

void SoundId::stop()
{
    _sound.stop();
}

} // namespace ng
