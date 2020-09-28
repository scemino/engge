#pragma once
#include <imgui-SFML.h>
#include <imgui.h>

namespace ng {
class _TextureTools {
public:
  bool _texturesVisible{false};

  void render() {
    if (!_texturesVisible)
      return;

    ImGui::Begin("Textures", &_texturesVisible);
    const auto &map = Locator<ResourceManager>::get().getTextureMap();
    size_t totalSize = 0;
    for (const auto&[key, value] :map) {
      totalSize += value._size;
    }
    auto totalSizeText = convertSize(totalSize);
    ImGui::Text("Total memory: %s", totalSizeText.data());
    ImGui::Separator();
    ImGui::Columns(3);
    ImGui::Text("Name"); ImGui::NextColumn();
    ImGui::Text("Size"); ImGui::NextColumn();
    ImGui::Text("Refs"); ImGui::NextColumn();
    ImGui::Separator();
    for (const auto&[key, value] :map) {
      auto fileSize = convertSize(value._size);
      ImGui::Text("%s", key.data()); ImGui::NextColumn();
      ImGui::Text("%s", fileSize.data());  ImGui::NextColumn();
      ImGui::Text("%ld", value._texture.use_count()); ImGui::NextColumn();
    }
    ImGui::Columns(1);
    ImGui::End();
  }

private:
  static std::string convertSize(size_t size) {
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
};
}