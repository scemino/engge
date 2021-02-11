#pragma once
#include <optional>
#include <vector>
#include <glm/vec2.hpp>
#include <ngf/System/TimeSpan.h>
#include <engge/Entities/Facing.hpp>

namespace ng {
class Actor;

class WalkingState final {
public:
  void setActor(Actor *pActor);
  void setDestination(const std::vector<glm::vec2> &path, std::optional<Facing> facing);
  void update(const ngf::TimeSpan &elapsed);
  void stop();

  [[nodiscard]] inline bool isWalking() const { return m_isWalking; }

private:
  Facing getFacing();

private:
  Actor *m_pActor{nullptr};
  std::vector<glm::vec2> m_path;
  std::optional<Facing> m_facing{Facing::FACE_FRONT};
  bool m_isWalking{false};
  glm::vec2 m_init{0, 0};
  ngf::TimeSpan m_elapsed;
};
}