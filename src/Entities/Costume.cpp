#include <filesystem>
#include <iostream>
#include <engge/Graphics/AnimDrawable.hpp>
#include <engge/Entities/Actor.hpp>
#include <engge/Entities/BlinkState.hpp>
#include <engge/Entities/Costume.hpp>
#include <engge/Engine/EngineSettings.hpp>
#include <engge/System/Locator.hpp>
#include <engge/Entities/AnimationLoader.hpp>
#include <engge/Room/Room.hpp>
#include "Util/Util.hpp"

namespace fs = std::filesystem;

namespace ng {
Costume::Costume(ResourceManager &textureManager)
    : m_textureManager(textureManager),
      m_blinkState(*this) {
  resetLockFacing();
  setLayerVisible("eyes_left", false);
  setLayerVisible("eyes_right", false);
}

Costume::~Costume() = default;

void Costume::setLayerVisible(const std::string &name, bool isVisible) {
  if (!isVisible) {
    m_hiddenLayers.emplace(name);
  } else {
    m_hiddenLayers.erase(name);
  }
  if (m_pCurrentAnimation == nullptr)
    return;
  if (m_pCurrentAnimation->layers.empty())
    return;
  auto it =
      std::find_if(m_pCurrentAnimation->layers.begin(), m_pCurrentAnimation->layers.end(), [name](auto &layer) {
        return layer.name == name;
      });
  if (it != m_pCurrentAnimation->layers.end()) {
    it->visible = isVisible;
  }
}

Facing Costume::getFacing() const {
  if (m_lockFacing) {
    return m_facings.at(m_facing);
  }
  return m_facing;
}

void Costume::setFacing(Facing facing) {
  if (m_facing == facing)
    return;
  m_facing = facing;
  updateAnimation();
}

void Costume::lockFacing(Facing left, Facing right, Facing front, Facing back) {
  m_facings[Facing::FACE_LEFT] = left;
  m_facings[Facing::FACE_RIGHT] = right;
  m_facings[Facing::FACE_FRONT] = front;
  m_facings[Facing::FACE_BACK] = back;
  m_lockFacing = true;
}

void Costume::resetLockFacing() {
  m_facings[Facing::FACE_LEFT] = Facing::FACE_LEFT;
  m_facings[Facing::FACE_RIGHT] = Facing::FACE_RIGHT;
  m_facings[Facing::FACE_FRONT] = Facing::FACE_FRONT;
  m_facings[Facing::FACE_BACK] = Facing::FACE_BACK;
}

void Costume::unlockFacing() {
  m_lockFacing = false;
}

std::optional<Facing> Costume::getLockFacing() const {
  if (!m_lockFacing)
    return std::nullopt;
  auto frontFacing = m_facings.find(Facing::FACE_FRONT)->second;
  auto backFacing = m_facings.find(Facing::FACE_BACK)->second;
  if (frontFacing != backFacing)
    return std::nullopt;
  return frontFacing;
}

void Costume::setState(const std::string &name, bool loop) {
  m_animation = name;
  auto pOldAnim = m_pCurrentAnimation;
  updateAnimation();
  if (pOldAnim != m_pCurrentAnimation) {
    m_animControl.play(loop);
  } else {
    m_animControl.resume(loop);
  }
}

void Costume::setReachState(Reaching reaching) {
  std::string animName;
  switch (reaching) {
  case Reaching::High:animName = m_reachAnimName + "_high";
    break;
  case Reaching::Medium:animName = m_reachAnimName + "_med";
    break;
  case Reaching::Low:animName = m_reachAnimName + "_low";
    break;
  }
  setState(animName);
  if (!m_pCurrentAnimation) {
    setState(m_reachAnimName);
  }
}

void Costume::loadCostume(const std::string &path, const std::string &sheet) {
  m_path = path;
  auto costumeSheet = m_sheet = sheet;

  auto costumePath = fs::path(path);
  if (!costumePath.has_extension()) {
    costumePath.replace_extension(".json");
  }
  auto hash = Locator<EngineSettings>::get().readEntry(costumePath.string());
  if (costumeSheet.empty()) {
    costumeSheet = hash["sheet"].getString();
  }

  m_costumeSheet.setTextureManager(&m_textureManager);

  if (!costumeSheet.empty()) {
    m_costumeSheet.load(costumeSheet);
  }

  // load animations
  m_animations.clear();
  m_pCurrentAnimation = nullptr;
  m_animControl.setAnimation(nullptr);
  setHeadIndex(m_headIndex);

  m_animations = AnimationLoader::parseAnimations(*m_pActor, hash["animations"], m_costumeSheet);

  // don't know if it's necessary, reyes has no costume in the intro
  setStandState();
}

bool Costume::setAnimation(const std::string &animName) {
  if (m_pCurrentAnimation && m_pCurrentAnimation->name == animName)
    return true;

  for (auto &anim : m_animations) {
    if (anim.name == animName) {
      m_pCurrentAnimation = &anim;
      m_animControl.setAnimation(m_pCurrentAnimation);
      for (auto &layer : m_pCurrentAnimation->layers) {
        auto layerName = layer.name;
        layer.visible = m_hiddenLayers.find(layerName) == m_hiddenLayers.end();
      }

      m_animControl.play();
      return true;
    }
  }

  return false;
}

bool Costume::setMatchingAnimation(const std::string &animName) {
  if (m_pCurrentAnimation && startsWith(m_pCurrentAnimation->name, animName))
    return true;

  for (auto &anim : m_animations) {
    if (startsWith(anim.name, animName)) {
      m_pCurrentAnimation = &anim;
      m_animControl.setAnimation(m_pCurrentAnimation);
      for (auto &layer : m_pCurrentAnimation->layers) {
        auto layerName = layer.name;
        layer.visible = m_hiddenLayers.find(layerName) == m_hiddenLayers.end();
      }

      m_animControl.play();
      return true;
    }
  }

  return false;
}

void Costume::updateAnimation() {
  std::string animName = m_animation;
  if (animName == "stand") {
    animName = m_standAnimName;
  } else if (animName == "head") {
    animName = m_headAnimName;
  } else if (animName == "walk") {
    animName = m_walkAnimName;
  } else if (animName == "reach") {
    animName = m_reachAnimName;
  }

  // special case for eyes... bof
  if (m_pCurrentAnimation && startsWith(animName, "eyes_")) {
    auto &layers = m_pCurrentAnimation->layers;
    for (auto &&layer : layers) {
      if (!startsWith(layer.name, "eyes_"))
        continue;
      setLayerVisible(layer.name, false);
    }
    setLayerVisible(animName, true);
    return;
  }

  if (!setAnimation(animName)) {
    std::string name(animName);
    name.append("_");
    switch (getFacing()) {
    case Facing::FACE_BACK:name.append("back");
      break;
    case Facing::FACE_FRONT:name.append("front");
      break;
    case Facing::FACE_LEFT:
    case Facing::FACE_RIGHT:name.append("right");
      break;
    }
    if (!setAnimation(name)) {
      setMatchingAnimation(name);
    }
  }

  setHeadIndex(m_headIndex);
}

void Costume::update(const ngf::TimeSpan &elapsed) {
  if (!m_pCurrentAnimation)
    return;
  m_animControl.update(elapsed);
  m_blinkState.update(elapsed);
}

void Costume::draw(ngf::RenderTarget &target, ngf::RenderStates states) const {
  if (!m_pCurrentAnimation)
    return;
  AnimDrawable animDrawable;
  animDrawable.setAnim(m_pCurrentAnimation);
  animDrawable.setColor(m_pActor->getColor());
  if (getFacing() == Facing::FACE_LEFT)
    animDrawable.setFlipX(true);
  animDrawable.draw(m_pActor->getPosition(), target, states);
}

void Costume::setHeadIndex(int index) {
  m_headIndex = index;

  setLayerVisible(m_headAnimName, m_headIndex == 0);
  for (int i = 0; i < 6; i++) {
    auto name = m_headAnimName;
    name.append(std::to_string(i + 1));
    setLayerVisible(name, i == m_headIndex);
  }
}

int Costume::getHeadIndex() const { return m_headIndex; }

void Costume::setAnimationNames(const std::string &headAnim,
                                const std::string &standAnim,
                                const std::string &walkAnim,
                                const std::string &reachAnim) {
  if (!headAnim.empty()) {
    setLayerVisible(m_headAnimName, false);
    for (int i = 0; i < 6; ++i) {
      auto name = m_headAnimName;
      name.append(std::to_string(i + 1));
      setLayerVisible(name, false);
    }
    m_headAnimName = headAnim;
    setLayerVisible(m_headAnimName, m_headIndex == 0);
    for (int i = 0; i < 6; ++i) {
      auto name = m_headAnimName;
      name.append(std::to_string(i + 1));
      setLayerVisible(name, m_headIndex == i);
    }
  }
  if (!standAnim.empty()) {
    m_standAnimName = standAnim;
  }
  if (!walkAnim.empty()) {
    m_walkAnimName = walkAnim;
  }
  if (!reachAnim.empty()) {
    m_reachAnimName = reachAnim;
  }
  // update animation if necessary
  if (m_pCurrentAnimation) {
    m_pCurrentAnimation = nullptr;
    setState(m_animation, m_animControl.getLoop());
  }
}

void Costume::getAnimationNames(std::string &headAnim,
                                std::string &standAnim,
                                std::string &walkAnim,
                                std::string &reachAnim) const {
  headAnim = m_headAnimName;
  standAnim = m_standAnimName;
  walkAnim = m_walkAnimName;
  reachAnim = m_reachAnimName;
}

void Costume::setBlinkRate(float min, float max) {
  m_blinkState.setRate(min, max);
}
} // namespace ng
