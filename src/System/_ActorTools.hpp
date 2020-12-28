#pragma once
#include <imgui.h>
#include <ngf/Graphics/ImGuiExtensions.h>

namespace ng {
class _ActorTools {
public:
  bool _showActorTable{false};

  explicit _ActorTools(Engine &engine) : _engine(engine) {}

  void render() {
    if (!ImGui::CollapsingHeader("Actors"))
      return;

    ImGui::Indent();
    auto &actors = _engine.getActors();
    _actorInfos.clear();
    for (auto &&actor : actors) {
      _actorInfos.push_back(toUtf8(actor->getTranslatedName()));
    }
    ImGui::Combo("##Actor",
                 &_selectedActor,
                 _DebugControls::stringGetter,
                 static_cast<void *>(&_actorInfos),
                 _actorInfos.size());
    ImGui::SameLine();
    if (ImGui::SmallButton("Table...")) {
      _showActorTable = true;
    }

    auto &actor = actors[_selectedActor];

    auto head = actor->getCostume().getHeadIndex();
    if (ImGui::SliderInt("Head index", &head, 0, 5)) {
      actor->getCostume().setHeadIndex(head);
    }

    showGeneral(actor.get());
    showInventory(actor.get());
    showCostume(actor.get());
    showAnimations(actor.get());

    ImGui::Unindent();
  }

  void showInventory(Actor *actor) {
    if (ImGui::TreeNode("Inventory")) {
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
      ImGui::TreePop();
    }
  }

