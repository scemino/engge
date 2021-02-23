#include "SwitchButton.hpp"
#include <engge/Engine/Engine.hpp>
#include <ngf/Graphics/FntFont.h>
#include "Util/Util.hpp"
#include "ControlConstants.hpp"
#include <ngf/System/Mouse.h>
#include <imgui.h>

namespace ng {
SwitchButton::SwitchButton(std::initializer_list<int> ids,
                           float y, bool enabled, int index, Callback callback)
    : Control(enabled), m_ids(ids), m_index(index), m_y(y), m_callback(std::move(callback)) {
}

void SwitchButton::draw(ngf::RenderTarget &target, ngf::RenderStates states) const {
  m_text.draw(target, states);
}

void SwitchButton::onEngineSet() {
  auto &uiFontMedium = m_pEngine->getResourceManager().getFntFont("UIFontMedium.fnt");
  m_text.setFont(uiFontMedium);
  m_text.setWideString(Engine::getText(m_ids[m_index]));
  auto textRect = m_text.getLocalBounds();
  m_text.getTransform().setOrigin({textRect.getWidth() / 2.f, 0});
  m_text.getTransform().setPosition({Screen::Width / 2.f, m_y});
}

bool SwitchButton::contains(glm::vec2 pos) const {
  auto textRect = ng::getGlobalBounds(m_text);
  return textRect.contains(pos);
}

void SwitchButton::onClick() {
  m_index = (m_index + 1) % static_cast<int>(m_ids.size());
  m_text.setWideString(Engine::getText(m_ids[m_index]));
  if (m_callback) {
    m_callback(m_index);
  }
  auto textRect = m_text.getLocalBounds();
  m_text.getTransform().setOrigin({textRect.getWidth() / 2.f, 0});
}

void SwitchButton::onStateChanged() {
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

void SwitchButton::update(const ngf::TimeSpan &elapsed, glm::vec2 pos) {
  Control::update(elapsed, pos);
  m_text.getTransform().setPosition(m_shakeOffset + glm::vec2{Screen::Width / 2.f, m_y});
}
}