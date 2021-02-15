#include <glm/vec2.hpp>
#include <engge/Entities/AnimationLoader.hpp>
#include <engge/Entities/Entity.hpp>
#include <engge/Graphics/Animation.hpp>
#include <engge/Graphics/SpriteSheet.hpp>
#include <engge/System/Locator.hpp>
#include <ngf/IO/GGPackValue.h>

namespace ng {
namespace {

bool toBool(const ngf::GGPackValue &gValue) {
  return !gValue.isNull() && gValue.getInt() == 1;
}

glm::ivec2 parseIVec2(std::string_view value) {
  char *ptr;
  auto x = std::strtol(value.data() + 1, &ptr, 10);
  auto y = std::strtol(ptr + 1, &ptr, 10);
  return glm::ivec2{x, y};
}

Animation parseAnimation(Entity &entity,
                         const ngf::GGPackValue &gAnimation,
                         const SpriteSheet &defaultSpriteSheet) {
  const SpriteSheet *spriteSheet = &defaultSpriteSheet;
  Animation anim;
  if (gAnimation["sheet"].isString()) {
    spriteSheet = &Locator<ResourceManager>::get().getSpriteSheet(gAnimation["sheet"].getString());
  }
  anim.texture = spriteSheet->getTextureName();
  anim.name = gAnimation["name"].getString();
  anim.loop = toBool(gAnimation["loop"]);
  anim.fps = gAnimation["fps"].isNull() ? 0 : gAnimation["fps"].getInt();
  anim.flags = gAnimation["flags"].isNull() ? 0 : gAnimation["flags"].getInt();
  if (!gAnimation["frames"].isNull()) {
    for (const auto &gFrame : gAnimation["frames"]) {
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
    for (const auto &gLayer : gAnimation["layers"]) {
      auto layer = parseAnimation(entity, gLayer, *spriteSheet);
      anim.layers.push_back(layer);
    }
  }
  if (!gAnimation["offsets"].isNull()) {
    for (const auto &gOffset : gAnimation["offsets"]) {
      auto offset = parseIVec2(gOffset.getString());
      anim.offsets.push_back(offset);
    }
  }
  if (!gAnimation["triggers"].isNull()) {
    auto numTriggers = gAnimation["triggers"].size();
    anim.callbacks.resize(numTriggers);
    anim.triggers.resize(numTriggers);
    for (auto i = 0; i < static_cast<int>(gAnimation["triggers"].size()); i++) {
      const auto &gTrigger = gAnimation["triggers"][i];
      if (gTrigger.isNull())
        continue;
      auto trigName = gTrigger.getString();
      anim.triggers[i] = trigName;
      anim.callbacks[i] = [&entity, trigName]() { entity.trig(trigName); };
    }
  }
  return anim;
}
}

std::vector<Animation> AnimationLoader::parseAnimations(Entity &entity,
                                                        const ngf::GGPackValue &gAnimations,
                                                        const SpriteSheet &spriteSheet) {
  std::vector<Animation> anims;
  if (gAnimations.isNull())
    return anims;
  for (const auto &gAnimation : gAnimations) {
    if (gAnimation.isNull())
      continue;
    anims.push_back(parseAnimation(entity, gAnimation, spriteSheet));
  }
  return anims;
}
}