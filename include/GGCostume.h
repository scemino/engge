#pragma once
#include <sstream>
#include "SFML/Graphics.hpp"
#include "NonCopyable.h"
#include "GGEngineSettings.h"
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
  explicit GGCostume(const GGEngineSettings &settings);
  ~GGCostume();

  void loadCostume(const std::string &name);
  void lockFacing(Facing facing);
  void setState(const std::string &name);
  void setAnimation(const std::string &name);
  const GGCostumeAnimation *getAnimation() const { return _pCurrentAnimation; }
  GGCostumeAnimation *getAnimation() { return _pCurrentAnimation; }

  void draw(sf::RenderWindow &window, const sf::RenderStates &states) const;
  void update(const sf::Time &elapsed);

private:
  void updateAnimation();

private:
  const GGEngineSettings &_settings;
  std::string _sheet;
  std::vector<std::unique_ptr<GGCostumeAnimation>> _animations;
  GGCostumeAnimation *_pCurrentAnimation;
  sf::Texture _texture;
  Facing _facing;
  std::string _animation;
};
} // namespace gg