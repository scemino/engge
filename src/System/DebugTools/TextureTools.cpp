#include "TextureTools.hpp"
#include <imgui.h>
#include <engge/Graphics/ResourceManager.hpp>
#include <engge/System/Locator.hpp>

namespace ng {
void TextureTools::render() {
  if (!texturesVisible)
    return;

  ImGui::Begin("Textures", &texturesVisible);
  const auto &map = Locator<ResourceManager>::get().getTextureMap();
  size_t totalSize = 0;
  for (const auto&[key, value] :map) {
    totalSize += value.size;
  }
  auto totalSizeText = convertSize(totalSize);
  ImGui::Text("Total memory: %s", totalSizeText.data());
  ImGui::Separator();

  if (ImGui::BeginTable("Textures",
                        3,
                        ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable
                            | ImGuiTableFlags_RowBg)) {
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Size");
    ImGui::TableSetupColumn("Refs");
    ImGui::TableHeadersRow();

    for (const auto&[key, value] :map) {
      auto fileSize = convertSize(value.size);
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("%s", key.data());
      ImGui::TableNextColumn();
      ImGui::Text("%s", fileSize.data());
      ImGui::TableNextColumn();
      ImGui::Text("%ld", value.texture.use_count());
    }
    ImGui::EndTable();
  }
  ImGui::End();
}

std::string TextureTools::convertSize(size_t size) {
  const char *suffix[] = {"B", "KB", "MB", "GB", "TB"};
  char length = sizeof(suffix) / sizeof(suffix[0]);
  auto dblBytes = static_cast<double>(size);
  auto i = 0;
  if (size > 1024) {
    for (i = 0; (size / 1024) > 0 && i < length - 1; i++, size /= 1024)
      dblBytes = size / 1024.0;
  }

  char output[200];
  sprintf(output, "%.02lf %s", dblBytes, suffix[i]);
  return output;
}
}