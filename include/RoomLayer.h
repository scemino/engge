#pragma once
#include <vector>
#include "SFML/Graphics.hpp"
#include "NGEntity.h"
#include "Screen.h"

namespace ng
{
class RoomLayer
{
public:
  RoomLayer();
  ~RoomLayer() = default;

  std::vector<sf::Sprite> &getSprites() { return _sprites; }
  const std::vector<sf::Sprite> &getSprites() const { return _sprites; }

  void setParallax(const sf::Vector2f &parallax) { _parallax = parallax; }
  const sf::Vector2f &getParallax() const { return _parallax; }

  void setZOrder(int zsort) { _zsort = zsort; }
  int getZOrder() const { return _zsort; }

  void addEntity(NGEntity &entity);
  void removeEntity(NGEntity &entity);

  void draw(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const;
  void update(const sf::Time &elapsed);

private:
  std::vector<sf::Sprite> _sprites;
  std::vector<std::reference_wrapper<NGEntity>> _entities;
  sf::Vector2f _parallax;
  int _zsort;
};
} // namespace ng
