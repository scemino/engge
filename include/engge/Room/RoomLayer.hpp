#pragma once
#include <vector>
#include "engge/Entities/Entity.hpp"

namespace ng {

struct Background {
public:
  Background(const glm::ivec2 &pos, std::string texture, const ngf::irect &rect)
      : m_pos(pos), m_texture(std::move(texture)), m_rect(rect) {
  }

public:
  glm::ivec2 m_pos;
  std::string m_texture;
  ngf::irect m_rect;
};

class RoomLayer {
public:
  RoomLayer();
  ~RoomLayer() = default;

  std::vector<Background> &getBackgrounds() { return _backgrounds; }

  void setParallax(const glm::vec2 &parallax) { _parallax = parallax; }
  [[nodiscard]] const glm::vec2 &getParallax() const { return _parallax; }

  void setZOrder(int zsort) { _zsort = zsort; }
  [[nodiscard]] int getZOrder() const { return _zsort; }

  void addEntity(Entity &entity);
  void removeEntity(Entity &entity);

  void setEnabled(bool enabled) { _enabled = enabled; }
  [[nodiscard]] bool isEnabled() const { return _enabled; }

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const;
  void drawForeground(ngf::RenderTarget &target, ngf::RenderStates states) const;
  void update(const ngf::TimeSpan &elapsed);

private:
  std::vector<Background> _backgrounds;
  std::vector<std::reference_wrapper<Entity>> _entities;
  glm::vec2 _parallax{1, 1};
  int _zsort{0};
  bool _enabled{true};
};
} // namespace ng
