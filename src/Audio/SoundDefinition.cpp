#include <utility>
#include "Engine/EngineSettings.h"
#include "System/Locator.h"
#include "System/Logger.h"
#include "Engine/ResourceManager.h"
#include "Audio/SoundDefinition.h"

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
