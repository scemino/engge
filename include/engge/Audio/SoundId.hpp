#pragma once
#include <memory>
#include <ngf/Audio/SoundHandle.h>
#include <engge/Engine/ChangeProperty.hpp>
#include "SoundCategory.hpp"
#include "SoundDefinition.hpp"

namespace ng {
class Entity;
class SoundManager;

class SoundId final : public Sound {
public:
  explicit SoundId(SoundManager &soundManager,
                   std::shared_ptr<SoundDefinition> soundDefinition,
                   std::shared_ptr<ngf::SoundHandle> sound,
                   SoundCategory category,
                   int entityId = 0);
  ~SoundId() final;

  std::shared_ptr<ng::SoundDefinition> getSoundDefinition() { return m_soundDefinition; }
  std::shared_ptr<ngf::SoundHandle> getSoundHandle() { return m_sound; }
  [[nodiscard]] SoundCategory getSoundCategory() const { return m_category; }

  [[nodiscard]] bool isPlaying() const;
  void stop(const ngf::TimeSpan &fadeOutTime = ngf::TimeSpan::Zero);

  void update(const ngf::TimeSpan &elapsed);

private:
  void updateVolume();

private:
  SoundManager &m_soundManager;
  std::shared_ptr<SoundDefinition> m_soundDefinition{};
  std::shared_ptr<ngf::SoundHandle> m_sound{};
  SoundCategory m_category;
  const int m_entityId{0};
};
} // namespace ng
