#pragma once
#include "GGObject.h"
#include "GGFont.h"
#include "SFML/Graphics.hpp"

namespace gg
{
class GGTextObject : public GGObject
{
public:
  explicit GGTextObject(GGFont &font);
  void setText(const std::string &text) { _text = text; }

private:
  virtual void draw(sf::RenderWindow &window);

private:
  GGFont &_font;
  std::string _text;
};
} // namespace gg
