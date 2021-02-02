#pragma once
#include <memory>
#include <string>
#include <optional>
#include <squirrel.h>
#include "Costume.hpp"
#include "engge/Entities/Entity.hpp"

namespace ng {
class Engine;
class Lip;
class Object;
class Room;

class Actor : public Entity {
public:
  explicit Actor(Engine &engine);
  ~Actor() override;

  [[nodiscard]] std::wstring getTranslatedName() const;

  [[nodiscard]] std::string getIcon() const;

  void useWalkboxes(bool use);
  [[nodiscard]] bool useWalkboxes() const;

  [[nodiscard]] int getZOrder() const override;

  void setCostume(const std::string &name, const std::string &sheet = "");
  [[nodiscard]] Costume &getCostume() const;
  Costume &getCostume();

  Room *getRoom() override;
  [[nodiscard]] const Room *getRoom() const override;
  void setRoom(Room *pRoom);

  void setFps(int fps) override;
  [[nodiscard]] int getFps() const;

  void setHotspot(const ngf::irect &hotspot);
  [[nodiscard]] ngf::irect getHotspot() const;
  void showHotspot(bool show);
  [[nodiscard]] bool isHotspotVisible() const;
  [[nodiscard]] bool contains(const glm::vec2 &pos) const;

  void update(const ngf::TimeSpan &time) override;

  void pickupObject(Object *pObject);
  void pickupReplacementObject(Object *pObject1, Object *pObject2);
  void giveTo(Object *pObject, Actor *pActor);
  void removeInventory(Object *pObject);
  void clearInventory();
  [[nodiscard]] const std::vector<Object *> &getObjects() const;
  void setInventoryOffset(int offset);
  [[nodiscard]] int getInventoryOffset() const;

  void setWalkSpeed(const glm::ivec2 &speed);
  [[nodiscard]] const glm::ivec2 &getWalkSpeed() const;

  std::vector<glm::vec2> walkTo(const glm::vec2 &destination, std::optional<Facing> facing = std::nullopt);
  void stopWalking();
  [[nodiscard]] bool isWalking() const;

  [[nodiscard]] bool isInventoryObject() const override;

  void setVolume(float volume);
  [[nodiscard]] std::optional<float> getVolume() const override;

  void drawForeground(ngf::RenderTarget &target, ngf::RenderStates states) const override;

  HSQOBJECT &getTable() override;
  [[nodiscard]] HSQOBJECT &getTable() const override;
  [[nodiscard]] float getScale() const override;

private:
  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const override;

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;
};
} // namespace ng
