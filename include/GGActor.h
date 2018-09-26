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
  explicit GGActor(TextureManager &textureManager);
  ~GGActor();

  void setCostume(const std::string &name);
  GGCostume &getCostume() { return _costume; }
  const GGCostume &getCostume() const { return _costume; }
  void setTalkColor(sf::Color color) { _color = color; }
  sf::Color getTalkColor() { return _color; }
  void move(const sf::Vector2f &offset);
  sf::Vector2f getPosition() const { return _transform.transformPoint(0, 0); }
  void setPosition(const sf::Vector2f &pos);
  void setRenderOffset(const sf::Vector2i &offset) { _renderOffset = offset; }

  void draw(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const;
  void update(const sf::Time &time);

private:
  const GGEngineSettings &_settings;
  GGCostume _costume;
  sf::Transform _transform;
  sf::Color _color;
  sf::Vector2i _renderOffset;
};
} // namespace gg
