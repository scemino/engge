#pragma once
#include <memory>
#include <optional>
#include <squirrel.h>
#include <glm/vec2.hpp>
#include <ngf/Graphics/Color.h>
#include <ngf/Graphics/Drawable.h>
#include <ngf/Graphics/RenderStates.h>
#include <ngf/Graphics/RenderTarget.h>
#include <ngf/Math/Transform.h>
#include <engge/Engine/Interpolations.hpp>
#include <engge/Scripting/ScriptObject.hpp>
#include <engge/Entities/UseDirection.hpp>

namespace ng {
class Actor;
class Engine;
class Room;
class SoundDefinition;
class SoundTrigger;
class Trigger;
struct Motor;

class Entity : public ScriptObject, public ngf::Drawable {
public:
  Entity();
  ~Entity() override;

  void setKey(const std::string &key);
  [[nodiscard]] const std::string &getKey() const;

  [[nodiscard]] uint32_t getFlags() const;

  virtual void update(const ngf::TimeSpan &elapsed);
  [[nodiscard]] virtual int getZOrder() const = 0;

  void setName(const std::string &name);
  [[nodiscard]] std::string getName() const;

  void setVisible(bool isVisible);
  [[nodiscard]] bool isVisible() const;

  void setLit(bool isLit);
  [[nodiscard]] bool isLit() const;

  void setTouchable(bool isTouchable);
  [[nodiscard]] virtual bool isTouchable() const;

  void objectBumperCycle(bool enabled);
  [[nodiscard]] bool objectBumperCycle() const;

  [[nodiscard]] virtual bool isInventoryObject() const = 0;

  void setRenderOffset(const glm::ivec2 &offset);
  [[nodiscard]] glm::ivec2 getRenderOffset() const;

  void setUseDirection(std::optional<UseDirection> direction);
  [[nodiscard]] std::optional<UseDirection> getUseDirection() const;

  void setUsePosition(std::optional<glm::vec2> pos);
  [[nodiscard]] std::optional<glm::vec2> getUsePosition() const;

  void setPosition(const glm::vec2 &pos);
  [[nodiscard]] glm::vec2 getPosition() const;
  [[nodiscard]] glm::vec2 getRealPosition() const;
  [[nodiscard]] ngf::Transform getTransform() const;

  void setOffset(const glm::vec2 &offset);
  [[nodiscard]] glm::vec2 getOffset() const;

  void setRotation(float angle);
  [[nodiscard]] float getRotation() const;

  void setScale(float s);
  [[nodiscard]] virtual float getScale() const;

  void setColor(const ngf::Color &color);
  [[nodiscard]] ngf::Color getColor() const;

  void setTalkColor(ngf::Color color);
  [[nodiscard]] ngf::Color getTalkColor() const;

  void setTalkOffset(const glm::ivec2 &offset);
  [[nodiscard]] glm::ivec2 getTalkOffset() const;

  void setTrigger(int triggerNumber, Trigger *pTrigger);
  void removeTrigger(int triggerNumber);
  void trig(const std::string &name);

  void setVolume(float volume);
  [[nodiscard]] float getVolume() const;

  virtual void drawForeground(ngf::RenderTarget &target, ngf::RenderStates states) const;

  virtual Room *getRoom() = 0;
  [[nodiscard]] virtual const Room *getRoom() const = 0;
  virtual void setFps(int fps) = 0;

  virtual HSQOBJECT &getTable() = 0;
  [[nodiscard]] virtual HSQOBJECT &getTable() const = 0;

  [[nodiscard]] bool hasParent() const;
  void setParent(Entity *pParent);
  Entity *getParent();
  [[nodiscard]] const Entity *getParent() const;

  SoundTrigger *createSoundTrigger(Engine &engine, const std::vector<std::shared_ptr<SoundDefinition>> &sounds);

  void alphaTo(float destination, ngf::TimeSpan time, InterpolationMethod method);
  void offsetTo(glm::vec2 destination, ngf::TimeSpan time, InterpolationMethod method);
  void moveTo(glm::vec2 destination, ngf::TimeSpan time, InterpolationMethod method);
  void rotateTo(float destination, ngf::TimeSpan time, InterpolationMethod method);
  void scaleTo(float destination, ngf::TimeSpan time, InterpolationMethod method);
  void shake(float amount);
  void jiggle(float amount);

  void stopObjectMotors();

  void say(const std::string &text, bool mumble = false);
  void stopTalking();
  [[nodiscard]] bool isTalking() const;

  [[nodiscard]] int getDefaultVerb(int defaultVerbId) const;
  static Actor *getActor(const Entity *pEntity);

private:
  static void update(Motor &motor, const ngf::TimeSpan &elapsed);

protected:
  [[nodiscard]] std::vector<Entity *> getChildren() const;

private:
  struct Impl;
  std::unique_ptr<Impl> m_pImpl;
};
} // namespace ng
