#pragma once
#include <memory>
#include <string>
#include <optional>
#include <squirrel.h>
#include <engge/Entities/Entity.hpp>
#include <engge/Entities/Facing.hpp>

namespace ng {
class Costume;
class Engine;
class Lip;
class Object;
class PathDrawable;
class Room;

class Actor final : public Entity {
public:
  explicit Actor(Engine &engine);
  ~Actor() final;

  [[nodiscard]] std::wstring getTranslatedName() const;

  [[nodiscard]] std::string getIcon() const;

  void useWalkboxes(bool use);
  [[nodiscard]] bool useWalkboxes() const;

  [[nodiscard]] int getZOrder() const final;

  void setCostume(const std::string &name, const std::string &sheet = "");
  [[nodiscard]] Costume &getCostume() const;
  Costume &getCostume();

  Room *getRoom() final;
  [[nodiscard]] const Room *getRoom() const final;
  void setRoom(Room *pRoom);

  void setFps(int fps) final;
  [[nodiscard]] int getFps() const;

  void setHotspot(const ngf::irect &hotspot);
  [[nodiscard]] ngf::irect getHotspot() const;
  void showHotspot(bool show);
  [[nodiscard]] bool isHotspotVisible() const;
  [[nodiscard]] bool contains(const glm::vec2 &pos) const;

  void update(const ngf::TimeSpan &time) final;

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
  std::unique_ptr<PathDrawable> getPath();

  [[nodiscard]] bool isInventoryObject() const final;

  HSQOBJECT &getTable() final;
  [[nodiscard]] HSQOBJECT &getTable() const final;
  [[nodiscard]] float getScale() const final;

private:
  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const final;

private:
  struct Impl;
  std::unique_ptr<Impl> m_pImpl;
};
} // namespace ng
