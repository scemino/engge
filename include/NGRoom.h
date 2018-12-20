#pragma once
#include <vector>
#include <nlohmann/json.hpp>
#include "NonCopyable.h"
#include "TextureManager.h"
#include "NGEngineSettings.h"
#include "NGObject.h"
#include "NGTextObject.h"
#include "Walkbox.h"
#include "RoomLayer.h"
#include "RoomScaling.h"

namespace ng
{
class NGRoom : public NonCopyable
{
public:
  NGRoom(TextureManager &textureManager, const NGEngineSettings &settings);
  ~NGRoom() = default;

  const std::string &getId() const { return _id; }

  void load(const char *name);
  std::vector<std::unique_ptr<NGObject>> &getObjects() { return _objects; }
  const std::string &getSheet() const { return _sheet; }

  void update(const sf::Time &elapsed);
  void draw(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const;

  void showDrawWalkboxes(bool show) { _showDrawWalkboxes = show; }
  bool areDrawWalkboxesVisible() const { return _showDrawWalkboxes; }
  const std::vector<std::unique_ptr<Walkbox>> &getWalkboxes() const { return _walkboxes; }
  std::vector<std::unique_ptr<Walkbox>> &getWalkboxes() { return _walkboxes; }
  NGObject &createObject(const std::string &sheet, const std::vector<std::string> &anims);
  NGObject &createObject(const std::vector<std::string> &anims);
  NGTextObject &createTextObject(const std::string &fontName);
  void deleteObject(NGObject &textObject);
  sf::Vector2i getRoomSize() const { return _roomSize; }
  void setAsParallaxLayer(NGEntity *pEntity, int layer);

private:
  void drawWalkboxes(sf::RenderWindow &window, sf::RenderStates states) const;

  void loadLayers(nlohmann::json jWimpy, nlohmann::json json);
  void loadObjects(nlohmann::json jWimpy, nlohmann::json json);
  void loadScalings(nlohmann::json jWimpy);
  void loadWalkboxes(nlohmann::json jWimpy);
  void loadBackgrounds(nlohmann::json jWimpy, nlohmann::json json);

private:
  TextureManager &_textureManager;
  const NGEngineSettings &_settings;
  std::vector<std::unique_ptr<NGObject>> _objects;
  std::vector<std::unique_ptr<Walkbox>> _walkboxes;
  std::vector<std::unique_ptr<RoomLayer>> _layers;
  std::vector<RoomScaling> _scalings;
  sf::Vector2i _roomSize;
  bool _showDrawWalkboxes;
  std::string _sheet;
  std::string _id;
  int _fullscreen;
};
} // namespace ng
