#pragma once
#include "imgui-SFML.h"
#include "imgui.h"

namespace ng {
class _CameraTools {
public:
  explicit _CameraTools(Engine &engine) : _engine(engine) {}

  void render() {
    if (!ImGui::CollapsingHeader("Camera"))
      return;

    ImGui::Text("Is moving: %s", _engine.getCamera().isMoving() ? "yes" : "no");
    sf::Vector2f pos = _engine.getCamera().getAt();
    if (_DebugControls::InputFloat2("Position", pos)) {
      _engine.getCamera().at(pos);
    }
    auto optBounds = _engine.getCamera().getBounds();
    sf::IntRect bounds;
    if (optBounds.has_value()) {
      bounds = optBounds.value();
    }
    if (_DebugControls::InputInt4("Bounds", bounds)) {
      _engine.getCamera().setBounds(bounds);
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset##Bounds")) {
      _engine.getCamera().resetBounds();
    }
  }

private:
  Engine &_engine;
};
}