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

void Control::update(const ngf::TimeSpan &elapsed, glm::vec2 pos) {
  if (m_state == ControlState::Disabled)
    return;

  // update state
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

  // play sound cursor is over it
  if (m_state != oldState) {
    if (m_state == ControlState::Hover) {
      m_shake = true;
      m_shakeTime = 0;
      auto pSound = m_pEngine->getSoundManager().getSoundHover();
      if (pSound) {
        m_pEngine->getSoundManager().playSound(pSound);
      }
    } else {
      m_shake = false;
    }
    onStateChanged();
  }

  // update shake offset
  if (m_shake && m_state == ControlState::Hover) {
    m_shakeTime += 20.f * elapsed.getTotalSeconds();
    m_shakeOffset = {0.6f * cosf(m_shakeTime + 0.3f), 0.6f * sinf(m_shakeTime)};
    m_shake = m_shakeTime < 10.f;
  }
}

void Control::setEngine(Engine *pEngine) {
  m_pEngine = pEngine;
  onEngineSet();
}
}