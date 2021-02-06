#pragma once
#include <ngf/Graphics/ImGuiExtensions.h>
#include <imgui.h>

namespace ng {
class ObjectTools final {
public:
  explicit ObjectTools(Engine &engine) : _engine(engine) {}

  void render() {
    if (!ImGui::CollapsingHeader("Objects"))
      return;

    auto &objects = _engine.getRoom()->getObjects();

    static ImGuiTextFilter filter;
    filter.Draw("Filter");
    std::ostringstream s;
    s << objects.size() << " Objects";
    if (ImGui::ListBoxHeader(s.str().c_str())) {
      for (const auto &object : objects) {
        auto name = toUtf8(_engine.getText(object->getKey()));
        if (filter.PassFilter(name.c_str())) {
          if (ImGui::Selectable(name.c_str(), _pSelectedObject == object.get())) {
            _pSelectedObject = object.get();
          }
        }
      }
      ImGui::ListBoxFooter();
    }

    ImGui::Separator();

    if (!objects.empty() && _pSelectedObject) {
      auto &object = _pSelectedObject;
      auto name = object->getName();
      ImGui::LabelText("Name", "%s", name.c_str());
      std::string type;
      switch (object->getType()) {
      case ObjectType::Object:type = "object";
        break;
      case ObjectType::Spot:type = "spot";
        break;
      case ObjectType::Trigger:type = "trigger";
        break;
      case ObjectType::Prop:type = "prop";
        break;
      }
      ImGui::LabelText("Type", "%s", type.c_str());
      auto pOwner = object->getOwner();
      ImGui::LabelText("Owner", "%s", pOwner ? pOwner->getName().c_str() : "(none)");
      auto isVisible = object->isVisible();
      if (ImGui::Checkbox("Visible", &isVisible)) {
        object->setVisible(isVisible);
      }
      auto isLit = object->isLit();
      if (ImGui::Checkbox("Is lit", &isLit)) {
        object->setLit(isLit);
      }
      auto state = object->getState();
      if (ImGui::DragInt("State", &state)) {
        object->playAnim(state, false);
      }
      auto isTouchable = object->isTouchable();
      if (ImGui::Checkbox("Touchable", &isTouchable)) {
        object->setTouchable(isTouchable);
      }
      auto zorder = object->getZOrder();
      if (ImGui::DragInt("Z-Order", &zorder)) {
        object->setZOrder(zorder);
      }
      auto pos = object->getPosition();
      if (ImGui::DragFloat2("Position", &pos.x)) {
        object->setPosition(pos);
      }
      auto usePos = object->getUsePosition().value_or(glm::vec2());
      if (ImGui::DragFloat2("Use Position", &usePos.x)) {
        object->setUsePosition(usePos);
      }
      auto offset = object->getOffset();
      if (ImGui::DragFloat2("Offset", &offset.x)) {
        object->setOffset(offset);
      }
      auto renderOffset = object->getRenderOffset();
      if (ImGui::DragInt2("Render Offset", &renderOffset.x)) {
        object->setRenderOffset(renderOffset);
      }
      auto hotspotVisible = object->isHotspotVisible();
      if (ImGui::Checkbox("Show hotspot", &hotspotVisible)) {
        object->showDebugHotspot(hotspotVisible);
      }
      auto hotspot = object->getHotspot();
      if (ImGui::DragInt4("Hotspot", &hotspot.min.x)) {
        object->setHotspot(hotspot);
      }
      auto color = object->getColor();
      if (ngf::ImGui::ColorEdit4("Color", &color)) {
        object->setColor(color);
      }
      auto trigger = object->getTrigger();
      if (trigger) {
        ImGui::LabelText("Trigger", "%s", trigger->getName().c_str());
      }
    }
  }

private:
  Engine &_engine;
  Object *_pSelectedObject{nullptr};
};
}