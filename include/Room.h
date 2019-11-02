#pragma once
#include <vector>
#include "SFML/Graphics.hpp"
#include "squirrel.h"
#include "ScriptObject.h"

namespace ng
{
class EngineSettings;
class Entity;
class Light;
class Object;
class RoomScaling;
class TextureManager;
class TextObject;
class ThreadBase;
class Walkbox;

namespace RoomEffectConstants
{
static const int EFFECT_NONE = 0;
static const int EFFECT_SEPIA = 1;
static const int EFFECT_EGA = 2;
static const int EFFECT_VHS = 3;
static const int EFFECT_GHOST = 4;
static const int EFFECT_BLACKANDWHITE = 5;
}

class Room : public ScriptObject
{
public:
  Room(TextureManager &textureManager, EngineSettings &settings);
  ~Room() override;

  void setName(const std::string& name);
  std::string getName() const;

  void load(const char *name);
  std::vector<std::unique_ptr<Object>> &getObjects();
  std::vector<std::unique_ptr<Light>> &getLights();
  const std::string &getSheet() const;

  void update(const sf::Time &elapsed);
  void draw(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const;
  void drawForeground(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const;

  void showDrawWalkboxes(bool show);
  bool areDrawWalkboxesVisible() const;
  void setWalkboxEnabled(const std::string &name, bool isEnabled);
  bool inWalkbox(const sf::Vector2f &pos) const;
  std::vector<sf::Vector2i> calculatePath(const sf::Vector2i &start, const sf::Vector2i &end) const;
  std::vector<Walkbox>& getWalkboxes();

  Object &createObject(const std::string &sheet, const std::vector<std::string> &anims);
  Object &createObject(const std::vector<std::string> &anims);
  Object &createObject(const std::string &image);
  TextObject &createTextObject(const std::string &fontName);
  void deleteObject(Object &textObject);
  sf::Vector2i getRoomSize() const;
  int32_t getFullscreen() const;
  int32_t getScreenHeight() const;
  void setAsParallaxLayer(Entity *pEntity, int layer);
  void roomLayer(int layer, bool enabled);
  void setRoomScaling(const RoomScaling & scaling);
  const RoomScaling &getRoomScaling() const;
  HSQOBJECT &getTable();

  bool walkboxesVisible() const;
  void setAmbientLight(sf::Color color);
  sf::Color getAmbientLight() const;

  void removeEntity(Entity *pEntity);
  std::vector<RoomScaling>& getScalings();

  float getRotation() const;
  void setRotation(float angle);

  Light* createLight(sf::Color color, sf::Vector2i pos);
  void addThread(std::unique_ptr<ThreadBase> thread);
  bool isThreadAlive(HSQUIRRELVM thread) const;
  void stopThread(HSQUIRRELVM thread);
  void exit();

  void setEffect(int shader);
  int getEffect() const;

  void setOverlayColor(sf::Color color);
  sf::Color getOverlayColor() const;

private:
  void drawWalkboxes(sf::RenderWindow &window, sf::RenderStates states) const;

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;
};
} // namespace ng
