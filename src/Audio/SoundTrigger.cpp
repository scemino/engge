#include "Audio/SoundTrigger.h"
#include "Engine/Engine.h"
#include "Audio/SoundDefinition.h"
#include "Audio/SoundId.h"
#include "Audio/SoundManager.h"

namespace ng
{
SoundTrigger::SoundTrigger(Engine &engine, const std::vector<SoundDefinition *> &sounds, Entity *pEntity)
    : _engine(engine), _distribution(0, sounds.size() - 1), _pEntity(pEntity)
{
    _name = "SoundTrigger ";
    _soundsDefinitions.resize(sounds.size());
    for (size_t i = 0; i < sounds.size(); i++)
    {
        _name.append(sounds[i]->getPath());
        _name.append(",");
         _soundsDefinitions[i] = sounds[i];
    }
    _sounds.resize(sounds.size());
    for (size_t i = 0; i < sounds.size(); i++)
    {
        _sounds[i] = 0;
    }
}

SoundTrigger::~SoundTrigger() = default;

void SoundTrigger::trigCore()
{
    int i = _distribution(_generator);
    auto pSound = _engine.getSoundManager().playSound(_soundsDefinitions[i], 1, _pEntity);
    if(!pSound) return;
    _sounds[i] = pSound->getId();
}

std::string SoundTrigger::getName() { return _name; }
} // namespace ng
