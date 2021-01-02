#pragma once
#include <string>
#include <glm/vec2.hpp>
#include <engge/Graphics/SpriteSheetItem.h>
#include <ngf/System/TimeSpan.h>

namespace ng {
struct ObjectAnimation {
  std::string name;
  std::vector <SpriteSheetItem> frames;
  std::vector <ObjectAnimation> layers;
  std::vector <glm::ivec2> offsets;
  std::vector <std::string> triggers;
  bool loop{false};
  int fps{0};
  int flags{0};
  int frameIndex{0};
  ngf::TimeSpan elapsed;
  bool visible{true};
};
}
