#pragma once
#include "SFML/Graphics.hpp"

namespace gg
{
class GGEntity : public sf::Drawable
{
public:
  virtual void update(const sf::Time &elapsed) {}
  virtual int getZOrder() const = 0;
};
} // namespace gg
