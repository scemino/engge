#pragma once
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
  FACE_FRONT,
  FACE_BACK,
  FACE_LEFT,
  FACE_RIGHT
};

class Actor;

class Costume : public sf::Drawable
{
public:
  explicit Costume(TextureManager &textureManager);
  ~Costume();

  void loadCostume(const std::string &name, const std::string &sheet = "");
  void lockFacing(Facing facing);
  void setFacing(Facing facing);
  Facing getFacing() const { return _facing; }
  void setState(const std::string &name);
  bool setAnimation(const std::string &name);
  const std::string &getAnimationName() const { return _animation; }
  const CostumeAnimation *getAnimation() const { return _pCurrentAnimation.get(); }
  CostumeAnimation *getAnimation() { return _pCurrentAnimation.get(); }
  void setLayerVisible(const std::string &name, bool isVisible);
  void setHeadIndex(int index);

  void setAnimationNames(const std::string &headAnim, const std::string &standAnim, const std::string &walkAnim, const std::string &reachAnim);
  void setActor(Actor *pActor) { _pActor = pActor; }

  void update(const sf::Time &elapsed);

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
  void updateAnimation();

private:
  const EngineSettings &_settings;
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
};
} // namespace ng