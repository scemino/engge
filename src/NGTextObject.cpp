#include "NGTextObject.h"
#include "Text.h"

namespace ng
{
NGTextObject::NGTextObject()
{
}

void NGTextObject::draw(sf::RenderWindow &window)
{
    Text txt;
    txt.setFont(_font);
    txt.setFillColor(sf::Color::White);
    txt.setString(_text);
    window.draw(txt);
}
} // namespace ng
