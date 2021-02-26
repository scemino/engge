#include "ObjectTools.hpp"
#include <engge/Engine/Engine.hpp>
#include <engge/Room/Room.hpp>
#include <engge/Entities/Object.hpp>
#include <engge/Entities/TextObject.hpp>
#include <engge/Engine/Trigger.hpp>
#include <ngf/Graphics/ImGuiExtensions.h>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <engge/Engine/EntityManager.hpp>
#include "Util/Util.hpp"

namespace ng {
namespace {
std::string getName(Object *object) {
  auto name = toUtf8(ng::Engine::getText(object->getKey()));
  if (!name.empty())
    return name;

  auto textObj = dynamic_cast<TextObject *>(object);
  if (textObj) {
    name = textObj->getText();
    if (name.size() > 10) {
      name.erase(name.begin() + 10, name.end());
      return name + "...";
    }
  }
  name = "#" + std::to_string(object->getId());
  return name;
}

std::string getType(Object *object) {
  switch (object->getType()) {
  case ObjectType::Object:return "object";
  case ObjectType::Spot:return "spot";
  case ObjectType::Trigger:return "trigger";
  case ObjectType::Prop:return "prop";
  }
  return "?";
}

bool showHorizontalTextAlignment(TextAlignment *alignment) {
  auto hAlign = *alignment & TextAlignment::Horizontal;
  if (ImGui::RadioButton("Left", hAlign == TextAlignment::Left)) {
    *alignment &= ~TextAlignment::Horizontal;
    *alignment |= TextAlignment::Left;
    return true;
  }
  ImGui::SameLine();
  if (ImGui::RadioButton("Center", hAlign == TextAlignment::Center)) {
    *alignment &= ~TextAlignment::Horizontal;
    *alignment |= TextAlignment::Center;
    return true;
  }
  ImGui::SameLine();
  if (ImGui::RadioButton("Right", hAlign == TextAlignment::Right)) {
    *alignment &= ~TextAlignment::Horizontal;
    *alignment |= TextAlignment::Right;
    return true;
  }
  return false;
}

bool showVerticalTextAlignment(TextAlignment *alignment) {
  auto vTop = *alignment & TextAlignment::Top;
  auto vBottom = *alignment & TextAlignment::Bottom;
  if (ImGui::RadioButton("Top", vTop == ng::TextAlignment::Top)) {
    *alignment &= ~TextAlignment::Vertical;
    *alignment |= TextAlignment::Top;
    return true;
  }
  ImGui::SameLine();
  if (ImGui::RadioButton("Center##Vertical", vTop == ng::TextAlignment::None && vBottom == ng::TextAlignment::None)) {
    *alignment &= ~TextAlignment::Vertical;
    return true;
  }
  ImGui::SameLine();
  if (ImGui::RadioButton("Bottom", vBottom == ng::TextAlignment::Bottom)) {
    *alignment &= ~TextAlignment::Vertical;
    *alignment |= TextAlignment::Bottom;
    return true;
  }
  return false;
}
}

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
  m_textFilter.Draw();

  for (auto &&object : objects) {
    auto name = getName(object.get());
    if (!m_textFilter.PassFilter(name.c_str()))
      continue;
    ImGui::PushID(object.get());
    bool isSelected = object->getId() == m_objectId;
    auto visible = object->isVisible();
    if (ImGui::Checkbox("", &visible)) {
      object->setVisible(visible);
    }
    ImGui::SameLine();
    if (ImGui::Selectable(name.c_str(), isSelected)) {
      m_objectId = object->getId();
    }
    ImGui::PopID();
  }

  ImGui::Separator();

  auto pObj = EntityManager::getObjectFromId(m_objectId);
  if (pObj) {
    showProperties(pObj);
    showAnimations(pObj);
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

  auto textObject = dynamic_cast<TextObject *>(object);

  ImGui::Begin("Object Properties", &m_showProperties);
  auto name = object->getName();
  ImGui::LabelText("Name", "%s", name.c_str());
  ImGui::Separator();

  if (textObject) {
    auto text = textObject->getText();
    if (ImGui::InputText("Text", &text)) {
      textObject->setText(text);
    }
    auto alignment = textObject->getAlignment();
    if (showHorizontalTextAlignment(&alignment)) {
      textObject->setAlignment(alignment);
    }
    if (showVerticalTextAlignment(&alignment)) {
      textObject->setAlignment(alignment);
    }
    auto maxWidth = textObject->getMaxWidth();
    if (ImGui::InputInt("Max width", &maxWidth)) {
      textObject->setMaxWidth(maxWidth);
    }
    ImGui::Separator();
  }

  auto type = getType(object);
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