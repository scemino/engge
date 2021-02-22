#include "SoundTools.hpp"
#include <imgui.h>
#include <engge/Audio/SoundManager.hpp>
#include <engge/Engine/Engine.hpp>
#include <ngf/Graphics/ImGuiExtensions.h>

namespace ng {
namespace {
std::string getName(SoundId *soundId) {
  auto sd = soundId ? soundId->getSoundDefinition() : nullptr;
  if (!sd)
    return {};
  return sd->getPath();
}

std::string getLoopTimes(SoundId *soundId) {
  if (!soundId)
    return {};
  auto loopTimes = soundId->getSoundHandle()->get().getNumLoops();
  if (loopTimes == -1)
    return "Inf.";
  return std::to_string(loopTimes);
}

std::string getCategory(SoundId *soundId) {
  if (!soundId)
    return {};

  switch (soundId->getSoundCategory()) {
  case SoundCategory::Music:return "music";
  case SoundCategory::Sound:return "sound";
  case SoundCategory::Talk:return "talk";
  default: return "?";
  }
}

ngf::Color getCategoryColor(SoundId *soundId) {
  if (!soundId)
    return ngf::Colors::White;
  switch (soundId->getSoundCategory()) {
  case SoundCategory::Music:return ngf::Colors::Green;
  case SoundCategory::Sound:return ngf::Colors::Red;
  case SoundCategory::Talk:return ngf::Colors::Yellow;
  default: return ngf::Colors::White;
  }
}

std::string getStatus(SoundId *soundId) {
  if (!soundId)
    return "Stopped";
  switch (soundId->getSoundHandle().get()->get().getStatus()) {
  case ngf::AudioChannel::Status::Playing:return "Playing";
  case ngf::AudioChannel::Status::Stopped:return "Stopped";
  case ngf::AudioChannel::Status::Paused:return "Paused";
  default: return "?";
  }
}

}

SoundTools::SoundTools(Engine &engine) : m_engine(engine) {}

void SoundTools::render() {
  if (!soundsVisible)
    return;

  auto &sounds = m_engine.getSoundManager().getSounds();
  auto numSounds = 0;
  for (auto &sound : sounds) {
    if (sound)
      numSounds++;
  }

  ImGui::Begin("Sounds", &soundsVisible);
  ImGui::Text("# sounds: %d/%lu", numSounds, sounds.size());
  ImGui::Separator();

  if (ImGui::BeginTable("Sounds",
                        7,
                        ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable
                            | ImGuiTableFlags_RowBg)) {
    ImGui::TableSetupColumn("");
    ImGui::TableSetupColumn("Id");
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Loops");
    ImGui::TableSetupColumn("Category");
    ImGui::TableSetupColumn("Volume");
    ImGui::TableSetupColumn("Status");
    ImGui::TableHeadersRow();

    for (auto i = 0; i < static_cast<int>(sounds.size()); i++) {
      const auto &sound = sounds[i];
      const auto name = getName(sound.get());
      const auto loopTimes = getLoopTimes(sound.get());
      const auto volume = sound ? sound->getSoundHandle()->get().getVolume() : 0.f;
      const auto category = getCategory(sound.get());
      const auto catColor = getCategoryColor(sound.get());
      const auto status = getStatus(sound.get());

      ImGui::TableNextRow();
      if (name.empty()) {
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        ImGui::Text("%2d", i);
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        continue;
      }

      ImGui::TableNextColumn();
      if (ImGui::SmallButton("stop")) {
        sound->stop();
      }
      ImGui::TableNextColumn();
      ImGui::Text("%2d", i);
      ImGui::TableNextColumn();
      ImGui::Text("%-48s", name.c_str());

      ImGui::TableNextColumn();
      ImGui::Text("%s", loopTimes.c_str());
      ImGui::TableNextColumn();
      ImGui::TextColored(ngf::ImGui::ImVec4(catColor), " %7s", category.data());
      ImGui::TableNextColumn();
      ImGui::Text("%.1f", volume);
      ImGui::TableNextColumn();
      ImGui::Text("%s", status.c_str());
    }
    ImGui::EndTable();
  }
  ImGui::End();
}
}