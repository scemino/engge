#pragma once
#include <vector>
#include <engge/Graphics/SpriteSheet.hpp>
#include <squirrel.h>
#include <ngf/System/TimeSpan.h>
#include <ngf/Graphics/RenderTarget.h>
#include <ngf/Graphics/Color.h>
#include <engge/Scripting/ScriptObject.hpp>
#include <engge/Graphics/LightingShader.h>

namespace ngf {
class Walkbox;
class Graph;
}

namespace ng {
class Entity;
class Light;
class Object;
class RoomScaling;
class ResourceManager;
class TextObject;
class ThreadBase;

/// @brief Room effect used by roomEffect method
///
/// Only None, Ghost and BlackAndWhite are explicitly used.
/// The others can be used with flashbackEffect settings.
namespace RoomEffectConstants {
static const int EFFECT_NONE = 0;
static const int EFFECT_SEPIA = 1;
static const int EFFECT_EGA = 2;
static const int EFFECT_VHS = 3;
static const int EFFECT_GHOST = 4;
static const int EFFECT_BLACKANDWHITE = 5;
}

class Room : public ScriptObject {
public:
  static std::unique_ptr<Room> define(HSQOBJECT roomTable, const char *name = nullptr);

  explicit Room(HSQOBJECT roomTable);
  ~Room() override;

  void setName(const std::string &name);
  [[nodiscard]] std::string getName() const;

  void load(const char *name);
  std::vector<std::unique_ptr<Object>> &getObjects();
  [[nodiscard]] const std::vector<std::unique_ptr<Object>> &getObjects() const;
  [[nodiscard]] std::array<Light, LightingShader::MaxLights> &getLights();
  [[nodiscard]] int getNumberLights() const;
  LightingShader& getLightingShader();

  void update(const ngf::TimeSpan &elapsed);
  void draw(ngf::RenderTarget &target, const glm::vec2 &cameraPos) const;
  void drawForeground(ngf::RenderTarget &target, const glm::vec2 &cameraPos) const;

  void setWalkboxEnabled(const std::string &name, bool isEnabled);
  [[nodiscard]] const ngf::Walkbox *getWalkbox(const std::string &name) const;
  [[nodiscard]] std::vector<glm::vec2> calculatePath(glm::vec2 start, glm::vec2 end) const;
  std::vector<ngf::Walkbox> &getWalkboxes();
  std::vector<ngf::Walkbox> &getGraphWalkboxes();
  [[nodiscard]] const ngf::Graph *getGraph() const;

  Object &createObject(const std::string &sheet, const std::vector<std::string> &anims);
  Object &createObject(const std::vector<std::string> &anims);
  Object &createObject(const std::string &image);
  Object &createObject();
  TextObject &createTextObject(const std::string &fontName);
  void deleteObject(Object &textObject);
  [[nodiscard]] glm::ivec2 getRoomSize() const;
  [[nodiscard]] glm::ivec2 getScreenSize() const;
  [[nodiscard]] int32_t getFullscreen() const;
  [[nodiscard]] int32_t getScreenHeight() const;
  void setAsParallaxLayer(Entity *pEntity, int layer);
  void roomLayer(int layer, bool enabled);
  void setRoomScaling(const RoomScaling &scaling);
  [[nodiscard]] const RoomScaling &getRoomScaling() const;
  HSQOBJECT &getTable();

  [[nodiscard]] const SpriteSheet &getSpriteSheet() const;

  void setAmbientLight(ngf::Color color);
  [[nodiscard]] ngf::Color getAmbientLight() const;

  void removeEntity(Entity *pEntity);
  std::vector<RoomScaling> &getScalings();

  [[nodiscard]] float getRotation() const;
  void setRotation(float angle);

  Light *createLight(ngf::Color color, glm::ivec2 pos);
  void exit();

  void setEffect(int shader);
  [[nodiscard]] int getEffect() const;

  void setOverlayColor(ngf::Color color);
  [[nodiscard]] ngf::Color getOverlayColor() const;

  void setPseudoRoom(bool pseudoRoom);
  [[nodiscard]] bool isPseudoRoom() const;

private:
  struct Impl;
  std::unique_ptr<Impl> m_pImpl;
};
} // namespace ng
