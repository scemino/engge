#include "Slider.hpp"
#include <engge/Engine/Engine.hpp>
#include <ngf/Graphics/FntFont.h>
#include "ControlConstants.hpp"
#include <ngf/System/Mouse.h>
#include "Util/Util.hpp"
#include <imgui.h>

namespace ng {
Slider::Slider(int id, float y, bool enabled, float value, Callback callback)
    : Control(enabled), m_id(id), m_y(y), m_value(value), m_onValueChanged(callback) {
}

void Slider::setSpriteSheet(SpriteSheet *pSpriteSheet) {
  const auto &uiFontMedium = m_pEngine->getResourceManager().getFntFont("UIFontMedium.fnt");
  m_text.setFont(uiFontMedium);
  m_text.setWideString(Engine::getText(m_id));
  auto textRect = m_text.getLocalBounds();
  m_text.getTransform().setOrigin({textRect.getWidth() / 2.f, textRect.getHeight()});
  m_text.getTransform().setPosition({Screen::Width / 2.f, m_y});

  auto sliderRect = pSpriteSheet->getRect("slider");
  auto handleRect = pSpriteSheet->getRect("slider_handle");
  glm::vec2 scale(Screen::Width / 320.f, Screen::Height / 180.f);
  m_sprite.getTransform().setPosition({Screen::Width / 2.f, m_y});
  m_sprite.getTransform().setScale(scale);
  m_sprite.getTransform().setOrigin({sliderRect.getWidth() / 2.f, 0});
  m_sprite.setTexture(*pSpriteSheet->getTexture());
  m_sprite.setTextureRect(sliderRect);

  m_min = Screen::Width / 2.f - (sliderRect.getWidth() * scale.x / 2.f);
  m_max = Screen::Width / 2.f + (sliderRect.getWidth() * scale.x / 2.f);
  auto x = m_min + m_value * (m_max - m_min);
  m_spriteHandle.getTransform().setPosition({x, m_y});
  m_spriteHandle.getTransform().setScale(scale);
  m_spriteHandle.getTransform().setOrigin({handleRect.getWidth() / 2.f, 0});
  m_spriteHandle.setTexture(*pSpriteSheet->getTexture());
  m_spriteHandle.setTextureRect(handleRect);
}

bool Slider::contains(glm::vec2 pos) const {
  auto textRect = ng::getGlobalBounds(m_sprite);
  return textRect.contains(pos);
}

void Slider::update(const ngf::TimeSpan &elapsed, glm::vec2 pos) {
  Control::update(elapsed, pos);

  auto textRect = ng::getGlobalBounds(m_sprite);
  bool isDown = ngf::Mouse::isButtonPressed(ngf::Mouse::Button::Left);
  if (!isDown) {
    m_isDragging = false;
  }
  ngf::Color color;
  if (m_state == ControlState::Disabled) {
    color = ControlConstants::DisabledColor;
  } else if (textRect.contains(pos)) {
    color = ControlConstants::HoverColor;
    ImGuiIO &io = ImGui::GetIO();
    if (!io.WantCaptureMouse && isDown) {
      m_isDragging = true;
    }
  } else {
    m_isDragging = false;
    color = ControlConstants::NormalColor;
  }
  m_sprite.setColor(color);
  m_text.setColor(color);

  if (m_isDragging) {
    auto x = std::clamp(pos.x, m_min, m_max);
    auto value = (x - m_min) / (m_max - m_min);
    if (m_value != value) {
      m_value = value;
      if (m_onValueChanged) {
        m_onValueChanged.value()(value);
      }
    }
    m_spriteHandle.getTransform().setPosition({x, m_y});
  } else if (m_state == ControlState::Hover) {
    m_text.getTransform().setPosition(m_shakeOffset + glm::vec2{Screen::Width / 2.f, m_y});
    m_sprite.getTransform().setPosition(m_shakeOffset + glm::vec2{Screen::Width / 2.f, m_y});
    m_spriteHandle.getTransform().setPosition(m_shakeOffset + glm::vec2{m_min + m_value * (m_max - m_min), m_y});
  }
}

void Slider::draw(ngf::RenderTarget &target, ngf::RenderStates states) const {
  m_text.draw(target, states);
  m_sprite.draw(target, states);
  m_spriteHandle.draw(target, states);
}
}