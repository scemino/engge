#pragma once
#include <sstream>
#include "SFML/Graphics.hpp"
#include "NonCopyable.h"
#include "TextureManager.h"
#include "GGAnim.h"
#include "GGFont.h"

namespace gg
{
enum class UseDirection
{
  Front,
  Back,
  Left,
  Right
};

class GGObject
{
public:
  GGObject();
  ~GGObject();

  void setZOrder(int zorder) { _zorder = zorder; }
  int getZOrder() const { return _zorder; }

  void setProp(bool prop) { _prop = prop; }
  bool getProp() const { return _prop; }

  void setUseDirection(UseDirection direction) { _direction = direction; }
  UseDirection getUseDirection() const { return _direction; }

  void setHotspot(const sf::IntRect &hotspot) { _hotspot = hotspot; }
  const sf::IntRect &getHotspot() const { return _hotspot; }
  sf::IntRect getRealHotspot() const;

  void setName(const std::string &name) { _name = name; }
  const std::string &getName() const { return _name; }

  std::vector<std::unique_ptr<GGAnim>> &getAnims() { return _anims; }

  void setStateAnimIndex(int animIndex);
  void setAnim(const std::string &name);

  void move(float x, float y);
  void setPosition(float x, float y);
  sf::Vector2f getPosition() const;
  void setUsePosition(float x, float y);
  const sf::Vector2f &getUsePosition() const;
  void setRotation(float angle) { _transform.setRotation(angle); }
  const float getRotation() const { return _transform.getRotation(); }

  void setColor(const sf::Color &color);
  const sf::Color &getColor() const;

  void setVisible(bool isVisible) { _isVisible = isVisible; }
  bool isVisible() const { return _isVisible; }
  void setScale(float s);

  void setHotspotVisible(bool isHotspotVisible) { _isHotspotVisible = isHotspotVisible; }
  bool isHotspotVisible() { return _isHotspotVisible; }

  void update(const sf::Time &elapsed);
  virtual void draw(sf::RenderWindow &window);

private:
  void drawHotspot(sf::RenderWindow &window) const;

private:
  std::vector<std::unique_ptr<GGAnim>> _anims;
  GGAnim *_pAnim;
  bool _isVisible;
  std::string _name;
  int _zorder;
  UseDirection _direction;
  bool _prop;
  sf::Vector2f _usePos;
  sf::Color _color;
  sf::IntRect _hotspot;
  bool _isHotspotVisible;
  sf::Transformable _transform;
  float _angle;
};

class GGTextObject : public GGObject
{
public:
  explicit GGTextObject(GGFont &font);
  void setText(const std::string &text) { _text = text; }

private:
  virtual void draw(sf::RenderWindow &window);

private:
  GGFont &_font;
  std::string _text;
};
} // namespace gg
