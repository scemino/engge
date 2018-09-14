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
    _font.draw(_text, window);
}
}
