#pragma once
#include <sstream>
#include <iostream>
#include "SFML/Graphics.hpp"
#include "NonCopyable.h"
#include "TextureManager.h"
#include "GGAnimation.h"
#include "GGFont.h"
#include "GGEntity.h"

namespace gg
{
enum class UseDirection
{
  Front,
  Back,
  Left,
  Right
};

class GGObject : public GGEntity
{
public:
  GGObject();
  ~GGObject();

  void setZOrder(int zorder) { _zorder = zorder; }
  int getZOrder() const override { return _zorder; }

  void setProp(bool prop) { _prop = prop; }
  bool getProp() const { return _prop; }

  void setTouchable(bool isTouchable) { _isTouchable = isTouchable; }
  bool isTouchable() const { return _isTouchable; }

  void setUseDirection(UseDirection direction) { _direction = direction; }
  UseDirection getUseDirection() const { return _direction; }

  void setHotspot(const sf::IntRect &hotspot) { _hotspot = hotspot; }
  const sf::IntRect &getHotspot() const { return _hotspot; }
  sf::IntRect getRealHotspot() const;

  void setName(const std::string &name) { _name = name; }
  const std::string &getName() const { return _name; }

  std::vector<std::unique_ptr<GGAnimation>> &getAnims() { return _anims; }

  void setStateAnimIndex(int animIndex);
  int getStateAnimIndex();
  void setAnimation(const std::string &name);

  void move(const sf::Vector2f &offset);
  void setPosition(const sf::Vector2f &pos);
  sf::Vector2f getPosition() const;
  void setUsePosition(const sf::Vector2f &pos);
  sf::Vector2f getUsePosition() const;
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

  void draw(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const override;

  friend std::ostream &operator<<(std::ostream &os, const GGObject &obj);

private:
  void drawHotspot(sf::RenderWindow &window, sf::RenderStates states) const;

private:
  std::vector<std::unique_ptr<GGAnimation>> _anims;
  GGAnimation *_pAnim;
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
  bool _isTouchable;
};
} // namespace gg
