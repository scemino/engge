#pragma once
#include <sstream>
#include <set>
#include "SFML/Graphics.hpp"
#include "NonCopyable.h"
#include "NGEngineSettings.h"
#include "TextureManager.h"
#include "NGCostumeAnimation.h"

namespace ng
{
enum class Facing
{
  FACE_FRONT,
  FACE_BACK,
  FACE_LEFT,
  FACE_RIGHT
};

class NGCostume : public sf::Drawable
{
public:
  explicit NGCostume(TextureManager &textureManager);
  ~NGCostume();

  void loadCostume(const std::string &name, const std::string &sheet = "");
  void lockFacing(Facing facing);
  void setFacing(Facing facing);
  void setState(const std::string &name);
  void setAnimation(const std::string &name);
  const std::string &getAnimationName() const { return _animation; }
  const NGCostumeAnimation *getAnimation() const { return _pCurrentAnimation.get(); }
  NGCostumeAnimation *getAnimation() { return _pCurrentAnimation.get(); }
  void setLayerVisible(const std::string &name, bool isVisible);
  void setHeadIndex(int index);

  void setAnimationNames(const std::string &headAnim, const std::string &standAnim, const std::string &walkAnim, const std::string &reachAnim);

  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
  void update(const sf::Time &elapsed);

private:
  void updateAnimation();

private:
  const NGEngineSettings &_settings;
  TextureManager &_textureManager;
  std::string _path;
  std::string _sheet;
  std::unique_ptr<NGCostumeAnimation> _pCurrentAnimation;
  sf::Texture _texture;
  Facing _facing;
  std::string _animation;
  std::set<std::string> _hiddenLayers;
  std::string _headAnimName;
  std::string _standAnimName;
  std::string _walkAnimName;
  std::string _reachAnimName;
  int _headIndex;
};
} // namespace ng