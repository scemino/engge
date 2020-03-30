#pragma once
#include "SFML/Graphics.hpp"

namespace ng {

class Walkbox;

class _WalkboxDrawable : public sf::Drawable {
public:
  _WalkboxDrawable(const ng::Walkbox &walkbox);

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
  const ng::Walkbox &_walkbox;
};
}
