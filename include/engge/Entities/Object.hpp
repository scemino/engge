#pragma once
#include <optional>
#include <squirrel.h>
#include <engge/Entities/Entity.hpp>
#include <engge/Graphics/Animation.hpp>
#include <engge/Graphics/AnimControl.hpp>

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
static const uint32_t USE_WITH = 2;
static const uint32_t USE_ON = 4;
static const uint32_t USE_IN = 32;
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
  explicit Object(HSQOBJECT obj);
  ~Object() override;

  void setZOrder(int zorder);
  [[nodiscard]] int getZOrder() const override;

  void setType(ObjectType type);
  [[nodiscard]] ObjectType getType() const;

  [[nodiscard]] bool isTouchable() const override;
  [[nodiscard]] bool isInventoryObject() const override;

  void setHotspot(const ngf::irect &hotspot);
  [[nodiscard]] ngf::irect getHotspot() const;

  [[nodiscard]] ngf::irect getRealHotspot() const;
  void showDebugHotspot(bool show);
  [[nodiscard]] bool isHotspotVisible() const;

  void setIcon(const std::string &icon);
  void setIcon(int fps, const std::vector<std::string> &icons);
  [[nodiscard]] std::string getIcon() const;

  HSQOBJECT &getTable() override;
  [[nodiscard]] HSQOBJECT &getTable() const override;

  std::vector<Animation> &getAnims();

  void setStateAnimIndex(int animIndex);
  void playAnim(int animIndex, bool loop);
  void playAnim(const std::string &anim, bool loop);
  [[nodiscard]] int getState() const;
  void setAnimation(const std::string &name);
  Animation *getAnimation();
  const Animation *getAnimation() const;
  AnimControl &getAnimControl();

  Room *getRoom() override;
  [[nodiscard]] const Room *getRoom() const override;
  void setRoom(Room *pRoom);

  void update(const ngf::TimeSpan &elapsed) override;

  friend std::wostream &operator<<(std::wostream &os, const Object &obj);

  void addTrigger(const std::shared_ptr<Trigger> &trigger);
  void removeTrigger();
  Trigger *getTrigger();
  void enableTrigger(bool enabled);

  void dependentOn(Object *parentObject, int state);

  void setFps(int fps) override;

  [[nodiscard]] Actor *getOwner() const;
  void setOwner(Actor *pActor);

  void setScreenSpace(ScreenSpace screenSpace);
  [[nodiscard]] ScreenSpace getScreenSpace() const;

  void setTemporary(bool isTemporary);
  [[nodiscard]] bool isTemporary() const;

  void setJiggle(bool enabled);
  [[nodiscard]] bool getJiggle() const;

  void setPop(int count);
  [[nodiscard]] int getPop() const;
  [[nodiscard]] float getPopScale() const;

private:
  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const override;

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;
};
} // namespace ng