  void showGeneral(Actor *actor) {
    if (ImGui::TreeNode("General")) {
      auto isVisible = actor->isVisible();
      if (ImGui::Checkbox("Visible", &isVisible)) {
        actor->setVisible(isVisible);
      }
      auto isTouchable = actor->isTouchable();
      if (ImGui::Checkbox("Touchable", &isTouchable)) {
        actor->setTouchable(isTouchable);
      }
      auto isLit = actor->isLit();
      if (ImGui::Checkbox("Lit", &isLit)) {
        actor->setLit(isLit);
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
      auto facing = facingToInt(actor->getCostume().getFacing());
      auto facings = "Front\0Back\0Left\0Right\0";
      if (ImGui::Combo("Facing", &facing, facings)) {
        actor->getCostume().setFacing(intToFacing(facing));
      }
      if (pRoom) {
        auto scale = actor->getScale();
        ImGui::Text("Scale: %.3f", scale);
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
      if (ngf::ImGui::InputInt2("Talk offset", &talkOffset)) {
        actor->setTalkOffset(talkOffset);
      }
      auto pos = actor->getPosition();
      if (ngf::ImGui::InputFloat2("Position", &pos)) {
        actor->setPosition(pos);
      }
      auto usePos = actor->getUsePosition().value_or(glm::vec2());
      if (ngf::ImGui::InputFloat2("Use Position", &usePos)) {
        actor->setUsePosition(usePos);
      }
      auto offset = actor->getOffset();
      if (ngf::ImGui::InputFloat2("Offset", &offset)) {
        actor->setOffset(offset);
      }
      auto renderOffset = actor->getRenderOffset();
      if (ngf::ImGui::InputInt2("Render Offset", &renderOffset)) {
        actor->setRenderOffset(renderOffset);
      }
      auto walkSpeed = actor->getWalkSpeed();
      if (ngf::ImGui::InputInt2("Walk speed", &walkSpeed)) {
        actor->setWalkSpeed(walkSpeed);
      }
      auto hotspotVisible = actor->isHotspotVisible();
      if (ImGui::Checkbox("Show hotspot", &hotspotVisible)) {
        actor->showHotspot(hotspotVisible);
      }
      auto hotspot = actor->getHotspot();
      if (ngf::ImGui::InputInt4("Hotspot", &hotspot)) {
        actor->setHotspot(hotspot);
      }
      auto useWalkboxes = actor->useWalkboxes();
      if (ImGui::Checkbox("Use Walkboxes", &useWalkboxes)) {
        actor->useWalkboxes(useWalkboxes);
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
      auto volume = actor->getVolume().value_or(1.0f);
      if (ImGui::SliderFloat("Volume", &volume, 0.f, 1.0f)) {
        actor->setVolume(volume);
      }
      auto rotation = actor->getRotation();
      if (ImGui::SliderFloat("Rotation", &rotation, 0.f, 360.0f)) {
        actor->setRotation(rotation);
      }
      auto scale = actor->getScale();
      if (ImGui::SliderFloat("scale", &scale, 0.f, 100.f)) {
        actor->setScale(scale);
      }
      ImGui::TreePop();
    }
  }

  void showCostume(Actor *actor) {
    if (ImGui::TreeNode("Costume")) {
      ImGui::PushID("costume");
      _filterCostume.Draw("Filter");
      if (ImGui::ListBoxHeader("Costume")) {
        auto actorKey = actor->getKey();
        std::vector<std::string> entries;
        Locator<EngineSettings>::get().getEntries(entries);
        for (const auto &entry : entries) {
          if (entry.length() < 15)
            continue;
          auto extension = entry.substr(entry.length() - 14, 14);
          CaseInsensitiveCompare cmp;
          if (!cmp(extension, "Animation.json"))
            continue;
          auto prefix = entry.substr(0, actorKey.length());
          if (!cmp(prefix, actorKey))
            continue;
          if (_filterCostume.PassFilter(entry.c_str())) {
            if (ImGui::Selectable(entry.c_str(), actor->getCostume().getPath() == entry)) {
              actor->getCostume().loadCostume(entry);
            }
          }
        }
        ImGui::ListBoxFooter();
      }
      ImGui::PopID();
      ImGui::TreePop();
    }
  }
  void showAnimations(Actor *actor) {
    if (ImGui::TreeNode("Animations")) {
      // actor animations
      auto &anims = actor->getCostume().getAnimations();
      static ImGuiTextFilter filter;
      filter.Draw("Filter");
      if (ImGui::ListBoxHeader("Animations")) {
        for (auto &anim : anims) {
          auto name = anim.getName();
          if (filter.PassFilter(name.c_str())) {
            if (ImGui::Selectable(name.c_str(), _pSelectedAnim == &anim)) {
              _pSelectedAnim = &anim;
            }
          }
        }
        ImGui::ListBoxFooter();
      }
      if (_pSelectedAnim && ImGui::Button("Set")) {
        actor->getCostume().setAnimation(_pSelectedAnim->getName());
      }
      if (_pSelectedAnim) {
        showLayers();
      }
      ImGui::TreePop();
    }
  }

private:
  static std::string getFlags(Actor &actor) {
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

  static int facingToInt(Facing facing) {
    switch (facing) {
    case Facing::FACE_FRONT:return 0;
    case Facing::FACE_BACK:return 1;
    case Facing::FACE_LEFT:return 2;
    case Facing::FACE_RIGHT:return 3;
    }
    return 0;
  }

  static Facing intToFacing(int facing) {
    switch (facing) {
    case 0:return Facing::FACE_FRONT;
    case 1:return Facing::FACE_BACK;
    case 2:return Facing::FACE_LEFT;
    case 3:return Facing::FACE_RIGHT;
    }
    return Facing::FACE_FRONT;
  }

  static int directionToInt(UseDirection dir) {
    switch (dir) {
    case UseDirection::Front:return 0;
    case UseDirection::Back:return 1;
    case UseDirection::Left:return 2;
    case UseDirection::Right:return 3;
    }
    return 0;
  }

  static UseDirection intToDirection(int dir) {
    switch (dir) {
    case 0:return UseDirection::Front;
    case 1:return UseDirection::Back;
    case 2:return UseDirection::Left;
    case 3:return UseDirection::Right;
    }
    return UseDirection::Front;
  }

  void showLayers() {
    ImGui::Columns(4, "LayersColumns", false);
    ImGui::Separator();
    for (auto &layer : _pSelectedAnim->getLayers()) {
      ImGui::Text("Layer %s", layer.getName().c_str());
      ImGui::NextColumn();
      auto &anim = layer.getAnimation();
      auto animIndex = static_cast<int>(anim.getIndex());
      auto animSize = static_cast<int>(anim.size() - 1);
      if (ImGui::SliderInt("Index", &animIndex, 0, animSize)) {
        anim.setIndex(animIndex);
      }
      ImGui::NextColumn();
      auto layerVisible = layer.getVisible();
      if (ImGui::Checkbox("Visible", &layerVisible)) {
        layer.setVisible(layerVisible);
      }
      ImGui::NextColumn();
      auto layerLoop = layer.getLoop();
      if (ImGui::Checkbox("Loop", &layerLoop)) {
        layer.setLoop(layerLoop);
      }
      ImGui::NextColumn();
    }
    ImGui::Columns(1);
    ImGui::Separator();
  }

private:
  Engine &_engine;
  int _selectedActor{0};
  CostumeAnimation *_pSelectedAnim{nullptr};
  ImGuiTextFilter _filterCostume;
  std::vector<std::string> _actorInfos;
};
}