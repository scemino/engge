#pragma once
#include "SFML/Graphics.hpp"
#include "Font/Font.hpp"

namespace ng
{
enum class NGTextAlignment
{
  Center,
  Left
};

class NGText : public sf::Drawable, public sf::Transformable
{
public:
  void setFont(const Font &font) { _font = font; }
  void setColor(const sf::Color &color) { _color = color; }
  void setText(const sf::String &text) { _text = text; }
  sf::String getText() const { return _text; }
  void setAlignment(NGTextAlignment alignment) { _alignment = alignment; }
  sf::FloatRect getBoundRect() const;

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
  Font _font;
  sf::Color _color;
  sf::String _text;
  NGTextAlignment _alignment{NGTextAlignment::Left};
};

} // namespace ng