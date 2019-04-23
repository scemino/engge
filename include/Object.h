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
  bool getProp() const;

  void setSpot(bool spot);
  bool getSpot() const;

  void setTrigger(bool trigger);
  bool getTrigger() const;

  void setTouchable(bool isTouchable);
  bool isTouchable() const;

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

  HSQOBJECT &getTable();

  std::vector<std::unique_ptr<Animation>> &getAnims();

  void setStateAnimIndex(int animIndex);
  void playAnim(int animIndex, bool loop);
  void playAnim(const std::string& anim, bool loop);
  int getStateAnimIndex();
  int getState();
  void setAnimation(const std::string &name);
  std::optional<Animation>& getAnimation();

  void setDefaultPosition(const sf::Vector2f &pos);
  sf::Vector2f getDefaultPosition() const override;
  void move(const sf::Vector2f &offset) override;

  void setRotation(float angle);
  float getRotation() const;

  void setColor(const sf::Color &color);
  const sf::Color &getColor() const;

  void setVisible(bool isVisible);
  bool isVisible() const;
  void setScale(float s);

  Room *getRoom() override;
  const Room *getRoom() const;
  void setRoom(Room *pRoom);

  void update(const sf::Time &elapsed) override;

  friend std::wostream &operator<<(std::wostream &os, const Object &obj);
  void setHotspotVisible(bool isVisible);
  void drawHotspot(sf::RenderTarget &target, sf::RenderStates states) const;

  void addTrigger(const std::shared_ptr<Trigger>& trigger);
  void removeTrigger();
  Trigger* getTrigger();
  void enableTrigger(bool enabled);

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;
};
} // namespace ng
