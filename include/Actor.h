#pragma once
#include <memory>
#include <string>
#include "squirrel.h"
#include "SFML/Graphics.hpp"
#include "Costume.h"
#include "Entity.h"

namespace ng
{
class Engine;
class InventoryObject;
class Lip;
class Room;

class Actor : public Entity
{
public:
  explicit Actor(Engine &engine);
  virtual ~Actor() override;

  void setName(const std::string &name);
  const std::string &getName() const;
  void setIcon(const std::string &icon);
  const std::string &getIcon() const;

  void useWalkboxes(bool use);

  int getZOrder() const override;

  void setCostume(const std::string &name, const std::string &sheet = "");
  Costume &getCostume();

  void setTalkColor(sf::Color color);
  void setTalkOffset(const sf::Vector2i &offset);
  void say(int id);
  void stopTalking();
  bool isTalking() const;
  bool isTalkingIdDone(int id) const;

  void setColor(sf::Color color);
  sf::Color getColor();

  void move(const sf::Vector2f &offset) override;
  void setRenderOffset(const sf::Vector2i &offset);

  Room *getRoom() override;
  const Room *getRoom() const;
  void setRoom(Room *pRoom);

  void setFps(int fps) override;

  void setHotspot(const sf::IntRect &hotspot);

  void update(const sf::Time &time) override;

  void pickupObject(std::unique_ptr<InventoryObject> pObject);
  const std::vector<std::unique_ptr<InventoryObject>> &getObjects() const;

  void setWalkSpeed(const sf::Vector2i &speed);
  const sf::Vector2i &getWalkSpeed() const;
  
  void walkTo(const sf::Vector2f &destination, std::optional<Facing> facing = std::nullopt);
  void stopWalking();
  bool isWalking() const;

  void setVolume(float volume);

  void trigSound(const std::string &name) override;

  void drawForeground(sf::RenderTarget &target, sf::RenderStates states) const override;

  HSQOBJECT &getTable();

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;
};
} // namespace ng
