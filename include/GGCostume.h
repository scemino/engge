#pragma once
#include <sstream>
#include <set>
#include "SFML/Graphics.hpp"
#include "NonCopyable.h"
#include "GGEngineSettings.h"
#include "TextureManager.h"
#include "GGCostumeAnimation.h"

namespace gg
{
enum class Facing
{
  FACE_FRONT,
  FACE_BACK,
  FACE_LEFT,
  FACE_RIGHT
};

class GGCostume : public NonCopyable
{
public:
  explicit GGCostume(TextureManager &textureManager);
  ~GGCostume();

  void loadCostume(const std::string &name, const std::string &sheet = "");
  void lockFacing(Facing facing);
  void setState(const std::string &name);
  void setAnimation(const std::string &name);
  const std::string &getAnimationName() const { return _animation; }
  const GGCostumeAnimation *getAnimation() const { return _pCurrentAnimation.get(); }
  GGCostumeAnimation *getAnimation() { return _pCurrentAnimation.get(); }
  void setLayerVisible(const std::string &name, bool isVisible);
  void setHeadIndex(int index);

  void setAnimationNames(const std::string &headAnim, const std::string &standAnim, const std::string &walkAnim, const std::string &reachAnim);

  void draw(sf::RenderWindow &window, const sf::RenderStates &states) const;
  void update(const sf::Time &elapsed);

private:
  void updateAnimation();

private:
  const GGEngineSettings &_settings;
  TextureManager &_textureManager;
  std::string _path;
  std::string _sheet;
  std::unique_ptr<GGCostumeAnimation> _pCurrentAnimation;
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
} // namespace gg