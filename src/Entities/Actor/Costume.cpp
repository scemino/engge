#include <filesystem>
#include <iostream>
#include "engge/Entities/Actor/Actor.hpp"
#include "engge/Entities/Actor/BlinkState.hpp"
#include "engge/Entities/Actor/Costume.hpp"
#include "engge/Entities/Objects/AnimationFrame.hpp"
#include "engge/Engine/EngineSettings.hpp"
#include "engge/System/Locator.hpp"
#include "../../System/_Util.hpp"
#include "engge/Parsers/GGPackValue.hpp"
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
  if (_pCurrentAnimation->getLayers().empty())
    return;
  auto it =
      std::find_if(_pCurrentAnimation->getLayers().begin(), _pCurrentAnimation->getLayers().end(), [name](auto &layer) {
        return layer.getName() == name;
      });
  if (it != _pCurrentAnimation->getLayers().end()) {
    it->setVisible(isVisible);
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
    _pCurrentAnimation->play(loop);
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

CostumeLayer Costume::loadLayer(const GGPackValue &jLayer) const {
  auto name = jLayer["name"].string_value;
  Animation animation(_costumeSheet.getTextureName(), name);
  auto fps = jLayer["fps"].isNull() ? _pActor->getFps() : jLayer["fps"].int_value;
  animation.setFps(fps);

  for (const auto &jFrame : jLayer["frames"].array_value) {
    auto frameName = jFrame.string_value;
    if (frameName == "null") {
      animation.addFrame(AnimationFrame(sf::IntRect()));
    } else {
      auto rect = _costumeSheet.getRect(frameName);
      auto size = _costumeSheet.getSourceSize(frameName);
      auto sourceRect = _costumeSheet.getSpriteSourceSize(frameName);
      AnimationFrame frame(rect);
      frame.setSourceRect(sourceRect);
      frame.setSize(size);
      animation.addFrame(std::move(frame));
    }
  }
  if (!jLayer["triggers"].isNull()) {
    size_t i = 0;
    for (const auto &jTrigger : jLayer["triggers"].array_value) {
      if (!jTrigger.isNull()) {
        auto triggerName = jTrigger.string_value;
        char *end;
        auto trigger = std::strtol(triggerName.data() + 1, &end, 10);
        if (end == triggerName.data() + 1) {
          animation.at(i).setCallback([triggerName, this]() {
            _pActor->trigSound(triggerName.data() + 1);
          });
        } else {
          animation.at(i).setCallback([trigger, this]() {
            _pActor->trig(trigger);
          });
        }
      }
      ++i;
    }
  }
  size_t i = 0;
  for (const auto &jOffset : jLayer["offsets"].array_value) {
    auto offset = _parsePos(jOffset.string_value);
    auto &frame = animation.at(i);
    frame.setOffset(offset);
    ++i;
  }

  CostumeLayer layer(std::move(animation));
  layer.setName(name);
  if (!jLayer["flags"].isNull()) {
    layer.setFlags(jLayer["flags"].int_value);
  }
  if (!jLayer["loop"].isNull() && jLayer["loop"].int_value == 1) {
    layer.setLoop(true);
  }
  layer.setActor(_pActor);
  return layer;
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
  for (auto j : hash["animations"].array_value) {
    auto name = j["name"].string_value;
    auto flags = j["flags"].isInteger() ? j["flags"].int_value : 0;
    if(!j["sheet"].isNull()) {
      _costumeSheet.load(j["sheet"].getString());
    }
    CostumeAnimation animation;
    animation.setName(name);
    animation.setFlags(flags);
    if (j["layers"].isNull()) {
      auto layer = loadLayer(j);
      animation.getLayers().push_back(std::move(layer));
    } else {
      for (const auto &jLayer : j["layers"].array_value) {
        auto layer = loadLayer(jLayer);
        animation.getLayers().push_back(std::move(layer));
      }
    }
    _animations.push_back(std::move(animation));
  }

  // don't know if it's necessary, reyes has no costume in the intro
  setStandState();
}

bool Costume::setAnimation(const std::string &animName) {
  if (_pCurrentAnimation && _pCurrentAnimation->getName() == animName)
    return true;

  for (auto &anim : _animations) {
    if (anim.getName() == animName) {
      _pCurrentAnimation = &anim;
      for (auto &layer : _pCurrentAnimation->getLayers()) {
        auto layerName = layer.getName();
        layer.setVisible(_hiddenLayers.find(layerName) == _hiddenLayers.end());
      }

      _pCurrentAnimation->play();
      return true;
    }
  }

  return false;
}

bool Costume::setMatchingAnimation(const std::string &animName) {
  if (_pCurrentAnimation && startsWith(_pCurrentAnimation->getName(), animName))
    return true;

  for (auto &anim : _animations) {
    if (startsWith(anim.getName(), animName)) {
      _pCurrentAnimation = &anim;
      for (auto &layer : _pCurrentAnimation->getLayers()) {
        auto layerName = layer.getName();
        layer.setVisible(_hiddenLayers.find(layerName) == _hiddenLayers.end());
      }

      _pCurrentAnimation->play();
      return true;
    }
  }

  return false;
}

void Costume::updateAnimation() {
  // special case for eyes... bof
  if (_pCurrentAnimation && startsWith(_animation, "eyes_")) {
    auto &layers = _pCurrentAnimation->getLayers();
    for (auto &&layer : layers) {
      if (!startsWith(layer.getName(), "eyes_"))
        continue;
      setLayerVisible(layer.getName(), false);
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

  if (_pCurrentAnimation) {
    auto &layers = _pCurrentAnimation->getLayers();
    for (auto &&layer : layers) {
      layer.setLeftDirection(getFacing() == Facing::FACE_LEFT);
    }
  }

  setHeadIndex(_headIndex);
}

void Costume::update(const sf::Time &elapsed) {
  if (!_pCurrentAnimation)
    return;
  _pCurrentAnimation->update(elapsed);
  _blinkState.update(elapsed);
}

void Costume::draw(sf::RenderTarget &target, sf::RenderStates states) const {
  if (!_pCurrentAnimation)
    return;
  target.draw(*_pCurrentAnimation, states);
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
