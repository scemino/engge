#include <utility>
#include "Locator.h"
#include "Logger.h"
#include "ResourceManager.h"
#include "SoundDefinition.h"

namespace ng
{
Sound::~Sound() = default;

SoundDefinition::SoundDefinition(std::string path)
    : _pSettings(nullptr), _path(std::move(path)), _isLoaded(false)
{
    _id = Locator<ResourceManager>::get().getSoundId();
}

SoundDefinition::~SoundDefinition() = default;

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
        error("Can't load the sound {}", _path);
    }
}

} // namespace ng
