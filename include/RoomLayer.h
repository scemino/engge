#pragma once
#include <vector>
#include "SFML/Graphics.hpp"
#include "GGEntity.h"

namespace gg
{
class RoomLayer : public GGEntity
{
public:
  RoomLayer() : _zsort(0), _parallax(1, 0) {}
  ~RoomLayer() {}

  std::vector<sf::Sprite> &getSprites() { return _sprites; }
  const std::vector<sf::Sprite> &getSprites() const { return _sprites; }

  void setParallax(const sf::Vector2f &parallax) { _parallax = parallax; }
  const sf::Vector2f &getParallax() const { return _parallax; }

  void setZOrder(int zsort) { _zsort = zsort; }
  int getZOrder() const { return _zsort; }

  virtual void draw(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const
  {
    for (const auto &s : getSprites())
    {
      auto sprite(s);
      auto parallax = getParallax();
      auto pos = (160.f - cameraPos.x) * parallax.x - 160.f;
      sprite.move(pos, -cameraPos.y);
      window.draw(sprite);
    }
  }

private:
  std::vector<std::string> _names;
  std::vector<sf::Sprite> _sprites;
  sf::Vector2f _parallax;
  int _zsort;
};
} // namespace gg
