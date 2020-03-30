#pragma once
#include <vector>
#include "SFML/Graphics.hpp"
#include "Entities/Entity.hpp"

namespace ng {
class RoomLayer {
public:
  RoomLayer();
  ~RoomLayer() = default;

  std::vector<sf::Sprite> &getSprites() { return _sprites; }
  [[nodiscard]] const std::vector<sf::Sprite> &getSprites() const { return _sprites; }

  void setParallax(const sf::Vector2f &parallax) { _parallax = parallax; }
  [[nodiscard]] const sf::Vector2f &getParallax() const { return _parallax; }

  void setZOrder(int zsort) { _zsort = zsort; }
  [[nodiscard]] int getZOrder() const { return _zsort; }

  void addEntity(Entity &entity);
  void removeEntity(Entity &entity);

  void setEnabled(bool enabled) { _enabled = enabled; }
  [[nodiscard]] bool isEnabled() const { return _enabled; }

  void draw(sf::RenderTarget &target, sf::RenderStates states) const;
  void drawForeground(sf::RenderTarget &target, sf::RenderStates states) const;
  void update(const sf::Time &elapsed);

private:
  std::vector<sf::Sprite> _sprites;
  std::vector<std::reference_wrapper<Entity>> _entities;
  sf::Vector2f _parallax{1, 1};
  int _zsort{0};
  bool _enabled{true};
};
} // namespace ng
