#include <fstream>
#include <memory>
#include "SoundManager.h"

namespace ng
{
SoundManager::SoundManager(EngineSettings &settings)
    : _settings(settings)
{
}

std::shared_ptr<SoundId> SoundManager::getSound(size_t index)
{
    if (index < 1 || index > _soundIds.size())
        return nullptr;
    return _soundIds[index - 1];
}

int SoundManager::getSlotIndex()
{
    for (size_t i = 0; i < _soundIds.size(); i++)
    {
        if (_soundIds[i] == nullptr || !_soundIds[i]->isPlaying())
            return i;
    }
    return -1;
}

std::shared_ptr<SoundDefinition> SoundManager::defineSound(const std::string &name)
{
    if (!_settings.hasEntry(name))
        return nullptr;

    auto sound = std::make_shared<SoundDefinition>(name);
    sound->setSettings(_settings);
    _sounds.push_back(sound);
    return sound;
}

std::shared_ptr<SoundId> SoundManager::playSound(SoundDefinition &soundDefinition, bool loop)
{
    auto soundId = std::make_shared<SoundId>(soundDefinition);
    auto index = getSlotIndex();
    if (index == -1)
        return nullptr;
    _soundIds[index] = soundId;
    soundId->play(loop);
    return soundId;
}

std::shared_ptr<SoundId> SoundManager::loopMusic(SoundDefinition &soundDefinition)
{
    auto soundId = std::make_shared<SoundId>(soundDefinition);
    auto index = getSlotIndex();
    if (index == -1)
        return nullptr;
    _soundIds[index] = soundId;
    soundId->play(true);
    return soundId;
}

void SoundManager::stopAllSounds()
{
    std::cout << "stopAllSounds" << std::endl;
    for (size_t i = 0; i < _soundIds.size(); i++)
    {
        if (_soundIds[i] != nullptr)
        {
            _soundIds[i]->stop();
            _soundIds[i] = nullptr;
        }
    }
}

void SoundManager::stopSound(SoundId &sound)
{
    std::cout << "stopSound" << std::endl;
    sound.stop();
    for (size_t i = 0; i < _soundIds.size(); i++)
    {
        if (_soundIds[i] != nullptr && _soundIds[i].get() == &sound)
        {
            _soundIds[i]->stop();
            _soundIds[i] = nullptr;
            return;
        }
    }
}
} // namespace ng
