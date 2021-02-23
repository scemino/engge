#include "Button.hpp"
#include <engge/Engine/Engine.hpp>
#include <engge/Audio/SoundManager.hpp>
#include <utility>
#include <ngf/System/Mouse.h>
#include <ngf/Graphics/RectangleShape.h>
#include <ngf/Graphics/FntFont.h>
#include "ControlConstants.hpp"
#include "Util/Util.hpp"

namespace ng {
Button::Button(int id, float y, Callback callback, bool enabled, Size size)
    : Control(enabled), m_callback(std::move(callback)), m_id(id), m_y(y), m_size(size) {
  m_text.setColor(enabled ? ControlConstants::NormalColor : ControlConstants::DisabledColor);
}

Button::~Button() = default;

void Button::onClick() {
  if (m_callback) {
    m_callback();
  }
}

void Button::draw(ngf::RenderTarget &target, ngf::RenderStates states) const {
  m_text.draw(target, states);
}

void Button::onEngineSet() {
  const auto &uiFontLargeOrMedium =
      m_pEngine->getResourceManager().getFntFont(m_size == Size::Large ? "UIFontLarge.fnt" : "UIFontMedium.fnt");
  m_text.setFont(uiFontLargeOrMedium);
  m_text.setWideString(ng::Engine::getText(m_id));
  auto textRect = m_text.getLocalBounds();
  m_text.getTransform().setOrigin({textRect.getWidth() / 2.f, 0});
  m_text.getTransform().setPosition({Screen::Width / 2.f, m_y});
}

void Button::onStateChanged() {
  ngf::Color color;
  switch (m_state) {
  case ControlState::Disabled:color = ControlConstants::DisabledColor;
    break;
  case ControlState::None:color = ControlConstants::NormalColor;
    break;
  case ControlState::Hover:color = ControlConstants::HoverColor;
    break;
  }
  m_text.setColor(color);
}

bool Button::contains(glm::vec2 pos) const {
  auto p = ngf::transform(glm::inverse(m_text.getTransform().getTransform()), pos);
  return m_text.getLocalBounds().contains(p);
}

void Button::update(const ngf::TimeSpan &elapsed, glm::vec2 pos) {
  Control::update(elapsed, pos);
  m_text.getTransform().setPosition(m_shakeOffset + glm::vec2{Screen::Width / 2.f, m_y});
}
}