#pragma once
#include <string>
#include <ngf/Graphics/Rect.h>
#include <glm/vec2.hpp>

namespace ng {
struct SpriteSheetItem {
  std::string name;
  ngf::irect frame{};
  ngf::irect spriteSourceSize{};
  glm::ivec2 sourceSize{};
  bool isNull{true};
};
}
