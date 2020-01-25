#include "Audio/SoundTrigger.hpp"
#include "Engine/Engine.hpp"
#include "Audio/SoundDefinition.hpp"
#include "Audio/SoundId.hpp"
#include "Audio/SoundManager.hpp"

namespace ng
{
SoundTrigger::SoundTrigger(Engine &engine, const std::vector<SoundDefinition *> &sounds, Entity *pEntity)
    : _engine(engine), _pEntity(pEntity), _distribution(0, sounds.size() - 1)
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
