#pragma once
#include "NGObject.h"
#include "FntFont.h"
#include "SFML/Graphics.hpp"

namespace ng
{
class NGTextObject : public NGObject
{
public:
  explicit NGTextObject();
  FntFont &getFont() { return _font; }
  void setText(const std::string &text) { _text = text; }

private:
  virtual void draw(sf::RenderWindow &window);

private:
  FntFont _font;
  std::string _text;
};
} // namespace ng
