#pragma once
#include <ngf/Graphics/ImGuiExtensions.h>
#include <ngf/Math/PathFinding/Walkbox.h>
#include <imgui.h>

namespace ng {
class _RoomTools {
public:
  bool _showRoomTable{false};

  explicit _RoomTools(Engine &engine) : _engine(engine) {}

  void render() {
    if (!ImGui::CollapsingHeader("Room"))
      return;

    auto &rooms = _engine.getRooms();
    _roomInfos.clear();
    int i = 0;
    int currentRoom = 0;
    for (auto &&room : rooms) {
      if (room.get() == _engine.getRoom()) {
        currentRoom = i;
      }
      _roomInfos.push_back(room->getName());
      i++;
    }

    if (ImGui::Combo("Room",
                     &currentRoom,
                     _DebugControls::stringGetter,
                     static_cast<void *>(&_roomInfos),
                     rooms.size())) {
      _engine.setRoom(rooms[currentRoom].get());
    }

    ImGui::SameLine();
    if (ImGui::SmallButton("Table...")) {
      _showRoomTable = true;
    }

    auto &room = rooms[currentRoom];

    auto options = _engine.getWalkboxesFlags();
    auto showWalkboxes = (options & 4) != 0;
    if (ImGui::Checkbox("Walkboxes", &showWalkboxes)) {
      _engine.setWalkboxesFlags(showWalkboxes ? (4 | options) : (options & ~4));
    }
    auto showMergedWalkboxes = (options & 1) != 0;
    if (ImGui::Checkbox("Merged Walkboxes", &showMergedWalkboxes)) {
      _engine.setWalkboxesFlags(showMergedWalkboxes ? (1 | options) : (options & ~1));
    }
    auto showGraph = (options & 2) != 0;
    if (ImGui::Checkbox("Graph", &showGraph)) {
      _engine.setWalkboxesFlags(showGraph ? (2 | options) : (options & ~2));
    }
    updateWalkboxInfos(room.get());
    ImGui::Combo("##walkboxes", &_selectedWalkbox, _DebugControls::stringGetter, static_cast<void *>(&_walkboxInfos),
                 _walkboxInfos.size());
    auto rotation = room->getRotation();
    if (ImGui::SliderFloat("rotation", &rotation, -180.f, 180.f, "%.0f deg")) {
      room->setRotation(rotation);
    }
    auto overlay = room->getOverlayColor();
    if (ngf::ImGui::ColorEdit4("Overlay", &overlay)) {
      room->setOverlayColor(overlay);
    }
    auto ambient = room->getAmbientLight();
    if (ngf::ImGui::ColorEdit4("ambient", &ambient)) {
      room->setAmbientLight(ambient);
    }
    for (i = 0; i < static_cast<int>(room->getLights().size()); ++i) {
      if (i >= room->getNumberLights())
        break;

      std::ostringstream ss;
      ss << "Light " << (i + 1);

      if (ImGui::TreeNode(ss.str().c_str())) {
        auto &light = room->getLights()[i];
        ImGui::DragInt2("Position", &light.pos.x);
        ngf::ImGui::ColorEdit4("Color", &light.color);
        ImGui::DragFloat("Direction angle", &light.coneDirection,
                         1.0f, 0.0f, 360.f);
        ImGui::DragFloat("Angle", &light.coneAngle, 1.0f, 0.0f, 360.f);
        ImGui::DragFloat("Cutoff", &light.cutOffRadius, 1.0f);
        ImGui::DragFloat("Falloff", &light.coneFalloff, 0.1f, 0.f, 1.0f);
        ImGui::DragFloat(
            "Brightness", &light.brightness, 1.0f, 1.0f, 100.f);
        ImGui::DragFloat(
            "Half Radius", &light.halfRadius, 1.0f, 0.01f, 0.99f);
        ImGui::TreePop();
      }
    }
    auto effect = room->getEffect();
    auto effects = "None\0Sepia\0EGA\0VHS\0Ghost\0Black & White\0";
    if (ImGui::Combo("Shader", &effect, effects)) {
      room->setEffect(effect);
    }
  }

  void updateWalkboxInfos(Room *pRoom) {
    _walkboxInfos.clear();
    if (!pRoom)
      return;
    auto &walkboxes = pRoom->getWalkboxes();
    for (size_t i = 0; i < walkboxes.size(); ++i) {
      auto walkbox = walkboxes.at(i);
      auto name = walkbox.getName();
      std::ostringstream s;
      if (!name.empty()) {
        s << name;
      } else {
        s << "Walkbox #" << i;
      }
      s << " " << (walkbox.isEnabled() ? "[enabled]" : "[disabled]");
      _walkboxInfos.push_back(s.str());
    }
  }

private:
  Engine &_engine;
  int _selectedWalkbox{0};
  std::vector<std::string> _walkboxInfos;
  std::vector<std::string> _roomInfos;
};
}