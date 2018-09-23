#pragma once
#include <vector>
#include "SFML/Graphics.hpp"
#include "GGEntity.h"
#include "Screen.h"

namespace gg
{
class RoomLayer : public GGEntity
{
public:
  RoomLayer();
  ~RoomLayer() = default;

  std::vector<sf::Sprite> &getSprites() { return _sprites; }
  const std::vector<sf::Sprite> &getSprites() const { return _sprites; }

  void setParallax(const sf::Vector2f &parallax) { _parallax = parallax; }
  const sf::Vector2f &getParallax() const { return _parallax; }

  void setZOrder(int zsort) { _zsort = zsort; }
  int getZOrder() const override { return _zsort; }

  void draw(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const override;

private:
  std::vector<sf::Sprite> _sprites;
  sf::Vector2f _parallax;
  int _zsort;
};
} // namespace gg
