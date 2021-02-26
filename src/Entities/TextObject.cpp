#include <engge/Entities/TextObject.hpp>
#include <ngf/Graphics/Text.h>
#include <engge/Room/Room.hpp>
#include <engge/Graphics/Text.hpp>
#include "Util/Util.hpp"

namespace ng {
namespace {
bool operator&(TextAlignment lhs, TextAlignment rhs) {
  return static_cast<TextAlignment>(
      static_cast<std::underlying_type<TextAlignment>::type>(lhs) &
          static_cast<std::underlying_type<TextAlignment>::type>(rhs)) > TextAlignment::None;
}

ngf::Anchor toAnchor(TextAlignment alignment) {
  auto anchor = static_cast<int>(ngf::Anchor::CenterLeft);
  if (alignment & TextAlignment::Center) {
    anchor = static_cast<int>(ngf::Anchor::Center);
  } else if (alignment & TextAlignment::Right) {
    anchor = static_cast<int>(ngf::Anchor::CenterRight);
  }
  if (alignment & TextAlignment::Top) {
    anchor -= 3;
  } else if (alignment & TextAlignment::Bottom) {
    anchor += 3;
  }
  return static_cast<ngf::Anchor>(anchor);
}
}

TextObject::TextObject() {
  setTemporary(true);
}

TextObject::~TextObject() = default;

void TextObject::setText(const std::string &text) {
  m_text = towstring(text);
}

std::string TextObject::getText() const {
  return tostring(m_text);
}

void TextObject::draw(ngf::RenderTarget &target, ngf::RenderStates states) const {
  if (!isVisible())
    return;

  const auto view = target.getView();
  if (getScreenSpace() == ScreenSpace::Object) {
    target.setView(ngf::View(ngf::frect::fromPositionSize({0, 0}, {Screen::Width, Screen::Height})));
  }

  ng::Text txt;
  txt.setFont(*m_font);
  txt.setColor(getColor());
  txt.setMaxWidth(static_cast<float>(m_maxWidth));
  txt.setWideString(m_text);
  txt.setAnchor(toAnchor(m_alignment));

  auto height = target.getView().getSize().y;
  auto transformable = getTransform();
  transformable.setPosition({transformable.getPosition().x, height - transformable.getPosition().y});

  if (getScreenSpace() == ScreenSpace::Object) {
    ngf::RenderStates s;
    s.transform = transformable.getTransform();
    txt.draw(target, s);
    target.setView(view);
  } else {
    states.transform = transformable.getTransform() * states.transform;
    txt.draw(target, states);
  }
}

} // namespace ng
