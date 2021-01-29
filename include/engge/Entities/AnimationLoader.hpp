#pragma once
#include <cstdlib>
#include <glm/vec2.hpp>
#include <engge/Entities/Entity.hpp>
#include <engge/Entities/Objects/ObjectAnimation.hpp>
#include <engge/Graphics/SpriteSheet.hpp>
#include <engge/Parsers/GGPackValue.hpp>

namespace ng {
class AnimationLoader final {
public:
  static std::vector<ObjectAnimation> parseAnimations(
      Entity &entity,
      const GGPackValue &gAnimations,
      const SpriteSheet &spriteSheet) {
    std::vector<ObjectAnimation> anims;
    if (gAnimations.isNull())
      return anims;
    for (const auto &gAnimation : gAnimations.array_value) {
      if (gAnimation.isNull())
        continue;
      anims.push_back(parseAnimation(entity, gAnimation, spriteSheet));
    }
    return anims;
  }

private:
  static bool toBool(const GGPackValue &gValue) {
    return !gValue.isNull() && gValue.getInt() == 1;
  }

  static glm::ivec2 parseIVec2(std::string_view value) {
    char *ptr;
    auto x = std::strtol(value.data() + 1, &ptr, 10);
    auto y = std::strtol(ptr + 1, &ptr, 10);
    return glm::ivec2{x, y};
  }

  static ObjectAnimation parseAnimation(Entity &entity,
                                        const GGPackValue &gAnimation,
                                        const SpriteSheet &defaultSpriteSheet) {
    const SpriteSheet *spriteSheet = &defaultSpriteSheet;
    ObjectAnimation anim;
    if (gAnimation["sheet"].isString()) {
      spriteSheet = &Locator<ResourceManager>::get().getSpriteSheet(gAnimation["sheet"].getString());
    }
    anim.texture = spriteSheet->getTextureName();
    anim.name = gAnimation["name"].getString();
    anim.loop = toBool(gAnimation["loop"]);
    anim.fps = gAnimation["fps"].isNull() ? 0 : gAnimation["fps"].getInt();
    anim.flags = gAnimation["flags"].isNull() ? 0 : gAnimation["flags"].getInt();
    if (!gAnimation["frames"].isNull()) {
      for (const auto &gFrame : gAnimation["frames"].array_value) {
        auto name = gFrame.getString();
        if (name == "null") {
          SpriteSheetItem item;
          item.isNull = true;
          anim.frames.push_back(item);
        } else {
          anim.frames.push_back(spriteSheet->getItem(name));
        }
      }
    }

    if (!gAnimation["layers"].isNull()) {
      for (const auto &gLayer : gAnimation["layers"].array_value) {
        auto layer = parseAnimation(entity, gLayer, *spriteSheet);
        anim.layers.push_back(layer);
      }
    }
    if (!gAnimation["offsets"].isNull()) {
      for (const auto &gOffset : gAnimation["offsets"].array_value) {
        auto offset = parseIVec2(gOffset.getString());
        anim.offsets.push_back(offset);
      }
    }
    if (!gAnimation["triggers"].isNull()) {
      auto numTriggers = gAnimation["triggers"].array_value.size();
      anim.callbacks.resize(numTriggers);
      anim.triggers.resize(numTriggers);
      for (auto i = 0; i < static_cast<int>(gAnimation["triggers"].array_value.size()); i++) {
        const auto &gTrigger = gAnimation["triggers"].array_value[i];
        if (gTrigger.isNull())
          continue;
        auto trigName = gTrigger.getString();
        anim.triggers[i] = trigName;
        anim.callbacks[i] = [&entity, trigName]() { entity.trig(trigName); };
      }
    }
    return anim;
  }
};
}