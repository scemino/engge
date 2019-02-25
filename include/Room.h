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
#include "Graph.h"
#include "PathFinder.h"
#include "SpriteSheet.h"

namespace ng
{
class Room : public NonCopyable
{
public:
  static int RoomType;

public:
  Room(TextureManager &textureManager, EngineSettings &settings);
  ~Room() = default;

  const std::string &getId() const { return _id; }

  void load(const char *name);
  std::vector<std::unique_ptr<Object>> &getObjects() { return _objects; }
  const std::string &getSheet() const { return _sheet; }

  void update(const sf::Time &elapsed);
  void draw(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const;

  void showDrawWalkboxes(bool show) { _showDrawWalkboxes = show; }
  bool areDrawWalkboxesVisible() const { return _showDrawWalkboxes; }
  void setWalkboxEnabled(const std::string &name, bool isEnabled);
  bool inWalkbox(const sf::Vector2f &pos) const;
  std::vector<sf::Vector2i> calculatePath(const sf::Vector2i &start, const sf::Vector2i &end) const;
  const std::vector<Walkbox> &getWalkboxes() const { return _walkboxes; }

  Object &createObject(const std::string &sheet, const std::vector<std::string> &anims);
  Object &createObject(const std::vector<std::string> &anims);
  Object &createObject(const std::string &image);
  TextObject &createTextObject(const std::string &fontName);
  void deleteObject(Object &textObject);
  sf::Vector2i getRoomSize() const { return _roomSize; }
  void setAsParallaxLayer(Entity *pEntity, int layer);
  const RoomScaling &getRoomScaling() const;
  void setTable(std::unique_ptr<HSQOBJECT> pTable) { _pTable = std::move(pTable); }
  HSQOBJECT *getTable() { return _pTable.get(); }

  bool walkboxesVisible() const { return _showDrawWalkboxes; }
  void setAmbientLight(sf::Color color) { _ambientColor = color; }
  sf::Color getAmbientLight() const { return _ambientColor; }

  void removeEntity(Entity *pEntity);

private:
  void drawWalkboxes(sf::RenderWindow &window, sf::RenderStates states) const;

  void loadLayers(GGPackValue& jWimpy);
  void loadObjects(GGPackValue& jWimpy);
  void loadScalings(GGPackValue& jWimpy);
  void loadWalkboxes(GGPackValue& jWimpy);
  void loadBackgrounds(GGPackValue& jWimpy);
  void updateGraph();

private:
  TextureManager &_textureManager;
  EngineSettings &_settings;
  std::vector<std::unique_ptr<Object>> _objects;
  std::vector<Walkbox> _walkboxes;
  std::vector<std::unique_ptr<RoomLayer>> _layers;
  std::vector<RoomScaling> _scalings;
  sf::Vector2i _roomSize;
  bool _showDrawWalkboxes;
  std::string _sheet;
  std::string _id;
  int _fullscreen;
  std::unique_ptr<HSQOBJECT> _pTable;
  std::shared_ptr<Path> _path;
  std::shared_ptr<PathFinder> _pf;
  std::vector<Walkbox> _graphWalkboxes;
  sf::Color _ambientColor;
  SpriteSheet _spriteSheet;
};
} // namespace ng
