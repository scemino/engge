#pragma once
#include <optional>
#include <sstream>
#include <set>
#include <unordered_map>
#include "BlinkState.hpp"
#include "DirectionConstants.hpp"
#include <ngf/Graphics/Drawable.h>
#include <ngf/Graphics/RenderStates.h>
#include <ngf/Graphics/RenderTarget.h>
#include <engge/Graphics/SpriteSheet.hpp>
#include <engge/Graphics/ResourceManager.hpp>
#include <engge/Entities/Objects/ObjectAnimation.hpp>
#include <engge/Entities/Objects/AnimControl.hpp>

namespace ng {

enum class Facing {
  FACE_FRONT = DirectionConstants::FACE_FRONT,
  FACE_BACK = DirectionConstants::FACE_BACK,
  FACE_LEFT = DirectionConstants::FACE_LEFT,
  FACE_RIGHT = DirectionConstants::FACE_RIGHT
};

enum class Reaching {
  High,
  Medium,
  Low
};

class Actor;

class Costume : public ngf::Drawable {
public:
  explicit Costume(ResourceManager &textureManager);
  ~Costume() override;

  void loadCostume(const std::string &name, const std::string &sheet = "");
  [[nodiscard]] std::string getPath() const { return _path; }
  [[nodiscard]] std::string getSheet() const { return _sheet; }
  void lockFacing(Facing left, Facing right, Facing front, Facing back);
  void unlockFacing();
  void resetLockFacing();
  [[nodiscard]] std::optional<Facing> getLockFacing() const;

  void setFacing(Facing facing);
  [[nodiscard]] Facing getFacing() const;
  void setState(const std::string &name, bool loop = false);
  void setStandState() { setState(_standAnimName); }
  void setWalkState() { setState(_walkAnimName, true); }
  void setReachState(Reaching reaching);
  bool setAnimation(const std::string &name);
  bool setMatchingAnimation(const std::string &animName);
  ObjectAnimation *getAnimation() { return _pCurrentAnimation; }
  AnimControl& getAnimControl() { return _animControl; }
  std::vector<ObjectAnimation> &getAnimations() { return _animations; }
  void setLayerVisible(const std::string &name, bool isVisible);
  void setHeadIndex(int index);
  [[nodiscard]] int getHeadIndex() const;

  void setAnimationNames(const std::string &headAnim,
                         const std::string &standAnim,
                         const std::string &walkAnim,
                         const std::string &reachAnim);
  void getAnimationNames(std::string &headAnim,
                         std::string &standAnim,
                         std::string &walkAnim,
                         std::string &reachAnim) const;

  void setActor(Actor *pActor) { _pActor = pActor; }

  void setBlinkRate(double min, double max);

  void update(const ngf::TimeSpan &elapsed);

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const override;

private:
  void updateAnimation();

private:
  ResourceManager &_textureManager;
  std::string _path;
  std::string _sheet;
  std::vector<ObjectAnimation> _animations;
  ObjectAnimation *_pCurrentAnimation{nullptr};
  Facing _facing{Facing::FACE_FRONT};
  std::set<std::string> _hiddenLayers;
  std::string _animation{"stand"};
  std::string _standAnimName{"stand"};
  std::string _headAnimName{"head"};
  std::string _walkAnimName{"walk"};
  std::string _reachAnimName{"reach"};
  int _headIndex{0};
  Actor *_pActor{nullptr};
  BlinkState _blinkState;
  std::unordered_map<Facing, Facing> _facings;
  bool _lockFacing{false};
  SpriteSheet _costumeSheet;
  AnimControl _animControl;
};
} // namespace ng