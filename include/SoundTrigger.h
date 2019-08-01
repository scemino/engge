#pragma once
#include <random>
#include <vector>
#include "Trigger.h"

namespace ng
{
class Engine;
class SoundDefinition;
class SoundId;
class SoundTrigger : public Trigger
{
public:
    SoundTrigger(Engine &engine, const std::vector<SoundDefinition*> &sounds);
    ~SoundTrigger() override;

private:
    void trigCore() override;

private:
    Engine &_engine;
    std::vector<SoundDefinition*> _soundsDefinitions;
    std::vector<SoundId*> _sounds;
    std::default_random_engine _generator;
    std::uniform_int_distribution<int> _distribution;
};
}
