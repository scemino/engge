#pragma once
#include <vector>
#include "Graphics/SpriteSheet.hpp"
#include "SFML/Graphics.hpp"
#include "squirrel.h"
#include "Scripting/ScriptObject.hpp"

namespace ng
{
class Entity;
class Graph;
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
  Room(TextureManager &textureManager);
  ~Room() override;

  void setName(const std::string& name);
  [[nodiscard]] std::string getName() const;

  void load(const char *name);
  std::vector<std::unique_ptr<Object>> &getObjects();
  std::vector<std::unique_ptr<Light>> &getLights();

  void update(const sf::Time &elapsed);
  void draw(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const;
  void drawForeground(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const;

  void setWalkboxEnabled(const std::string &name, bool isEnabled);
  [[nodiscard]] bool inWalkbox(const sf::Vector2f &pos) const;
  [[nodiscard]] std::vector<sf::Vector2f> calculatePath(sf::Vector2f start, sf::Vector2f end) const;
  std::vector<Walkbox>& getWalkboxes();
  std::vector<Walkbox>& getGraphWalkboxes();
  [[nodiscard]] const Graph* getGraph() const;

  Object &createObject(const std::string &sheet, const std::vector<std::string> &anims);
  Object &createObject(const std::vector<std::string> &anims);
  Object &createObject(const std::string &image);
  Object &createObject();
  TextObject &createTextObject(const std::string &fontName);
  void deleteObject(Object &textObject);
  [[nodiscard]] sf::Vector2i getRoomSize() const;
  [[nodiscard]] sf::Vector2i getScreenSize() const;
  [[nodiscard]] int32_t getFullscreen() const;
  [[nodiscard]] int32_t getScreenHeight() const;
  void setAsParallaxLayer(Entity *pEntity, int layer);
  void roomLayer(int layer, bool enabled);
  void setRoomScaling(const RoomScaling & scaling);
  [[nodiscard]] const RoomScaling &getRoomScaling() const;
  HSQOBJECT &getTable();

  const SpriteSheet& getSpriteSheet() const;

  void setAmbientLight(sf::Color color);
  [[nodiscard]] sf::Color getAmbientLight() const;

  void removeEntity(Entity *pEntity);
  std::vector<RoomScaling>& getScalings();

  [[nodiscard]] float getRotation() const;
  void setRotation(float angle);

  Light* createLight(sf::Color color, sf::Vector2i pos);
  void exit();

  void setEffect(int shader);
  [[nodiscard]] int getEffect() const;

  void setOverlayColor(sf::Color color);
  [[nodiscard]] sf::Color getOverlayColor() const;

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;
};
} // namespace ng
