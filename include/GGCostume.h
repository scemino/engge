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

class GGCostume : public NonCopyable
{
public:
  GGCostume(const GGEngineSettings &settings);
  ~GGCostume();

  void loadCostume(const std::string &name);
  const std::string &getSheet() const { return _sheet; }
  const std::vector<GGLayer> &getAnim(const std::string &name) const { return *_animations.at(name).get(); }
  void lockFacing(Facing facing);
  void setState(const std::string& name);
  void setAnim(const std::string& name);

  void draw(sf::RenderWindow &window) const;
  void update(const sf::Time &elapsed);

private:
  void updateAnim();

private:
  const GGEngineSettings &_settings;
  std::string _sheet;
  std::map<std::string, std::unique_ptr<std::vector<GGLayer>>> _animations;
  std::vector<GGLayer> *_pCurrentAnim;
  std::vector<sf::Sprite> _sprites;
  sf::Texture _texture;
  Facing _facing;
  std::string _anim;
};
} // namespace gg