#pragma once
#include <sstream>
#include "SFML/Graphics.hpp"
#include "NonCopyable.h"
#include "TextureManager.h"
#include "GGCostume.h"

namespace gg
{
class GGActor : public NonCopyable
{
public:
  GGActor(TextureManager &textureManager);
  ~GGActor();

  void setCostume(const std::string &name);
  GGCostume &getCostume() { return _costume; }
  const GGCostume &getCostume() const { return _costume; }
  void setTalkColor(sf::Color color) { _color = color; }
  sf::Color getTalkColor() { return _color; }
  //   void setUseDirection(UseDirection direction) { _direction = direction; }
  //   UseDirection getUseDirection() const { return _direction; }
  void move(const sf::Vector2f &offset);
  void setPosition(const sf::Vector2f &pos);

  void draw(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const;
  void update(const sf::Time &time);

private:
  const GGEngineSettings &_settings;
  //   UseDirection _direction;
  GGCostume _costume;
  sf::Transform _transform;
  sf::Color _color;
};
} // namespace gg
