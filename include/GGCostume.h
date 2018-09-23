#pragma once
#include <sstream>
#include "SFML/Graphics.hpp"
#include "NonCopyable.h"
#include "GGEngineSettings.h"
#include "GGLayer.h"

namespace gg
{
enum class Facing
{
  FACE_FRONT,
  FACE_BACK,
  FACE_LEFT,
  FACE_RIGHT
};

class CostumeAnimation
{
public:
  CostumeAnimation(const std::string &name);
  ~CostumeAnimation();
  const std::string &getName() const { return _name; }
  std::vector<GGLayer*> &getLayers() { return _layers; }

private:
  std::string _name;
  std::vector<GGLayer*> _layers;
};

class GGCostume : public NonCopyable
{
public:
  GGCostume(const GGEngineSettings &settings);
  ~GGCostume();

  void loadCostume(const std::string &name);
  void lockFacing(Facing facing);
  void setState(const std::string &name);
  void setAnimation(const std::string &name);

  void draw(sf::RenderWindow &window, const sf::RenderStates &states) const;
  void update(const sf::Time &elapsed);

private:
  void updateAnimation();

private:
  const GGEngineSettings &_settings;
  std::string _sheet;
  std::vector<std::unique_ptr<CostumeAnimation>> _animations;
  CostumeAnimation *_pCurrentAnimation;
  std::vector<sf::Sprite> _sprites;
  sf::Texture _texture;
  Facing _facing;
  std::string _animation;
};
} // namespace gg