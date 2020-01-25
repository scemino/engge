#include <utility>
#include "Engine/EngineSettings.hpp"
#include "System/Locator.hpp"
#include "System/Logger.hpp"
#include "Engine/ResourceManager.hpp"
#include "Audio/SoundDefinition.hpp"

namespace ng
{
Sound::~Sound() = default;

SoundDefinition::SoundDefinition(std::string path)
    : _path(std::move(path)), _isLoaded(false)
{
    _id = Locator<ResourceManager>::get().getSoundId();
}

SoundDefinition::~SoundDefinition() = default;

void SoundDefinition::load()
{
    if (_isLoaded)
        return;
    std::vector<char> buffer;
    Locator<EngineSettings>::get().readEntry(_path, buffer);
    _isLoaded = _buffer.loadFromMemory(buffer.data(), buffer.size());
    if (!_isLoaded)
    {
        error("Can't load the sound {}", _path);
    }
}

} // namespace ng
