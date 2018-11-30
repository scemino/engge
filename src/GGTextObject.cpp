#include "GGTextObject.h"

namespace gg
{
GGTextObject::GGTextObject(GGFont &font)
    : _font(font)
{
    _font.load("FontModernSheet");
}

void GGTextObject::draw(sf::RenderWindow &window)
{
    GGText txt;
    txt.setFont(_font);
    txt.setColor(sf::Color::White);
    txt.setText(_text);
    window.draw(txt);
}
} // namespace gg
