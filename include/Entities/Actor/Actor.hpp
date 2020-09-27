#pragma once
#include <memory>
#include <string>
#include <optional>
#include "squirrel.h"
#include "SFML/Graphics.hpp"
#include "Costume.hpp"
#include "Entities/Entity.hpp"

namespace ng {
class Engine;
class Lip;
class Object;
class Room;

class Actor : public Entity {
public:
  explicit Actor(Engine &engine);
  ~Actor() override;

  std::wstring getTranslatedName() const;

  std::string getIcon() const;

  void useWalkboxes(bool use);
  bool useWalkboxes() const;

  int getZOrder() const override;

  void setCostume(const std::string &name, const std::string &sheet = "");
  Costume &getCostume() const;
  Costume &getCostume();

  Room *getRoom() override;
  const Room *getRoom() const override;
  void setRoom(Room *pRoom);

  void setFps(int fps) override;
  int getFps() const;

  void setHotspot(const sf::IntRect &hotspot);
  sf::IntRect getHotspot() const;
  void showHotspot(bool show);
  bool isHotspotVisible() const;
  bool contains(const sf::Vector2f &pos) const;

  void update(const sf::Time &time) override;

  void pickupObject(Object *pObject);
  void pickupReplacementObject(Object *pObject1, Object *pObject2);
  void giveTo(Object *pObject, Actor *pActor);
  void removeInventory(Object *pObject);
  void clearInventory();
  const std::vector<Object *> &getObjects() const;
  void setInventoryOffset(int offset);
  int getInventoryOffset() const;

  void setWalkSpeed(const sf::Vector2i &speed);
  const sf::Vector2i &getWalkSpeed() const;

  std::vector<sf::Vector2f> walkTo(const sf::Vector2f &destination, std::optional<Facing> facing = std::nullopt);
  void stopWalking();
  bool isWalking() const;

  bool isInventoryObject() const override;

  void setVolume(float volume);
  std::optional<float> getVolume() const override;

  void trigSound(const std::string &name) override;

  void drawForeground(sf::RenderTarget &target, sf::RenderStates states) const override;

  HSQOBJECT &getTable() override;
  HSQOBJECT &getTable() const override;
  float getScale() const override;

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;
};
} // namespace ng
