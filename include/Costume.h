#pragma once
#include <unordered_map>
#include <sstream>
#include <set>
#include "SFML/Graphics.hpp"
#include "NonCopyable.h"
#include "EngineSettings.h"
#include "TextureManager.h"
#include "CostumeAnimation.h"

namespace ng
{
enum class Facing
{
  FACE_FRONT = 0x4,
  FACE_BACK  = 0x8,
  FACE_LEFT  = 0x2,
  FACE_RIGHT = 0x1
};

class Actor;
class Costume;
class BlinkState
{
public:
    explicit BlinkState(Costume& costume);
    
    void setRate(double min, double max);
    void update(sf::Time elapsed);

private:
    Costume& _costume;
    double _min{0};
    double _max{0};
    sf::Time _value;
    int32_t _state{-1};
    sf::Time _elapsed;
};

class Costume : public sf::Drawable
{
public:
  explicit Costume(TextureManager &textureManager);
  ~Costume() override;

  void loadCostume(const std::string &name, const std::string &sheet = "");
  void lockFacing(Facing left, Facing right, Facing front, Facing back);
  void unlockFacing();
  void resetLockFacing();
  void setFacing(Facing facing);
  Facing getFacing() const;
  void setState(const std::string &name);
  bool setAnimation(const std::string &name);
  CostumeAnimation *getAnimation() { return _pCurrentAnimation.get(); }
  void setLayerVisible(const std::string &name, bool isVisible);
  void setHeadIndex(int index);

  void setAnimationNames(const std::string &headAnim, const std::string &standAnim, const std::string &walkAnim, const std::string &reachAnim);
  void setActor(Actor *pActor) { _pActor = pActor; }

  void setBlinkRate(double min, double max);

  void update(const sf::Time &elapsed);

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
  void updateAnimation();

private:
  EngineSettings &_settings;
  TextureManager &_textureManager;
  std::string _path;
  std::string _sheet;
  std::unique_ptr<CostumeAnimation> _pCurrentAnimation;
  sf::Texture _texture;
  Facing _facing;
  std::string _animation;
  std::set<std::string> _hiddenLayers;
  std::string _headAnimName;
  std::string _standAnimName;
  std::string _walkAnimName;
  std::string _reachAnimName;
  int _headIndex;
  Actor *_pActor;
  BlinkState _blinkState;
  std::unordered_map<Facing,Facing> _facings;
  bool _lockFacing{false};
};
} // namespace ng