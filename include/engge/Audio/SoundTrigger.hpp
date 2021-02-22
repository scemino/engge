#pragma once
#include <memory>
#include <random>
#include <vector>
#include "engge/Engine/Trigger.hpp"

namespace ng {
class Entity;
class Engine;
class SoundDefinition;
class SoundId;
class SoundTrigger final : public Trigger {
public:
  SoundTrigger(Engine &engine, const std::vector<std::shared_ptr<SoundDefinition>> &sounds, int id);
  ~SoundTrigger() final;

  std::string getName() final;

private:
  void trigCore() final;

private:
  Engine &m_engine;
  int m_id{0};
  std::vector<std::shared_ptr<SoundDefinition>> m_soundsDefinitions;
  std::vector<int> m_sounds;
  std::default_random_engine m_defaultRandomEngine;
  std::uniform_int_distribution<int> m_distribution;
  std::string m_name;
};
}
