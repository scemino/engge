#include "TextObject.h"
#include "Text.h"

namespace ng
{
TextObject::TextObject()
{
}

void TextObject::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    Text txt;
    txt.setFont(_font);
    txt.setFillColor(getColor());
    txt.setString(_text);
    txt.setPosition(getPosition());
    txt.scale(0.5, 0.5);
    auto bounds = txt.getGlobalBounds();
    sf::Vector2f offset;
    if (_alignment & TextAlignment::Center)
    {
        offset.x = -bounds.width / 2;
    }
    else if (_alignment & TextAlignment::Right)
    {
        offset.x = bounds.width / 2;
    }
    if (_alignment & TextAlignment::Top)
    {
        offset.x = -bounds.height / 2;
    }
    else if (_alignment & TextAlignment::Bottom)
    {
        offset.x = bounds.height / 2;
    }
    txt.move(offset);
    target.draw(txt, states);
}
} // namespace ng
