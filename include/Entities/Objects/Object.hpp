#pragma once
#include <optional>
#include "squirrel.h"
#include "SFML/Graphics.hpp"
#include "Entities/Entity.hpp"

namespace ng {
enum class ScreenSpace {
  Room,
  Object
};

enum class ObjectType {
  Object,
  Spot,
  Prop,
  Trigger
};

class Actor;
class Animation;
class Room;

namespace ObjectStateConstants {
static const int ALL = 1;
static const int HERE = 0;
static const int GONE = 4;
static const int OFF = 0;
static const int ON = 1;
static const int FULL = 0;
static const int EMPTY = 1;
static const int OPEN = 1;
static const int CLOSED = 0;
}

namespace ObjectFlagConstants {
static const int USE_WITH = 2;
static const int USE_ON = 4;
static const int USE_IN = 8;
static const int DOOR = 0x40;
static const int DOOR_LEFT = 0x140;
static const int DOOR_RIGHT = 0x240;
static const int DOOR_BACK = 0x440;
static const int DOOR_FRONT = 0x840;
static const int GIVEABLE = 0x1000;
static const int TALKABLE = 0x2000;
static const int IMMEDIATE = 0x4000;
static const int FEMALE = 0x80000;
static const int MALE = 0x100000;
static const int PERSON = 0x200000;
static const int REACH_HIGH = 0x8000;
static const int REACH_MED = 0x10000;
static const int REACH_LOW = 0x20000;
static const int REACH_NONE = 0x40000;
}

class Object : public Entity {
public:
  Object();
  ~Object() override;

  void setKey(const std::string &key);
  const std::string &getKey() const;

  void setZOrder(int zorder);
  int getZOrder() const override;

  void setType(ObjectType type);
  ObjectType getType() const;

  bool isTouchable() const override;
  bool isInventoryObject() const override;

  void setHotspot(const sf::IntRect &hotspot);
  const sf::IntRect &getHotspot() const;
  sf::IntRect getRealHotspot() const;
  void showHotspot(bool show);
  bool isHotspotVisible() const;

  void setIcon(const std::string &icon);
  void setIcon(int fps, const std::vector<std::string> &icons);
  std::string getIcon() const;

  HSQOBJECT &getTable() override;
  HSQOBJECT &getTable() const override;

  std::vector<std::unique_ptr<Animation>> &getAnims();

  bool isVisible() const override;
  void setStateAnimIndex(int animIndex);
  void playAnim(int animIndex, bool loop);
  void playAnim(const std::string &anim, bool loop);
  int getState();
  void setAnimation(const std::string &name);
  std::optional<Animation *> &getAnimation();

  Room *getRoom() override;
  const Room *getRoom() const override;
  void setRoom(Room *pRoom);

  void update(const sf::Time &elapsed) override;

  friend std::wostream &operator<<(std::wostream &os, const Object &obj);
  void drawHotspot(sf::RenderTarget &target, sf::RenderStates states) const;

  void addTrigger(const std::shared_ptr<Trigger> &trigger);
  void removeTrigger();
  Trigger *getTrigger();
  void enableTrigger(bool enabled);

  void dependentOn(Object *parentObject, int state);
  void addChild(Object *child);

  void setFps(int fps) override;

  Actor *getOwner() const;
  void setOwner(Actor *pActor);

  void setScreenSpace(ScreenSpace screenSpace);
  ScreenSpace getScreenSpace() const;

  void stopObjectMotors() override;

  void setTemporary(bool isTemporary);
  bool isTemporary() const;

  void setJiggle(bool enabled);
  bool getJiggle() const;

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
  void drawForeground(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;
};
} // namespace ng
