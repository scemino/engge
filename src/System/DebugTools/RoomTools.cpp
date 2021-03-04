#include "RoomTools.hpp"
#include <imgui.h>
#include <sstream>
#include <engge/Room/Room.hpp>
#include <engge/Engine/Engine.hpp>
#include <ngf/Math/PathFinding/Walkbox.h>
#include <ngf/Graphics/ImGuiExtensions.h>
#include "DebugControls.hpp"

namespace ng {
namespace {
const char *RoomEffects = "None\0Sepia\0EGA\0VHS\0Ghost\0Black & White\0";
const char *FadeEffects = "None\0In\0Out\0Wobble\0";
}

RoomTools::RoomTools(Engine &engine) : m_engine(engine) {}

void RoomTools::render() {
  if (!ImGui::CollapsingHeader("Room"))
    return;

  auto &rooms = m_engine.getRooms();
  m_roomInfos.clear();
  int i = 0;
  int currentRoom = 0;
  for (auto &&room : rooms) {
    if (room.get() == m_engine.getRoom()) {
      currentRoom = i;
    }
    m_roomInfos.push_back(room->getName());
    i++;
  }

  if (ImGui::Combo("Room",
                   &currentRoom,
                   DebugControls::stringGetter,
                   static_cast<void *>(&m_roomInfos),
                   rooms.size())) {
    m_engine.setRoom(rooms[currentRoom].get());
  }

  ImGui::SameLine();
  if (ImGui::SmallButton("Table...")) {
    m_showRoomTable = true;
  }

  auto &room = rooms[currentRoom];

  auto options = m_engine.getWalkboxesFlags();
  auto showWalkboxes = (options & WalkboxesFlags::Walkboxes) == WalkboxesFlags::Walkboxes;
  if (ImGui::Checkbox("Walkboxes", &showWalkboxes)) {
    m_engine.setWalkboxesFlags(showWalkboxes ? (WalkboxesFlags::Walkboxes | options) : (options
        & ~WalkboxesFlags::Walkboxes));
  }
  auto showMergedWalkboxes = (options & WalkboxesFlags::Merged) == WalkboxesFlags::Merged;
  if (ImGui::Checkbox("Merged Walkboxes", &showMergedWalkboxes)) {
    m_engine.setWalkboxesFlags(showMergedWalkboxes ? (WalkboxesFlags::Merged | options) : (options
        & ~WalkboxesFlags::Merged));
  }
  auto showGraph = (options & WalkboxesFlags::Graph) == WalkboxesFlags::Graph;
  if (ImGui::Checkbox("Graph", &showGraph)) {
    m_engine.setWalkboxesFlags(showGraph ? (WalkboxesFlags::Graph | options) : (options & ~WalkboxesFlags::Graph));
  }
  updateWalkboxInfos(room.get());
  ImGui::Combo("##walkboxes", &m_selectedWalkbox, DebugControls::stringGetter, static_cast<void *>(&m_walkboxInfos),
               m_walkboxInfos.size());
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
  if (ImGui::Combo("Shader", &effect, RoomEffects)) {
    room->setEffect(effect);
  }
  ImGui::DragFloat("iGlobalTime", &m_engine.roomEffect.iGlobalTime);
  if (effect == 1) {
    ImGui::DragFloat("sepiaFlicker", &m_engine.roomEffect.sepiaFlicker, 0.01f, 0.f, 1.f);
    ImGui::DragFloat("RandomValue", &m_engine.roomEffect.RandomValue[0], 0.01f, 0.f, 1.f);
    ImGui::DragFloat("TimeLapse", &m_engine.roomEffect.TimeLapse);
  } else if (effect == 3) {
    ImGui::DragFloat("iNoiseThreshold", &m_engine.roomEffect.iNoiseThreshold, 0.01f, 0.f, 1.f);
  } else if (effect == 4) {
    ImGui::DragFloat("iFade", &m_engine.roomEffect.iFade, 0.01f, 0.f, 1.f);
    ImGui::DragFloat("wobbleIntensity", &m_engine.roomEffect.wobbleIntensity, 0.01f, 0.f, 1.f);
    ImGui::DragFloat3("shadows", glm::value_ptr(m_engine.roomEffect.shadows), 0.1f, -1.f, 1.f);
    ImGui::DragFloat3("midtones", glm::value_ptr(m_engine.roomEffect.midtones), 0.1f, -1.f, 1.f);
    ImGui::DragFloat3("highlights", glm::value_ptr(m_engine.roomEffect.highlights), 0.1f, -1.f, 1.f);
  }
  ImGui::Separator();

  ImGui::Combo("Effect", (int *) &m_fadeEffect, FadeEffects);
  ImGui::DragFloat("Duration", &m_fadeDuration, 0.1f, 0.f, 10.f);
  if (ImGui::Button("Fade")) {
    m_engine.fadeTo((FadeEffect) m_fadeEffect, ngf::TimeSpan::seconds(m_fadeDuration));
  }
}

void RoomTools::updateWalkboxInfos(Room *pRoom) {
  m_walkboxInfos.clear();
  if (!pRoom)
    return;
  auto &walkboxes = pRoom->getWalkboxes();
  for (size_t i = 0; i < walkboxes.size(); ++i) {
    auto walkbox = walkboxes.at(i);
    const auto &name = walkbox.getName();
    std::ostringstream s;
    if (!name.empty()) {
      s << name;
    } else {
      s << "Walkbox #" << i;
    }
    s << " " << (walkbox.isEnabled() ? "[enabled]" : "[disabled]");
    m_walkboxInfos.push_back(s.str());
  }
}
}