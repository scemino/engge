#include "Control.hpp"
#include <ngf/System/Mouse.h>
#include <imgui.h>
#include <engge/Audio/SoundManager.hpp>
#include <engge/Engine/Engine.hpp>

namespace ng {
Control::Control(bool enabled) :
    m_state(enabled ? ControlState::None : ControlState::Disabled) {
}

Control::~Control() = default;

void Control::update(glm::vec2 pos) {
  if (m_state == ControlState::Disabled)
    return;

  auto oldState = m_state;
  m_state = ControlState::None;
  if (contains((glm::vec2) pos)) {
    m_state = ControlState::Hover;
    auto isDown = ngf::Mouse::isButtonPressed(ngf::Mouse::Button::Left);
    auto &io = ImGui::GetIO();
    if (!io.WantCaptureMouse && m_wasMouseDown && !isDown) {
      onClick();
    }
    m_wasMouseDown = isDown;
  }

  if (m_state != oldState) {
    if (m_state == ControlState::Hover) {
      auto pSound = m_pEngine->getSoundManager().getSoundHover();
      if (pSound) {
        m_pEngine->getSoundManager().playSound(pSound);
      }
    }
    onStateChanged();
  }
}

void Control::setEngine(Engine *pEngine) {
  m_pEngine = pEngine;
  onEngineSet();
}
}