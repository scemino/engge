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

  void setTexture(const ngf::Texture *texture);
  void setRoomSizeY(int roomSizeY) { _roomSizeY = roomSizeY; }
  void setOffsetY(int offsetY) { _offsetY = offsetY; }

  std::vector<SpriteSheetItem> &getBackgrounds() { return _backgrounds; }

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
  const ngf::Texture *_texture{nullptr};
  std::vector<SpriteSheetItem> _backgrounds;
  std::vector<std::reference_wrapper<Entity>> _entities;
  glm::vec2 _parallax{1, 1};
  int _zsort{0};
  bool _enabled{true};
  int _offsetY{0};
  int _roomSizeY{0};
};
} // namespace ng
