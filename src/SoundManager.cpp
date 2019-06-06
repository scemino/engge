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

SoundId *SoundManager::getSound(size_t index)
{
    if (index < 1 || index > _soundIds.size())
        return nullptr;
    return _soundIds.at(index - 1).get();
}

SoundId *SoundManager::getSoundFromId(Sound* id)
{
    for (auto &&soundId : _soundIds)
    {
        if (soundId.get() == (SoundId *)id)
            return soundId.get();
    }
    return nullptr;
}

SoundDefinition *SoundManager::getSoundDefinitionFromId(Sound* id)
{
    for (auto &&soundDef : _sounds)
    {
        if (soundDef.get() == (SoundDefinition *)id)
            return soundDef.get();
    }
    return nullptr;
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

SoundDefinition *SoundManager::defineSound(const std::string &name)
{
    if (!_settings.hasEntry(name))
        return nullptr;

    auto sound = std::make_unique<SoundDefinition>(name);
    sound->setSettings(_settings);
    auto pSound = sound.get();
    _sounds.push_back(std::move(sound));
    return pSound;
}

SoundId *SoundManager::playSound(SoundDefinition *pSoundDefinition, bool loop)
{
    auto soundId = std::make_unique<SoundId>(pSoundDefinition);
    auto index = getSlotIndex();
    if (index == -1)
    {
        std::cerr << "cannot play sound no more channel available" << std::endl;
        return nullptr;
    }
    std::cout << " [" << index << "]"
              << "play sound " << pSoundDefinition->getPath() << std::endl;
    SoundId *pSoundId = soundId.get();
    _soundIds.at(index) = std::move(soundId);
    pSoundId->play(loop);
    return pSoundId;
}

SoundId *SoundManager::loopMusic(SoundDefinition *pSoundDefinition)
{
    auto soundId = std::make_unique<SoundId>(pSoundDefinition);
    auto index = getSlotIndex();
    if (index == -1)
    {
        std::cerr << "cannot play sound no more channel available" << std::endl;
        return nullptr;
    }
    std::cout << " [" << index << "]"
              << "loop music " << pSoundDefinition->getPath() << std::endl;
    SoundId *pSoundId = soundId.get();
    _soundIds.at(index) = std::move(soundId);
    pSoundId->play(true);
    return pSoundId;
}

void SoundManager::stopAllSounds()
{
    std::cout << "stopAllSounds" << std::endl;
    for (auto &_soundId : _soundIds)
    {
        if (_soundId != nullptr)
        {
            _soundId->stop();
            _soundId.reset();
        }
    }
}

void SoundManager::stopSound(SoundId *pSound)
{
    if (!pSound)
        return;
    for (auto &_soundId : _soundIds)
    {
        if (_soundId != nullptr && _soundId.get() == pSound)
        {
            _soundId->stop();
            _soundId.reset();
            return;
        }
    }
}

void SoundManager::stopSound(const SoundDefinition *pSoundDef)
{
    std::cout << "stopSound (sound definition: " << pSoundDef->getPath() << ")" << std::endl;
    for (size_t i = 1; i <= getSize(); i++)
    {
        auto &&sound = getSound(i);
        if (sound && pSoundDef == sound->getSoundDefinition())
        {
            stopSound(sound);
        }
    }
}

void SoundManager::setVolume(const SoundDefinition *pSoundDef, float volume)
{
    std::cout << "setVolume (sound definition: " << pSoundDef->getPath() << ")" << std::endl;
    for (size_t i = 1; i <= getSize(); i++)
    {
        auto &&sound = getSound(i);
        if (sound && pSoundDef == sound->getSoundDefinition())
        {
            sound->setVolume(volume);
        }
    }
}

} // namespace ng
