#pragma once
#include "NGObject.h"
#include "NGFont.h"
#include "SFML/Graphics.hpp"

namespace ng
{
class NGTextObject : public NGObject
{
public:
  explicit NGTextObject(NGFont &font);
  void setText(const std::string &text) { _text = text; }

private:
  virtual void draw(sf::RenderWindow &window);

private:
  NGFont &_font;
  std::string _text;
};
} // namespace ng
