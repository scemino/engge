#include "Engine.h"
#include "SoundDefinition.h"
#include "SoundManager.h"
#include "SoundTrigger.h"

namespace ng
{
SoundTrigger::SoundTrigger(Engine &engine, const std::vector<SoundDefinition *> &sounds, Entity* pEntity)
    : _engine(engine), _distribution(0, sounds.size() - 1), _pEntity(pEntity)
{
    _soundsDefinitions.resize(sounds.size());
    for (size_t i = 0; i < sounds.size(); i++)
    {
        _soundsDefinitions[i] = sounds[i];
    }
    _sounds.resize(sounds.size());
    for (size_t i = 0; i < sounds.size(); i++)
    {
        _sounds[i] = nullptr;
    }
}

SoundTrigger::~SoundTrigger() = default;

void SoundTrigger::trigCore()
{
    int i = _distribution(_generator);
    _sounds[i] = _engine.getSoundManager().playSound(_soundsDefinitions[i], 1, _pEntity);
}
} // namespace ng
