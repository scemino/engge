#include "CameraTools.hpp"
#include <ngf/Graphics/ImGuiExtensions.h>
#include <engge/Engine/Engine.hpp>
#include <engge/Engine/Camera.hpp>

namespace ng {
CameraTools::CameraTools(Engine &engine) : m_engine(engine) {}

void CameraTools::render() {
  if (!ImGui::CollapsingHeader("Camera"))
    return;

  ImGui::Text("Is moving: %s", m_engine.getCamera().isMoving() ? "yes" : "no");
  auto pos = m_engine.getCamera().getAt();
  if (ngf::ImGui::InputFloat2("Position", &pos)) {
    m_engine.getCamera().at(pos);
  }
  auto optBounds = m_engine.getCamera().getBounds();
  ngf::irect bounds;
  if (optBounds.has_value()) {
    bounds = optBounds.value();
  }
  if (ngf::ImGui::InputInt4("Bounds", &bounds)) {
    m_engine.getCamera().setBounds(bounds);
  }
  ImGui::SameLine();
  if (ImGui::Button("Reset##Bounds")) {
    m_engine.getCamera().resetBounds();
  }
}
}