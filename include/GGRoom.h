#pragma once
#include <vector>
#include "NonCopyable.h"
#include "TextureManager.h"
#include "GGEngineSettings.h"
#include "GGObject.h"
#include "Walkbox.h"
#include "RoomLayer.h"
#include "RoomScaling.h"

namespace gg
{
class GGRoom : public NonCopyable
{
public:
  GGRoom(TextureManager &textureManager, const GGEngineSettings &settings);
  ~GGRoom();

  void load(const char *name);
  std::vector<std::unique_ptr<GGObject>> &getObjects() { return _objects; }
  const std::string &getSheet() const { return _sheet; }

  void update(const sf::Time &elapsed);
  void draw(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const;

  void showDrawWalkboxes(bool show) { _showDrawWalkboxes = show; }
  bool areDrawWalkboxesVisible() const { return _showDrawWalkboxes; }
  void showObjects(bool show) { _showObjects = show; }
  bool areObjectsVisible() const { return _showObjects; }
  GGObject &createObject(const std::vector<std::string> &anims);
  void showLayers(bool show) { _showLayers = show; }
  bool areLayersVisible() const { return _showLayers; }

private:
  void drawBackgrounds(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const;
  void drawBackgroundLayers(sf::RenderWindow &window, const sf::Vector2f &cameraX) const;
  void drawForegroundLayers(sf::RenderWindow &window, const sf::Vector2f &cameraX) const;
  void drawObjects(sf::RenderWindow &window) const;
  void drawWalkboxes(sf::RenderWindow &window) const;

private:
  TextureManager &_textureManager;
  const GGEngineSettings &_settings;
  std::vector<std::unique_ptr<GGObject>> _objects;
  std::vector<sf::Sprite> _backgrounds;
  std::vector<Walkbox> _walkboxes;
  std::vector<RoomLayer> _layers;
  std::vector<RoomScaling> _scalings;
  sf::Vector2i _roomSize;
  bool _showDrawWalkboxes;
  bool _showObjects;
  bool _showLayers;
  std::string _sheet;
};
} // namespace gg
