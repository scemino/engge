#pragma once
#include "SFML/Graphics.hpp"

namespace gg
{
class GGEntity
{
public:
  virtual void draw(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const = 0;
  virtual int getZOrder() const = 0;
};
} // namespace gg
