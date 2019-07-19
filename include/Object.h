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

class Animation;
class Room;

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

  void setName(const std::wstring &name);
  const std::wstring &getName() const;

  void setId(const std::wstring &id);
  const std::wstring &getId() const;

  void setDefaultVerb(int verb);
  int getDefaultVerb() const;

  HSQOBJECT &getTable() override;

  std::vector<std::unique_ptr<Animation>> &getAnims();

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
private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;
};
} // namespace ng
