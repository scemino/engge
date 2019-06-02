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
  FACE_FRONT = 0x4,
  FACE_BACK  = 0x8,
  FACE_LEFT  = 0x2,
  FACE_RIGHT = 0x1
};

class Actor;

class Costume : public sf::Drawable
{
public:
  explicit Costume(TextureManager &textureManager);
  ~Costume() override;

  void loadCostume(const std::string &name, const std::string &sheet = "");
  void lockFacing(Facing facing);
  void unlockFacing();
  void setFacing(Facing facing);
  Facing getFacing() const { return _facing; }
  void setState(const std::string &name);
  bool setAnimation(const std::string &name);
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
  EngineSettings &_settings;
  TextureManager &_textureManager;
  std::string _path;
  std::string _sheet;
  std::unique_ptr<CostumeAnimation> _pCurrentAnimation;
  sf::Texture _texture;
  Facing _facing;
  std::optional<Facing> _lockFacing;
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