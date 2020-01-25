#pragma once
#include <unordered_map>
#include <sstream>
#include <set>
#include "SFML/Graphics.hpp"
#include "BlinkState.hpp"
#include "DirectionConstants.hpp"
#include "Parsers/GGPack.hpp"
#include "Graphics/SpriteSheet.hpp"
#include "Graphics/TextureManager.hpp"
#include "CostumeAnimation.hpp"

namespace ng
{

enum class Facing
{
  FACE_FRONT = DirectionConstants::FACE_FRONT,
  FACE_BACK  = DirectionConstants::FACE_BACK,
  FACE_LEFT  = DirectionConstants::FACE_LEFT,
  FACE_RIGHT = DirectionConstants::FACE_RIGHT
};

class Actor;

class Costume : public sf::Drawable
{
public:
  explicit Costume(TextureManager &textureManager);
  ~Costume() override;

  void loadCostume(const std::string &name, const std::string &sheet = "");
  std::string getPath() const {return _path;}
  void lockFacing(Facing left, Facing right, Facing front, Facing back);
  void unlockFacing();
  void resetLockFacing();
  void setFacing(Facing facing);
  Facing getFacing() const;
  void setState(const std::string &name);
  bool setAnimation(const std::string &name);
  CostumeAnimation *getAnimation() { return _pCurrentAnimation; }
  std::vector<std::unique_ptr<CostumeAnimation>>& getAnimations() { return _animations; }
  void setLayerVisible(const std::string &name, bool isVisible);
  void setHeadIndex(int index);

  void setAnimationNames(const std::string &headAnim, const std::string &standAnim, const std::string &walkAnim, const std::string &reachAnim);
  void setActor(Actor *pActor) { _pActor = pActor; }

  void setBlinkRate(double min, double max);

  void update(const sf::Time &elapsed);

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
  void updateAnimation();
  std::unique_ptr<CostumeLayer> loadLayer(const GGPackValue& jLayer) const;

private:
  TextureManager &_textureManager;
  std::string _path;
  std::string _sheet;
  std::vector<std::unique_ptr<CostumeAnimation>> _animations;
  CostumeAnimation* _pCurrentAnimation{nullptr};
  Facing _facing{Facing::FACE_FRONT};
  std::string _animation{"stand"};
  std::set<std::string> _hiddenLayers;
  std::string _headAnimName{"head"};
  std::string _standAnimName{"stand"};
  std::string _walkAnimName{"walk"};
  std::string _reachAnimName{"reach"};
  int _headIndex{0};
  Actor *_pActor{nullptr};
  BlinkState _blinkState;
  std::unordered_map<Facing,Facing> _facings;
  bool _lockFacing{false};
  SpriteSheet _costumeSheet;
  GGPackValue _hash;
};
} // namespace ng