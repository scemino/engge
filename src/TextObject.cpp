#include "TextObject.h"
#include "Text.h"

namespace ng
{
TextObject::TextObject()
{
}

void TextObject::draw(sf::RenderWindow &window)
{
    Text txt;
    txt.setFont(_font);
    txt.setFillColor(sf::Color::White);
    txt.setString(_text);
    window.draw(txt);
}
} // namespace ng
