#include <filesystem>
#include <iostream>
#include <engge/Entities/Objects/AnimDrawable.hpp>
#include <engge/Entities/Actor/Actor.hpp>
#include <engge/Entities/Actor/BlinkState.hpp>
#include <engge/Entities/Actor/Costume.hpp>
#include <engge/Engine/EngineSettings.hpp>
#include <engge/System/Locator.hpp>
#include <engge/Parsers/GGPackValue.hpp>
#include <engge/Entities/AnimationLoader.hpp>
#include <engge/Room/Room.hpp>
#include "../../System/_Util.hpp"

namespace fs = std::filesystem;

namespace ng {
Costume::Costume(ResourceManager &textureManager)
    : _textureManager(textureManager),
      _blinkState(*this) {
  resetLockFacing();
}

Costume::~Costume() = default;

void Costume::setLayerVisible(const std::string &name, bool isVisible) {
  if (!isVisible) {
    _hiddenLayers.emplace(name);
  } else {
    _hiddenLayers.erase(name);
  }
  if (_pCurrentAnimation == nullptr)
    return;
  if (_pCurrentAnimation->layers.empty())
    return;
  auto it =
      std::find_if(_pCurrentAnimation->layers.begin(), _pCurrentAnimation->layers.end(), [name](auto &layer) {
        return layer.name == name;
      });
  if (it != _pCurrentAnimation->layers.end()) {
    it->visible = isVisible;
  }
}

Facing Costume::getFacing() const {
  if (_lockFacing) {
    return _facings.at(_facing);
  }
  return _facing;
}

void Costume::setFacing(Facing facing) {
  if (_facing == facing)
    return;
  _facing = facing;
  updateAnimation();
}

void Costume::lockFacing(Facing left, Facing right, Facing front, Facing back) {
  _facings[Facing::FACE_LEFT] = left;
  _facings[Facing::FACE_RIGHT] = right;
  _facings[Facing::FACE_FRONT] = front;
  _facings[Facing::FACE_BACK] = back;
  _lockFacing = true;
}

void Costume::resetLockFacing() {
  _facings[Facing::FACE_LEFT] = Facing::FACE_LEFT;
  _facings[Facing::FACE_RIGHT] = Facing::FACE_RIGHT;
  _facings[Facing::FACE_FRONT] = Facing::FACE_FRONT;
  _facings[Facing::FACE_BACK] = Facing::FACE_BACK;
}

void Costume::unlockFacing() {
  _lockFacing = false;
}

std::optional<Facing> Costume::getLockFacing() const {
  if (!_lockFacing)
    return std::nullopt;
  auto frontFacing = _facings.find(Facing::FACE_FRONT)->second;
  auto backFacing = _facings.find(Facing::FACE_BACK)->second;
  if (frontFacing != backFacing)
    return std::nullopt;
  return frontFacing;
}

void Costume::setState(const std::string &name, bool loop) {
  _animation = name;
  auto pOldAnim = _pCurrentAnimation;
  updateAnimation();
  if (pOldAnim != _pCurrentAnimation) {
    _animControl.play(loop);
  }
}

void Costume::setReachState(Reaching reaching) {
  std::string animName;
  switch (reaching) {
  case Reaching::High:animName = _reachAnimName + "_high";
    break;
  case Reaching::Medium:animName = _reachAnimName + "_med";
    break;
  case Reaching::Low:animName = _reachAnimName + "_low";
    break;
  }
  setState(animName);
  if (!_pCurrentAnimation) {
    setState(_reachAnimName);
  }
}

void Costume::loadCostume(const std::string &path, const std::string &sheet) {
  _path = path;
  auto costumeSheet = _sheet = sheet;

  auto costumePath = fs::path(path);
  if (!costumePath.has_extension()) {
    costumePath.replace_extension(".json");
  }
  GGPackValue hash;
  Locator<EngineSettings>::get().readEntry(costumePath.string(), hash);
  if (costumeSheet.empty()) {
    costumeSheet = hash["sheet"].getString();
  }

  _costumeSheet.setTextureManager(&_textureManager);
  _costumeSheet.load(costumeSheet);

  // load animations
  _animations.clear();
  _pCurrentAnimation = nullptr;
  for (int i = 0; i < 6; i++) {
    std::ostringstream s;
    s << _headAnimName << (i + 1);
    auto layerName = s.str();
    setLayerVisible(layerName, i == _headIndex);
  }

  _animations = AnimationLoader::parseObjectAnimations(*_pActor, hash["animations"], _costumeSheet);

  // don't know if it's necessary, reyes has no costume in the intro
  setStandState();
}

bool Costume::setAnimation(const std::string &animName) {
  if (_pCurrentAnimation && _pCurrentAnimation->name == animName)
    return true;

  for (auto &anim : _animations) {
    if (anim.name == animName) {
      _pCurrentAnimation = &anim;
      _animControl.setAnimation(_pCurrentAnimation);
      for (auto &layer : _pCurrentAnimation->layers) {
        auto layerName = layer.name;
        layer.visible = _hiddenLayers.find(layerName) == _hiddenLayers.end();
      }

      _animControl.play();
      return true;
    }
  }

  return false;
}

bool Costume::setMatchingAnimation(const std::string &animName) {
  if (_pCurrentAnimation && startsWith(_pCurrentAnimation->name, animName))
    return true;

  for (auto &anim : _animations) {
    if (startsWith(anim.name, animName)) {
      _pCurrentAnimation = &anim;
      _animControl.setAnimation(_pCurrentAnimation);
      for (auto &layer : _pCurrentAnimation->layers) {
        auto layerName = layer.name;
        layer.visible = _hiddenLayers.find(layerName) == _hiddenLayers.end();
      }

      _animControl.play();
      return true;
    }
  }

  return false;
}

void Costume::updateAnimation() {
  // special case for eyes... bof
  if (_pCurrentAnimation && startsWith(_animation, "eyes_")) {
    auto &layers = _pCurrentAnimation->layers;
    for (auto &&layer : layers) {
      if (!startsWith(layer.name, "eyes_"))
        continue;
      setLayerVisible(layer.name, false);
    }
    setLayerVisible(_animation, true);
    return;
  }

  if (!setAnimation(_animation)) {
    std::string name(_animation);
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

  setHeadIndex(_headIndex);
}

void Costume::update(const ngf::TimeSpan &elapsed) {
  if (!_pCurrentAnimation)
    return;
  _animControl.update(elapsed);
  _blinkState.update(elapsed);
}

void Costume::draw(ngf::RenderTarget &target, ngf::RenderStates states) const {
  if (!_pCurrentAnimation)
    return;
  AnimDrawable animDrawable;
  animDrawable.setAnim(_pCurrentAnimation);
  animDrawable.setColor(_pActor->getColor());
  if (getFacing() == Facing::FACE_LEFT)
    animDrawable.setFlipX(true);
  animDrawable.draw(_pActor->getPosition(), target, states);
}

void Costume::setHeadIndex(int index) {
  _headIndex = index;
  if (!_pCurrentAnimation)
    return;
  for (int i = 0; i < 6; i++) {
    std::ostringstream s;
    s << _headAnimName;
    if (i != 0) {
      s << (i + 1);
    }
    auto layerName = s.str();
    setLayerVisible(layerName, i == _headIndex);
  }
}

int Costume::getHeadIndex() const { return _headIndex; }

void Costume::setAnimationNames(const std::string &headAnim,
                                const std::string &standAnim,
                                const std::string &walkAnim,
                                const std::string &reachAnim) {
  if (!headAnim.empty()) {
    _headAnimName = headAnim;
  }
  if (!standAnim.empty()) {
    _standAnimName = standAnim;
  }
  if (!walkAnim.empty()) {
    _walkAnimName = walkAnim;
  }
  if (!reachAnim.empty()) {
    _reachAnimName = reachAnim;
  }
}

void Costume::setBlinkRate(double min, double max) {
  _blinkState.setRate(min, max);
}
} // namespace ng
