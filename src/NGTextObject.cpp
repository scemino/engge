#include "NGTextObject.h"

namespace ng
{
NGTextObject::NGTextObject(NGFont &font)
    : _font(font)
{
    _font.load("FontModernSheet");
}

void NGTextObject::draw(sf::RenderWindow &window)
{
    NGText txt;
    txt.setFont(_font);
    txt.setColor(sf::Color::White);
    txt.setText(_text);
    window.draw(txt);
}
} // namespace ng
