#include <fstream>
#include <memory>
#include "SoundDefinition.h"
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
    return _soundIds.at(index - 1);
}

std::shared_ptr<SoundDefinition> SoundManager::getSoundDefinition(void *pSoundDefinition)
{
    auto it = std::find_if(_sounds.begin(), _sounds.end(), [pSoundDefinition](std::shared_ptr<SoundDefinition> sd) {
        return sd.get() == pSoundDefinition;
    });
    if (it == _sounds.end())
        return nullptr;
    return *it;
}
std::shared_ptr<SoundId> SoundManager::getSound(void *pSound)
{
    auto it = std::find_if(_soundIds.begin(), _soundIds.end(), [pSound](std::shared_ptr<SoundId> sd) {
        return sd.get() == pSound;
    });
    if (it == _soundIds.end())
        return nullptr;
    return *it;
}

int SoundManager::getSlotIndex()
{
    for (size_t i = 0; i < _soundIds.size(); i++)
    {
        if (_soundIds.at(i) == nullptr || !_soundIds.at(i)->isPlaying())
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

std::shared_ptr<SoundId> SoundManager::playSound(std::shared_ptr<SoundDefinition> soundDefinition, bool loop)
{
    auto soundId = std::make_shared<SoundId>(soundDefinition);
    auto index = getSlotIndex();
    if (index == -1)
    {
        std::cerr << "cannot play sound no more channel available" << std::endl;
        return nullptr;
    }
    std::cout << " [" << index << "]"
              << "play sound " << soundDefinition->getPath() << std::endl;
    _soundIds.at(index) = soundId;
    soundId->play(loop);
    return soundId;
}

std::shared_ptr<SoundId> SoundManager::loopMusic(std::shared_ptr<SoundDefinition> soundDefinition)
{
    auto soundId = std::make_shared<SoundId>(soundDefinition);
    auto index = getSlotIndex();
    if (index == -1)
    {
        std::cerr << "cannot play sound no more channel available" << std::endl;
        return nullptr;
    }
    std::cout << " [" << index << "]"
              << "loop music " << soundDefinition->getPath() << std::endl;
    _soundIds.at(index) = soundId;
    soundId->play(true);
    return soundId;
}

void SoundManager::stopAllSounds()
{
    std::cout << "stopAllSounds" << std::endl;
    for (size_t i = 0; i < _soundIds.size(); i++)
    {
        if (_soundIds.at(i) != nullptr)
        {
            _soundIds.at(i)->stop();
            _soundIds.at(i).reset();
        }
    }
}

void SoundManager::stopSound(std::shared_ptr<SoundId> sound)
{
    if(!sound) return;
    sound->stop();
    for (size_t i = 0; i < _soundIds.size(); i++)
    {
        if (_soundIds.at(i) != nullptr && _soundIds.at(i) == sound)
        {
            _soundIds.at(i)->stop();
            _soundIds.at(i) = nullptr;
            return;
        }
    }
}

void SoundManager::stopSound(std::shared_ptr<SoundDefinition> soundDef)
{
    std::cout << "stopSound (sound definition: " << soundDef->getPath() << ")" << std::endl;
    for (size_t i = 1; i <= getSize(); i++)
    {
        auto &&sound = getSound(i);
        if (sound && soundDef == sound->getSoundDefinition())
        {
            stopSound(sound);
        }
    }
}

void SoundManager::setVolume(std::shared_ptr<SoundDefinition> soundDef, float volume)
{
    std::cout << "setVolume (sound definition: " << soundDef->getPath() << ")" << std::endl;
    for (size_t i = 1; i <= getSize(); i++)
    {
        auto &&sound = getSound(i);
        if (sound && soundDef == sound->getSoundDefinition())
        {
            sound->setVolume(volume);
        }
    }
}

} // namespace ng
