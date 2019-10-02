#pragma once
#include <optional>
#include "squirrel.h"
#include "SFML/Graphics.hpp"
#include "Entity.h"

namespace ng
{
enum class UseDirection
{
  Front,
  Back,
  Left,
  Right
};

class Actor;
class Animation;
class Room;

namespace ObjectStateConstants
{
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

class Object : public Entity
{
public:
  Object();
  ~Object();

  void setZOrder(int zorder);
  int getZOrder() const override;

  void setProp(bool prop);

  void setSpot(bool spot);

  void setTrigger(bool trigger);
  bool isTrigger() const;

  bool isTouchable() const override;

  void setUseDirection(UseDirection direction);
  UseDirection getUseDirection() const;

  void setHotspot(const sf::IntRect &hotspot);
  const sf::IntRect &getHotspot() const;
  sf::IntRect getRealHotspot() const;
  void showHotspot(bool show);
  bool isHotspotVisible() const;

  void setId(const std::wstring &id);
  const std::wstring &getId() const;

  void setIcon(const std::string &icon);
  std::string getIcon() const;

  int getDefaultVerb(HSQUIRRELVM vm) const;

  HSQOBJECT &getTable() override;

  std::vector<std::unique_ptr<Animation>> &getAnims();

  bool isVisible() const override;
  void setStateAnimIndex(int animIndex);
  void playAnim(int animIndex, bool loop);
  void playAnim(const std::string& anim, bool loop);
  int getState();
  void setAnimation(const std::string &name);
  std::optional<Animation>& getAnimation();

  Room *getRoom() override;
  const Room *getRoom() const override;
  void setRoom(Room *pRoom);

  void update(const sf::Time &elapsed) override;

  friend std::wostream &operator<<(std::wostream &os, const Object &obj);
  void drawHotspot(sf::RenderTarget &target, sf::RenderStates states) const;

  void addTrigger(const std::shared_ptr<Trigger>& trigger);
  void removeTrigger();
  Trigger* getTrigger();
  void enableTrigger(bool enabled);

  void dependentOn(Object* parentObject, int state);

  void setFps(int fps) override;

  Actor* getOwner() const;
  void setOwner(Actor* pActor);

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;
};
} // namespace ng
