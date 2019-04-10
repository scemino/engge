#pragma once
#include <vector>
#include "squirrel.h"
#include "nlohmann/json.hpp"
#include "NonCopyable.h"

namespace ng
{
class EngineSettings;
class Entity;
class Object;
class RoomScaling;
class TextureManager;
class TextObject;
class Walkbox;

class Room : public NonCopyable
{
public:
  static int RoomType;

public:
  Room(TextureManager &textureManager, EngineSettings &settings);
  ~Room();

  void setId(const std::string &id);
  const std::string &getId() const;

  void load(const char *name);
  std::vector<std::unique_ptr<Object>> &getObjects();
  const std::string &getSheet() const;

  void update(const sf::Time &elapsed);
  void draw(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const;

  void showDrawWalkboxes(bool show);
  bool areDrawWalkboxesVisible() const;
  void setWalkboxEnabled(const std::string &name, bool isEnabled);
  bool inWalkbox(const sf::Vector2f &pos) const;
  std::vector<sf::Vector2i> calculatePath(const sf::Vector2i &start, const sf::Vector2i &end) const;

  Object &createObject(const std::string &sheet, const std::vector<std::string> &anims);
  Object &createObject(const std::vector<std::string> &anims);
  Object &createObject(const std::string &image);
  TextObject &createTextObject(const std::string &fontName);
  void deleteObject(Object &textObject);
  sf::Vector2i getRoomSize() const;
  void setAsParallaxLayer(Entity *pEntity, int layer);
  void roomLayer(int layer, bool enabled);
  const RoomScaling &getRoomScaling() const;
  HSQOBJECT &getTable();

  bool walkboxesVisible() const;
  void setAmbientLight(sf::Color color);
  sf::Color getAmbientLight() const;

  void removeEntity(Entity *pEntity);

private:
  void drawWalkboxes(sf::RenderWindow &window, sf::RenderStates states) const;

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;
};
} // namespace ng
