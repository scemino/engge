#include "engge/Entities/Objects/TextObject.hpp"
#include "engge/Graphics/Text.hpp"
#include "../../System/_Util.hpp"

namespace ng {
TextAlignment operator|=(TextAlignment &lhs, TextAlignment rhs) {
  lhs = static_cast<TextAlignment>(
      static_cast<std::underlying_type<TextAlignment>::type>(lhs) |
          static_cast<std::underlying_type<TextAlignment>::type>(rhs));
  return lhs;
}

bool operator&(TextAlignment lhs, TextAlignment rhs) {
  return static_cast<TextAlignment>(
      static_cast<std::underlying_type<TextAlignment>::type>(lhs) &
          static_cast<std::underlying_type<TextAlignment>::type>(rhs)) > TextAlignment::None;
}

TextObject::TextObject()
    : _alignment(TextAlignment::Left) {
  setTemporary(true);
}

TextObject::~TextObject() = default;

void TextObject::setText(const std::string &text) {
  _text = towstring(text);
}

void TextObject::draw(sf::RenderTarget &target, sf::RenderStates states) const {
  if (!isVisible())
    return;

  const auto view = target.getView();
  if (getScreenSpace() == ScreenSpace::Object) {
    target.setView(sf::View(sf::FloatRect(0, 0, Screen::Width, Screen::Height)));
  }

  Text txt;
  txt.setFont(_font);
  txt.setFillColor(getColor());
  txt.setString(_text);
  txt.setMaxWidth(static_cast<float>(_maxWidth));
  auto bounds = txt.getLocalBounds();
  sf::Vector2f offset;
  if (_alignment & TextAlignment::Center) {
    offset.x = getScale() * -bounds.width / 2;
  } else if (_alignment & TextAlignment::Right) {
    offset.x = getScale() * bounds.width / 2;
  }
  if (_alignment & TextAlignment::Top) {
    offset.y = 0;
  } else if (_alignment & TextAlignment::Bottom) {
    offset.y = getScale() * bounds.height;
  } else {
    offset.y = getScale() * bounds.height / 2;
  }
  auto height = target.getView().getSize().y;
  auto transformable = getTransform();
  transformable.move(offset.x, offset.y);
  transformable.setPosition(transformable.getPosition().x, height - transformable.getPosition().y);

  if (getScreenSpace() == ScreenSpace::Object) {
    sf::RenderStates s;
    s.transform *= transformable.getTransform();
    target.draw(txt, s);
    target.setView(view);
  } else {
    states.transform *= transformable.getTransform();
    target.draw(txt, states);
  }
}

} // namespace ng
