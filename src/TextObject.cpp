#include "Screen.h"
#include "TextObject.h"
#include "Text.h"
#include "_NGUtil.h"

namespace ng
{
TextAlignment operator|=(TextAlignment &lhs, TextAlignment rhs)
{
    lhs = static_cast<TextAlignment>(
        static_cast<std::underlying_type<TextAlignment>::type>(lhs) |
        static_cast<std::underlying_type<TextAlignment>::type>(rhs));
    return lhs;
}

bool operator&(TextAlignment lhs, TextAlignment rhs)
{
    return static_cast<TextAlignment>(
               static_cast<std::underlying_type<TextAlignment>::type>(lhs) &
               static_cast<std::underlying_type<TextAlignment>::type>(rhs)) > TextAlignment::None;
}

TextObject::TextObject()
    : _alignment(TextAlignment::Left)
{
}

void TextObject::setText(const std::string &text)
{
    _text = towstring(text);
}

void TextObject::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    Text txt;
    txt.setFont(_font);
    txt.setFillColor(getColor());
    txt.setString(_text);
    txt.setPosition(getPosition());
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
        offset.y = -bounds.height / 2;
    }
    else if (_alignment & TextAlignment::Bottom)
    {
        offset.y = bounds.height / 2;
    }
    txt.move(offset);
    states.transform *= _transform.getTransform();
    target.draw(txt, states);
}
} // namespace ng
