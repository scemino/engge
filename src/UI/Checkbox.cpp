#include "Checkbox.hpp"
#include <engge/Engine/Engine.hpp>
#include "Util/Util.hpp"
#include "ControlConstants.hpp"
#include <ngf/Graphics/FntFont.h>
#include <ngf/System/Mouse.h>

namespace ng {
Checkbox::Checkbox(int id, float y, bool enabled, bool checked, Callback callback)
    : Control(enabled), m_callback(callback), m_id(id), m_y(y), m_isChecked(checked) {
}

void Checkbox::draw(ngf::RenderTarget &target, ngf::RenderStates states) const {
  m_text.draw(target, states);
  m_sprite.draw(target, states);
}

void Checkbox::onEngineSet() {
  const auto &uiFontMedium = m_pEngine->getResourceManager().getFntFont("UIFontMedium.fnt");
  m_text.setFont(uiFontMedium);
  m_text.setWideString(ng::Engine::getText(m_id));
  auto textRect = m_text.getLocalBounds();
  m_text.getTransform().setOrigin(glm::vec2(0, textRect.getHeight() / 2.f));
  m_text.getTransform().setPosition({420.f, m_y});
}

void Checkbox::setSpriteSheet(SpriteSheet *pSpriteSheet) {
  m_pSpriteSheet = pSpriteSheet;
  auto checkedRect = pSpriteSheet->getRect("option_unchecked");
  m_sprite.getTransform().setPosition({820.f, m_y});
  glm::vec2 scale(Screen::Width / 320.f, Screen::Height / 180.f);
  m_sprite.getTransform().setScale(scale);
  m_sprite.getTransform().setOrigin({checkedRect.getWidth() / 2.f, checkedRect.getHeight() / 2.f});
  m_sprite.setTexture(*pSpriteSheet->getTexture());
  m_sprite.setTextureRect(checkedRect);

  updateCheckState();
}

void Checkbox::setChecked(bool checked) {
  if (m_isChecked != checked) {
    m_isChecked = checked;
    if (m_callback) {
      m_callback(checked);
    }
    updateCheckState();
  }
}

bool Checkbox::contains(glm::vec2 pos) const {
  auto textRect = ng::getGlobalBounds(m_sprite);
  return textRect.contains(pos);
}

void Checkbox::updateCheckState() {
  ngf::Color color;
  switch (m_state) {
  case ControlState::Disabled:color = ControlConstants::DisabledColor;
    break;
  case ControlState::None:color = ControlConstants::NormalColor;
    break;
  case ControlState::Hover:color = ControlConstants::HoverColor;
    break;
  }

  m_sprite.setColor(color);
  m_text.setColor(color);

  auto checkedRect =
      m_isChecked ? m_pSpriteSheet->getRect("option_checked") : m_pSpriteSheet->getRect("option_unchecked");
  m_sprite.setTextureRect(checkedRect);
}

void Checkbox::onStateChanged() {
  updateCheckState();
}

void Checkbox::onClick() {
  setChecked(!m_isChecked);
}

void Checkbox::update(const ngf::TimeSpan &elapsed, glm::vec2 pos) {
  Control::update(elapsed, pos);
  m_sprite.getTransform().setPosition(m_shakeOffset + glm::vec2{820.f, m_y});
  m_text.getTransform().setPosition(m_shakeOffset + glm::vec2{420.f, m_y});
}
}