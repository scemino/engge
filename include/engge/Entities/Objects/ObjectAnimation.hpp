#pragma once
#include <string>
#include <functional>
#include <vector>
#include <glm/vec2.hpp>
#include <ngf/Graphics/Texture.h>
#include <ngf/System/TimeSpan.h>
#include <engge/Graphics/SpriteSheetItem.h>
#include <engge/Entities/Objects/AnimState.hpp>

namespace ng {
struct ObjectAnimation {
  std::string name;
  const ngf::Texture *texture{nullptr};
  std::vector<SpriteSheetItem> frames;
  std::vector<ObjectAnimation> layers;
  std::vector<glm::ivec2> offsets;
  std::vector<std::string> triggers;
  std::vector<std::function<void()>> callbacks;
  bool loop{false};
  int fps{0};
  int flags{0};
  int frameIndex{0};
  ng::AnimState state{AnimState::Pause};
  ngf::TimeSpan elapsed;
  bool visible{true};
};
}
