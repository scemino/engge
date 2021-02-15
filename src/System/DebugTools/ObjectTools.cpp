#include "ObjectTools.hpp"
#include <engge/Engine/Engine.hpp>
#include <engge/Room/Room.hpp>
#include <engge/Entities/Object.hpp>
#include <engge/Engine/Trigger.hpp>
#include <ngf/Graphics/ImGuiExtensions.h>
#include <imgui.h>
#include "Util/Util.hpp"

namespace ng {
ObjectTools::ObjectTools(Engine &engine) : m_engine(engine) {}

void ObjectTools::render() {
  if (!objectsVisible)
    return;

  ImGui::Begin("Objects", &objectsVisible);
  ImGui::Checkbox("Properties", &m_showProperties);
  ImGui::Checkbox("Animations", &m_showAnimations);
  ImGui::Separator();

  // show object list
  auto &objects = m_engine.getRoom()->getObjects();
  ImGui::Text("Count: %ld", objects.size());
  for (auto &&object : objects) {
    ImGui::PushID(object.get());
    bool isSelected = object.get() == m_pSelectedObject;
    auto visible = object->isVisible();
    if (ImGui::Checkbox("", &visible)) {
      object->setVisible(visible);
    }
    ImGui::SameLine();
    auto name = toUtf8(ng::Engine::getText(object->getKey()));
    if (name.empty()) {
      name = "#" + std::to_string(object->getId());
    }
    if (ImGui::Selectable(name.c_str(), isSelected)) {
      m_pSelectedObject = object.get();
    }
    ImGui::PopID();
  }

  ImGui::Separator();

  if (m_pSelectedObject) {
    showProperties(m_pSelectedObject);
    showAnimations(m_pSelectedObject);
  }

  ImGui::End();
}

void ObjectTools::showAnimations(Object *object) {
  if (!m_showAnimations)
    return;

  ImGui::Begin("Object Animations", &m_showAnimations);
  for (auto &anim : object->getAnims()) {
    showAnimationNode(&anim);
  }
  ImGui::End();
}

void ObjectTools::showAnimationNode(Animation *anim) {
  if (ImGui::TreeNode(anim, "%s", anim->name.c_str())) {
    for (auto &frame : anim->frames) {
      ImGui::Text("%s", frame.name.c_str());
    }
    for (auto &layer : anim->layers) {
      showAnimationNode(&layer);
    }
    ImGui::TreePop();
  }
}

void ObjectTools::showProperties(Object *object) {
  if (!m_showProperties)
    return;

  ImGui::Begin("Object Properties", &m_showProperties);
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
  auto pParent = object->getParent();
  ImGui::LabelText("Parent", "%s", pParent ? pParent->getName().c_str() : "(none)");
  auto isVisible = object->isVisible();
  if (ImGui::Checkbox("Visible", &isVisible)) {
    object->setVisible(isVisible);
  }
  auto isLit = object->isLit();
  if (ImGui::Checkbox("Is lit", &isLit)) {
    object->setLit(isLit);
  }
  auto isTouchable = object->isTouchable();
  if (ImGui::Checkbox("Touchable", &isTouchable)) {
    object->setTouchable(isTouchable);
  }
  auto state = object->getState();
  if (ImGui::DragInt("State", &state, 1.0f, 0)) {
    object->playAnim(state, false);
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
  ImGui::End();
}
}