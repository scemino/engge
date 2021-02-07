#include "SoundTools.hpp"
#include <imgui.h>
#include <engge/Audio/SoundManager.hpp>
#include <engge/Engine/Engine.hpp>
#include <ngf/Graphics/ImGuiExtensions.h>

namespace ng {
SoundTools::SoundTools(Engine &engine) : m_engine(engine) {}

void SoundTools::render() {
  if (!ImGui::CollapsingHeader("Sounds"))
    return;

  ImGui::Indent();
  auto &sounds = m_engine.getSoundManager().getSounds();
  for (auto i = 0; i < static_cast<int>(sounds.size()); i++) {
    ImGui::PushID(i);
    const auto &sound = sounds[i];
    const auto *sd = sound ? sound->getSoundDefinition() : nullptr;
    const auto *name = !sd ? "<free>" : sd->getPath().data();
    auto loopTimes = sound ? sound->getLoopTimes() : 0;
    auto volume = sound ? sound->getVolume() : 0.f;
    std::string category;
    auto catColor = ngf::Colors::White;
    if (sound) {
      switch (sound->getSoundCategory()) {
      case SoundCategory::Music:catColor = ngf::Colors::Green;
        category = "[music]";
        break;
      case SoundCategory::Sound:catColor = ngf::Colors::Red;
        category = "[sound]";
        break;
      case SoundCategory::Talk:catColor = ngf::Colors::Yellow;
        category = "[talk]";
        break;
      }
    }
    if (ImGui::SmallButton("stop")) {
      sound->stop();
    }
    ImGui::SameLine();
    ImGui::Text("%2d: %-48s x%2d", i, name, loopTimes);
    ImGui::SameLine();
    ImGui::TextColored(ngf::ImGui::ImVec4(catColor), " %7s", category.data());
    ImGui::SameLine();
    ImGui::Text(" %.1f", volume);
    ImGui::PopID();
  }
  ImGui::Unindent();
}
}