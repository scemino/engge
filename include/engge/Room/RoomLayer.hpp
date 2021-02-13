#pragma once
#include <vector>
#include <ngf/Graphics/Texture.h>
#include <engge/Entities/Entity.hpp>
#include <engge/Graphics/SpriteSheetItem.h>

namespace ng {

class RoomLayer {
public:
  RoomLayer();
  ~RoomLayer() = default;

  void setTexture(const std::string &texture);
  void setRoomSizeY(int roomSizeY) { m_roomSizeY = roomSizeY; }
  void setOffsetY(int offsetY) { m_offsetY = offsetY; }

  std::vector<SpriteSheetItem> &getBackgrounds() { return m_backgrounds; }

  void setParallax(const glm::vec2 &parallax) { m_parallax = parallax; }
  [[nodiscard]] const glm::vec2 &getParallax() const { return m_parallax; }

  void setZOrder(int zsort) { m_zsort = zsort; }
  [[nodiscard]] int getZOrder() const { return m_zsort; }

  void addEntity(Entity &entity);
  void removeEntity(Entity &entity);

  void setEnabled(bool enabled) { m_enabled = enabled; }
  [[nodiscard]] bool isEnabled() const { return m_enabled; }

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const;
  void drawForeground(ngf::RenderTarget &target, ngf::RenderStates states) const;
  void update(const ngf::TimeSpan &elapsed);

private:
  std::string m_textureName;
  std::vector<SpriteSheetItem> m_backgrounds;
  std::vector<std::reference_wrapper<Entity>> m_entities;
  glm::vec2 m_parallax{1, 1};
  int m_zsort{0};
  bool m_enabled{true};
  int m_offsetY{0};
  int m_roomSizeY{0};
};
} // namespace ng
