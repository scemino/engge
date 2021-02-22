#include <utility>
#include "engge/Engine/EngineSettings.hpp"
#include "engge/System/Locator.hpp"
#include "engge/System/Logger.hpp"
#include "engge/Engine/EntityManager.hpp"
#include "engge/Audio/SoundDefinition.hpp"

namespace ng {
Sound::~Sound() = default;

SoundDefinition::SoundDefinition(std::string path)
    : m_path(std::move(path)) {
  m_id = Locator<EntityManager>::get().getSoundId();
}

SoundDefinition::~SoundDefinition() = default;

void SoundDefinition::load() {
  if (m_isLoaded)
    return;
  auto buffer = Locator<EngineSettings>::get().readBuffer(m_path);
  m_buffer.loadFromMemory(buffer.data(), buffer.size());
  m_isLoaded = true;
}

} // namespace ng
