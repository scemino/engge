#pragma once
#include <vector>
#include <nlohmann/json.hpp>
#include "NonCopyable.h"
#include "TextureManager.h"
#include "EngineSettings.h"
#include "Object.h"
#include "TextObject.h"
#include "Walkbox.h"
#include "RoomLayer.h"
#include "RoomScaling.h"

namespace ng
{
class Room : public NonCopyable
{
public:
  Room(TextureManager &textureManager, const EngineSettings &settings);
  ~Room() = default;

  const std::string &getId() const { return _id; }

  void load(const char *name);
  std::vector<std::unique_ptr<Object>> &getObjects() { return _objects; }
  const std::string &getSheet() const { return _sheet; }

  void update(const sf::Time &elapsed);
  void draw(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const;

  void showDrawWalkboxes(bool show) { _showDrawWalkboxes = show; }
  bool areDrawWalkboxesVisible() const { return _showDrawWalkboxes; }
  const std::vector<std::unique_ptr<Walkbox>> &getWalkboxes() const { return _walkboxes; }
  std::vector<std::unique_ptr<Walkbox>> &getWalkboxes() { return _walkboxes; }
  Object &createObject(const std::string &sheet, const std::vector<std::string> &anims);
  Object &createObject(const std::vector<std::string> &anims);
  TextObject &createTextObject(const std::string &fontName);
  void deleteObject(Object &textObject);
  sf::Vector2i getRoomSize() const { return _roomSize; }
  void setAsParallaxLayer(Entity *pEntity, int layer);
  const RoomScaling &getRoomScaling() const;
  void setTable(std::unique_ptr<HSQOBJECT> pTable) { _pTable = std::move(pTable); }
  HSQOBJECT* getTable() { return _pTable.get(); }

private:
  void drawWalkboxes(sf::RenderWindow &window, sf::RenderStates states) const;

  void loadLayers(nlohmann::json jWimpy, nlohmann::json json);
  void loadObjects(nlohmann::json jWimpy, nlohmann::json json);
  void loadScalings(nlohmann::json jWimpy);
  void loadWalkboxes(nlohmann::json jWimpy);
  void loadBackgrounds(nlohmann::json jWimpy, nlohmann::json json);

private:
  TextureManager &_textureManager;
  const EngineSettings &_settings;
  std::vector<std::unique_ptr<Object>> _objects;
  std::vector<std::unique_ptr<Walkbox>> _walkboxes;
  std::vector<std::unique_ptr<RoomLayer>> _layers;
  std::vector<RoomScaling> _scalings;
  sf::Vector2i _roomSize;
  bool _showDrawWalkboxes;
  std::string _sheet;
  std::string _id;
  int _fullscreen;
  std::unique_ptr<HSQOBJECT> _pTable;
};
} // namespace ng
