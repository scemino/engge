#pragma once
#include "Object.h"
#include "FntFont.h"
#include "SFML/Graphics.hpp"

namespace ng
{
class TextObject : public Object
{
public:
  explicit TextObject();
  FntFont &getFont() { return _font; }
  void setText(const std::string &text) { _text = text; }

private:
  virtual void draw(sf::RenderWindow &window);

private:
  FntFont _font;
  std::string _text;
};
} // namespace ng
