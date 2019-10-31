#include <fstream>
#include <memory>
#include "Logger.h"
#include "SoundDefinition.h"
#include "SoundId.h"
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

SoundId *SoundManager::getSoundFromId(Sound *id)
{
    for (auto &&soundId : _soundIds)
    {
        if (soundId.get() == (SoundId *)id)
            return soundId.get();
    }
    return nullptr;
}

SoundDefinition *SoundManager::getSoundDefinitionFromId(Sound *id)
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

SoundId *SoundManager::playSound(SoundDefinition *pSoundDefinition, int loopTimes, Entity* pEntity)
{
    return play(pSoundDefinition, SoundCategory::Sound, loopTimes, pEntity);
}

SoundId *SoundManager::playTalkSound(SoundDefinition *pSoundDefinition, int loopTimes, Entity* pEntity)
{
    return play(pSoundDefinition, SoundCategory::Talk, loopTimes, pEntity);
}

SoundId *SoundManager::playMusic(SoundDefinition *pSoundDefinition, int loopTimes)
{
    return play(pSoundDefinition, SoundCategory::Music, loopTimes);
}

SoundId *SoundManager::play(SoundDefinition *pSoundDefinition, SoundCategory category, int loopTimes, Entity* pEntity)
{
    auto soundId = std::make_unique<SoundId>(*this, pSoundDefinition, category);
    soundId->setEntity(pEntity);
    auto index = getSlotIndex();
    if (index == -1)
    {
        error("cannot play sound no more channel available");
        return nullptr;
    }
    std::string sCategory;
    switch (category)
    {
    case SoundCategory::Music:
        sCategory = "music";
        break;
    case SoundCategory::Sound:
        sCategory = "sound";
        break;
    case SoundCategory::Talk:
        sCategory = "talk";
        break;
    }
    trace(" [{}]loop {} {} {}", index, loopTimes, sCategory, pSoundDefinition->getPath());
    SoundId *pSoundId = soundId.get();
    _soundIds.at(index) = std::move(soundId);
    pSoundId->play(loopTimes);
    return pSoundId;
}

void SoundManager::stopAllSounds()
{
    trace("stopAllSounds");
    for (auto &_soundId : _soundIds)
    {
        if (_soundId)
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
        if (_soundId && _soundId.get() == pSound)
        {
            _soundId->stop();
            _soundId.reset();
            return;
        }
    }
}

void SoundManager::stopSound(const SoundDefinition *pSoundDef)
{
    trace("stopSound (sound definition: {})", pSoundDef->getPath());
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
    trace("setVolume (sound definition: {})", pSoundDef->getPath());
    for (size_t i = 1; i <= getSize(); i++)
    {
        auto &&sound = getSound(i);
        if (sound && pSoundDef == sound->getSoundDefinition())
        {
            sound->setVolume(volume);
        }
    }
}

void SoundManager::update(const sf::Time &elapsed)
{
    for (auto &&soundId : _soundIds)
    {
        if (soundId)
        {
            soundId->update(elapsed);
        }
    }
}

void SoundManager::pauseAllSounds()
{
    for (auto &&soundId : _soundIds)
    {
        if (soundId)
        {
            soundId->pause();
        }
    }
}

void SoundManager::resumeAllSounds()
{
    for (auto &&soundId : _soundIds)
    {
        if (soundId)
        {
            soundId->resume();
        }
    }
}
} // namespace ng
