#pragma once
#include <sstream>
#include <iostream>
#include <optional>
#include "SFML/Graphics.hpp"
#include "NonCopyable.h"
#include "TextureManager.h"
#include "Animation.h"
#include "Font.h"
#include "Entity.h"
#include "Actor.h"

namespace ng
{
enum class UseDirection
{
  Front,
  Back,
  Left,
  Right
};

class Room;

class Object : public Entity
{
public:
  Object();
  ~Object();

  void setZOrder(int zorder) { _zorder = zorder; }
  int getZOrder() const override { return _zorder; }

  void setProp(bool prop) { _prop = prop; }
  bool getProp() const { return _prop; }

  void setSpot(bool spot) { _spot = spot; }
  bool getSpot() const { return _spot; }

  void setTrigger(bool trigger) { _trigger = trigger; }
  bool getTrigger() const { return _trigger; }

  void setTouchable(bool isTouchable) { _isTouchable = isTouchable; }
  bool isTouchable() const
  {
    if (_trigger)
      return false;
    if (_spot)
      return false;
    if (_prop)
      return false;
    return _isTouchable;
  }

  void setLit(bool isLit) { _isLit = isLit; }
  bool isLit() const { return _isLit; }

  void setUseDirection(UseDirection direction) { _direction = direction; }
  UseDirection getUseDirection() const { return _direction; }

  void setHotspot(const sf::IntRect &hotspot) { _hotspot = hotspot; }
  const sf::IntRect &getHotspot() const { return _hotspot; }
  sf::IntRect getRealHotspot() const;

  void setName(const std::string &name) { _name = name; }
  const std::string &getName() const { return _name; }

  void setId(const std::string &id) { _id = id; }
  const std::string &getId() const { return _id; }

  void setDefaultVerb(const std::string &verb) { _verb = verb; }
  const std::string &getDefaultVerb() const { return _verb; }

  std::vector<std::unique_ptr<Animation>> &getAnims() { return _anims; }

  void setStateAnimIndex(int animIndex);
  int getStateAnimIndex();
  void setAnimation(const std::string &name);

  void move(const sf::Vector2f &offset);
  void setPosition(const sf::Vector2f &pos);
  sf::Vector2f getPosition() const;
  sf::Vector2f getDefaultPosition() const;

  void setRotation(float angle) { _transform.setRotation(angle); }
  const float getRotation() const { return _transform.getRotation(); }

  void setColor(const sf::Color &color);
  const sf::Color &getColor() const;

  void setVisible(bool isVisible);
  bool isVisible() const { return _isVisible; }
  void setScale(float s);

  Actor *getOwner() { return _pOwner; }
  void setOwner(Actor *pActor) { _pOwner = pActor; }

  Room *getRoom() { return _pRoom; }
  void setRoom(Room *pRoom) { _pRoom = pRoom; }

  // TODO: void setIcon(const std::string& icon);

  void update(const sf::Time &elapsed) override;

  friend std::ostream &operator<<(std::ostream &os, const Object &obj);
  void drawHotspot(sf::RenderTarget &target, sf::RenderStates states) const;

  void addTrigger(std::shared_ptr<Trigger> trigger) { _triggers.push_back(trigger);}
  void removeTrigger() { _triggers.clear();}

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
  std::vector<std::unique_ptr<Animation>> _anims;
  std::optional<Animation> _pAnim;
  bool _isVisible;
  std::string _name, _id;
  int _zorder;
  UseDirection _direction;
  bool _prop;
  bool _spot;
  bool _trigger;
  sf::Vector2f _usePos;
  sf::Color _color;
  sf::IntRect _hotspot;
  sf::Transformable _transform;
  float _angle;
  bool _isTouchable;
  bool _isLit;
  Actor *_pOwner;
  Room *_pRoom;
  int _state;
  std::string _verb;
  std::vector<std::shared_ptr<Trigger>> _triggers;
  std::optional<sf::Vector2f> _defaultPosition;
};
} // namespace ng
