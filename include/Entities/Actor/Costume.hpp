#pragma once
#include <optional>
#include <sstream>
#include <set>
#include <unordered_map>
#include "SFML/Graphics.hpp"
#include "BlinkState.hpp"
#include "DirectionConstants.hpp"
#include "Parsers/GGPack.hpp"
#include "Graphics/SpriteSheet.hpp"
#include "Graphics/TextureManager.hpp"
#include "CostumeAnimation.hpp"

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

class Costume : public sf::Drawable {
public:
  explicit Costume(TextureManager &textureManager);
  ~Costume() override;

  void loadCostume(const std::string &name, const std::string &sheet = "");
  std::string getPath() const { return _path; }
  std::string getSheet() const { return _sheet; }
  void lockFacing(Facing left, Facing right, Facing front, Facing back);
  void unlockFacing();
  void resetLockFacing();
  std::optional<Facing> getLockFacing() const;

  void setFacing(Facing facing);
  Facing getFacing() const;
  void setState(const std::string &name);
  void setStandState() { setState(_standAnimName); }
  void setWalkState() { setState(_walkAnimName); }
  void setReachState(Reaching reaching);
  bool setAnimation(const std::string &name);
  bool setMatchingAnimation(const std::string &animName);
  CostumeAnimation *getAnimation() { return _pCurrentAnimation; }
  std::vector<CostumeAnimation> &getAnimations() { return _animations; }
  void setLayerVisible(const std::string &name, bool isVisible);
  void setHeadIndex(int index);
  int getHeadIndex() const;

  void setAnimationNames(const std::string &headAnim,
                         const std::string &standAnim,
                         const std::string &walkAnim,
                         const std::string &reachAnim);
  void setActor(Actor *pActor) { _pActor = pActor; }

  void setBlinkRate(double min, double max);

  void update(const sf::Time &elapsed);

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
  void updateAnimation();
  CostumeLayer loadLayer(const GGPackValue &jLayer) const;

private:
  TextureManager &_textureManager;
  std::string _path;
  std::string _sheet;
  std::vector<CostumeAnimation> _animations;
  CostumeAnimation *_pCurrentAnimation{nullptr};
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
  GGPackValue _hash;
};
} // namespace ng