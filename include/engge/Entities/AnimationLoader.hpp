#pragma once
#include <vector>
#include <engge/Entities/Entity.hpp>
#include <engge/Graphics/Animation.hpp>
#include <engge/Graphics/SpriteSheet.hpp>
#include <ngf/IO/GGPackValue.h>

namespace ng {
class AnimationLoader final {
public:
  static std::vector<Animation> parseAnimations(
      Entity &entity,
      const ngf::GGPackValue &gAnimations,
      const SpriteSheet &spriteSheet);
};
}