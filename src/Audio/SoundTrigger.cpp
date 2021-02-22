#include "engge/Audio/SoundTrigger.hpp"
#include "engge/Engine/Engine.hpp"
#include "engge/Audio/SoundDefinition.hpp"
#include "engge/Audio/SoundId.hpp"
#include "engge/Audio/SoundManager.hpp"

namespace ng {
SoundTrigger::SoundTrigger(Engine &engine, const std::vector<std::shared_ptr<SoundDefinition>> &sounds, int id)
    : m_engine(engine), m_id(id), m_distribution(0, sounds.size() - 1) {
  m_name = "SoundTrigger ";
  m_soundsDefinitions.resize(sounds.size());
  for (size_t i = 0; i < sounds.size(); i++) {
    m_name.append(sounds[i]->getPath());
    m_name.append(",");
    m_soundsDefinitions[i] = sounds[i];
  }
  m_sounds.resize(sounds.size());
  for (size_t i = 0; i < sounds.size(); i++) {
    m_sounds[i] = 0;
  }
}

SoundTrigger::~SoundTrigger() = default;

void SoundTrigger::trigCore() {
  int i = m_distribution(m_defaultRandomEngine);
  auto pSound = m_engine.getSoundManager().playSound(m_soundsDefinitions[i], 1, ngf::TimeSpan::Zero, m_id);
  if (!pSound)
    return;
  m_sounds[i] = pSound->getId();
}

std::string SoundTrigger::getName() { return m_name; }
} // namespace ng
