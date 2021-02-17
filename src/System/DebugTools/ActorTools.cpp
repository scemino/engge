#include "ActorTools.hpp"
#include <ngf/Graphics/ImGuiExtensions.h>
#include <engge/Engine/Engine.hpp>
#include <engge/Engine/EngineSettings.hpp>
#include <engge/Room/Room.hpp>
#include <engge/Scripting/ScriptEngine.hpp>
#include "Util/Util.hpp"
#include "DebugControls.hpp"

namespace ng {
ActorTools::ActorTools(Engine &engine) : m_engine(engine) {}

void ActorTools::render() {
  if (!actorsVisible)
    return;

  ImGui::Begin("Actors", &actorsVisible);
  if (ImGui::SmallButton("Table...")) {
    m_showActorTable = true;
  }
  ImGui::Checkbox("General", &m_showProperties);
  ImGui::Checkbox("Inventory", &m_showInventory);
  ImGui::Checkbox("Costume", &m_showCostume);
  ImGui::Separator();

  // show actor list
  auto &actors = m_engine.getActors();
  for (auto &&actor : actors) {
    ImGui::PushID(actor.get());
    bool isSelected = actor.get() == m_selectedActor;
    auto visible = actor->isVisible();
    if (ImGui::Checkbox("", &visible)) {
      actor->setVisible(visible);
    }
    ImGui::SameLine();
    if (ImGui::Selectable(toUtf8(actor->getTranslatedName()).c_str(), isSelected)) {
      m_selectedActor = actor.get();
    }
    ImGui::PopID();
  }

  ImGui::End();

  if (m_selectedActor) {
    showActorTable(m_selectedActor);
    showProperties(m_selectedActor);
    showInventory(m_selectedActor);
    showCostume(m_selectedActor);
  }
}

void ActorTools::showInventory(Actor *actor) {
  if (!m_showInventory)
    return;

  ImGui::Begin("Inventory", &m_showInventory);
  for (const auto &obj : actor->getObjects()) {
    if (ImGui::TreeNode(&obj, "%s", obj->getKey().c_str())) {
      auto jiggle = obj->getJiggle();
      if (ImGui::Checkbox("Jiggle", &jiggle)) {
        obj->setJiggle(jiggle);
      }
      auto pop = obj->getPop();
      if (ImGui::DragInt("Pop", &pop, 1, 0, 10)) {
        obj->setPop(pop);
      }
      ImGui::TreePop();
    }
  }
  ImGui::End();
}

void ActorTools::showProperties(Actor *actor) {
  if (!m_showProperties)
    return;

  ImGui::Begin("Actor properties", &m_showProperties);

  auto isTouchable = actor->isTouchable();
  if (ImGui::Checkbox("Touchable", &isTouchable)) {
    actor->setTouchable(isTouchable);
  }
  auto isLit = actor->isLit();
  if (ImGui::Checkbox("Lit", &isLit)) {
    actor->setLit(isLit);
  }
  auto useWalkboxes = actor->useWalkboxes();
  if (ImGui::Checkbox("Use Walkboxes", &useWalkboxes)) {
    actor->useWalkboxes(useWalkboxes);
  }
  auto pRoom = actor->getRoom();
  auto flags = getFlags(*actor);
  ImGui::Text("Key: %s", actor->getKey().c_str());
  ImGui::Text("Flags: %s", flags.c_str());
  ImGui::Text("Icon: %s", actor->getIcon().c_str());
  ImGui::Text("Room: %s", pRoom ? pRoom->getName().c_str() : "(none)");
  ImGui::Text("Talking: %s", actor->isTalking() ? "yes" : "no");
  ImGui::Text("Walking: %s", actor->isWalking() ? "yes" : "no");
  ImGui::Text("Z-Order: %d", actor->getZOrder());
  if (pRoom) {
    auto scale = actor->getScale();
    ImGui::Text("Scale: %.3f", scale);
  }
  auto facing = facingToInt(actor->getCostume().getFacing());
  auto facings = "Front\0Back\0Left\0Right\0";
  if (ImGui::Combo("Facing", &facing, facings)) {
    actor->getCostume().setFacing(intToFacing(facing));
  }
  auto color = actor->getColor();
  if (ngf::ImGui::ColorEdit4("Color", &color)) {
    actor->setColor(color);
  }
  auto talkColor = actor->getTalkColor();
  if (ngf::ImGui::ColorEdit4("Talk color", &talkColor)) {
    actor->setTalkColor(talkColor);
  }
  auto talkOffset = actor->getTalkOffset();
  if (ImGui::DragInt2("Talk offset", &talkOffset.x)) {
    actor->setTalkOffset(talkOffset);
  }
  auto pos = actor->getPosition();
  if (ImGui::DragFloat2("Position", &pos.x)) {
    actor->setPosition(pos);
  }
  auto usePos = actor->getUsePosition().value_or(glm::vec2());
  if (ImGui::DragFloat2("Use Position", &usePos.x)) {
    actor->setUsePosition(usePos);
  }
  auto offset = actor->getOffset();
  if (ImGui::DragFloat2("Offset", &offset.x)) {
    actor->setOffset(offset);
  }
  auto renderOffset = actor->getRenderOffset();
  if (ImGui::DragInt2("Render Offset", &renderOffset.x)) {
    actor->setRenderOffset(renderOffset);
  }
  auto walkSpeed = actor->getWalkSpeed();
  if (ImGui::DragInt2("Walk speed", &walkSpeed.x)) {
    actor->setWalkSpeed(walkSpeed);
  }
  auto hotspotVisible = actor->isHotspotVisible();
  if (ImGui::Checkbox("Show hotspot", &hotspotVisible)) {
    actor->showHotspot(hotspotVisible);
  }
  auto hotspot = actor->getHotspot();
  if (ImGui::DragInt4("Hotspot", &hotspot.min.x)) {
    actor->setHotspot(hotspot);
  }
  auto useDirection = directionToInt(actor->getUseDirection().value_or(UseDirection::Front));
  auto directions = "Front\0Back\0Left\0Right\0";
  if (ImGui::Combo("Use direction", &useDirection, directions)) {
    actor->setUseDirection(intToDirection(useDirection));
  }
  auto fps = actor->getFps();
  if (ImGui::InputInt("FPS", &fps)) {
    actor->setFps(fps);
  }
  auto inventoryOffset = actor->getInventoryOffset();
  if (ImGui::InputInt("Inventory Offset", &inventoryOffset)) {
    actor->setInventoryOffset(inventoryOffset);
  }
  auto volume = actor->getVolume();
  if (ImGui::SliderFloat("Volume", &volume, 0.f, 1.0f)) {
    actor->setVolume(volume);
  }
  auto rotation = actor->getRotation();
  if (ImGui::DragFloat("Rotation", &rotation, 0.f, 360.0f)) {
    actor->setRotation(rotation);
  }
  auto scale = actor->getScale();
  if (ImGui::DragFloat("scale", &scale, 0.f, 100.f)) {
    actor->setScale(scale);
  }

  // head index
  ImGui::Separator();
  auto head = actor->getCostume().getHeadIndex();
  for (int i = 0; i < 6; ++i) {
    std::string s;
    s.append("Head #").append(std::to_string(i + 1));
    if (ImGui::RadioButton(s.c_str(), head == i)) {
      actor->getCostume().setHeadIndex(head);
    }
    ImGui::SameLine();
  }
  ImGui::NewLine();

  // animation names
  ImGui::Separator();
  std::string headAnim;
  std::string standAnim;
  std::string walkAnim;
  std::string reachAnim;
  actor->getCostume().getAnimationNames(headAnim, standAnim, walkAnim, reachAnim);
  ImGui::Text("Head: %s", headAnim.c_str());
  ImGui::Text("Stand: %s", standAnim.c_str());
  ImGui::Text("Walk: %s", walkAnim.c_str());
  ImGui::Text("Reach: %s", reachAnim.c_str());

  ImGui::End();
}

void ActorTools::showCostume(Actor *actor) {
  if (!m_showCostume)
    return;

  ImGui::Begin("General", &m_showCostume);

  auto state = actor->getCostume().getAnimControl().getState();
  auto loop = actor->getCostume().getAnimControl().getLoop();
  auto pAnim = actor->getCostume().getAnimControl().getAnimation();
  std::string stateText;
  switch (state) {
  case AnimState::Stopped:stateText = "Stopped";
    break;
  case AnimState::Play:stateText = "Play";
    break;
  case AnimState::Pause:stateText = "Pause";
    break;
  default:stateText = "?";
    break;
  }
  ImGui::Text("Anim: %s", pAnim ? pAnim->name.c_str() : "(none)");
  ImGui::Text("State: %s", stateText.c_str());
  ImGui::Text("Loop: %s", loop ? "yes" : "no");
  ImGui::Separator();

  m_filterCostume.Draw("Filter");
  if (ImGui::ListBoxHeader("Costume")) {
    auto actorKey = actor->getKey();
    std::vector<std::string> entries;
    for (const auto &pack : Locator<EngineSettings>::get()) {
      for (const auto &itEntry : *pack) {
        const auto &entry = itEntry.first;
        if (entry.length() < 15)
          continue;
        auto extension = entry.substr(entry.length() - 14, 14);
        CaseInsensitiveCompare cmp;
        if (!cmp(extension, "Animation.json"))
          continue;
        auto prefix = entry.substr(0, actorKey.length());
        if (!cmp(prefix, actorKey))
          continue;
        if (m_filterCostume.PassFilter(entry.c_str())) {
          if (ImGui::Selectable(entry.c_str(), actor->getCostume().getPath() == entry)) {
            actor->getCostume().loadCostume(entry);
          }
        }
      }
    }
    ImGui::ListBoxFooter();
  }
  ImGui::End();
}

std::string ActorTools::getFlags(Actor &actor) {
  auto flags = actor.getFlags();
  std::ostringstream os;
  if (flags & ObjectFlagConstants::GIVEABLE) {
    os << "GIVEABLE ";
  }
  if (flags & ObjectFlagConstants::TALKABLE) {
    os << "TALKABLE ";
  }
  if (flags & ObjectFlagConstants::FEMALE) {
    os << "FEMALE ";
  }
  if (flags & ObjectFlagConstants::MALE) {
    os << "MALE ";
  }
  if (flags & ObjectFlagConstants::MALE) {
    os << "PERSON ";
  }
  os << std::hex << flags;
  return os.str();
}

void ActorTools::showActorTable(Actor *actor) {
  if (!m_showActorTable)
    return;

  ImGui::Begin("Actor table", &m_showActorTable);
  if (actor) {
    auto table = actor->getTable();
    DebugControls::createTree(actor->getName().c_str(), table);
  }
  ImGui::End();
}

int ActorTools::facingToInt(Facing facing) {
  switch (facing) {
  case Facing::FACE_FRONT:return 0;
  case Facing::FACE_BACK:return 1;
  case Facing::FACE_LEFT:return 2;
  case Facing::FACE_RIGHT:return 3;
  }
  return 0;
}

Facing ActorTools::intToFacing(int facing) {
  switch (facing) {
  case 0:return Facing::FACE_FRONT;
  case 1:return Facing::FACE_BACK;
  case 2:return Facing::FACE_LEFT;
  case 3:return Facing::FACE_RIGHT;
  }
  return Facing::FACE_FRONT;
}

int ActorTools::directionToInt(UseDirection dir) {
  switch (dir) {
  case UseDirection::Front:return 0;
  case UseDirection::Back:return 1;
  case UseDirection::Left:return 2;
  case UseDirection::Right:return 3;
  }
  return 0;
}

UseDirection ActorTools::intToDirection(int dir) {
  switch (dir) {
  case 0:return UseDirection::Front;
  case 1:return UseDirection::Back;
  case 2:return UseDirection::Left;
  case 3:return UseDirection::Right;
  }
  return UseDirection::Front;
}
}