#pragma once
#include <random>
#include <vector>
#include "engge/Engine/Trigger.hpp"

namespace ng {
class Entity;
class Engine;
class SoundDefinition;
class SoundId;
class SoundTrigger : public Trigger {
public:
  SoundTrigger(Engine &engine, const std::vector<SoundDefinition *> &sounds, int id);
  ~SoundTrigger() override;

  std::string getName() override;

private:
  void trigCore() override;

private:
  Engine &_engine;
  int _id{0};
  std::vector<SoundDefinition *> _soundsDefinitions;
  std::vector<int> _sounds;
  std::default_random_engine _generator;
  std::uniform_int_distribution<int> _distribution;
  std::string _name;
};
}
