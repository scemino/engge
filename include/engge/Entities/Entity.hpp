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
#include "engge/Engine/Function.hpp"
#include "engge/Engine/Interpolations.hpp"
#include "engge/Scripting/ScriptObject.hpp"
#include "engge/Entities/Actor/DirectionConstants.hpp"

namespace ng {
enum class UseDirection {
  Front = DirectionConstants::FACE_FRONT,
  Back = DirectionConstants::FACE_BACK,
  Left = DirectionConstants::FACE_LEFT,
  Right = DirectionConstants::FACE_RIGHT,
};

struct Motor {
  bool isEnabled{false};
  std::unique_ptr<Function> function;
};

class Actor;
class Engine;
class Room;
class SoundDefinition;
class SoundTrigger;
class Trigger;
class Entity : public ScriptObject, public ngf::Drawable {
public:
  Entity();
  ~Entity() override;

  void setKey(const std::string &key);
  [[nodiscard]] const std::string &getKey() const;

  uint32_t getFlags() const;

  virtual void update(const ngf::TimeSpan &elapsed);
  virtual int getZOrder() const = 0;

  void setName(const std::string &name);
  std::string getName() const;

  void setVisible(bool isVisible);
  virtual bool isVisible() const;

  void setLit(bool isLit);
  bool isLit() const;

  void setTouchable(bool isTouchable);
  virtual bool isTouchable() const;

  void objectBumperCycle(bool enabled);
  bool objectBumperCycle() const;

  virtual bool isInventoryObject() const = 0;

  void setRenderOffset(const glm::ivec2 &offset);
  glm::ivec2 getRenderOffset() const;

  void setUseDirection(std::optional<UseDirection> direction);
  std::optional<UseDirection> getUseDirection() const;

  void setUsePosition(std::optional<glm::vec2> pos);
  void setPosition(const glm::vec2 &pos);

  glm::vec2 getPosition() const;
  glm::vec2 getRealPosition() const;
  std::optional<glm::vec2> getUsePosition() const;

  void setOffset(const glm::vec2 &offset);
  glm::vec2 getOffset() const;

  void setRotation(float angle);
  float getRotation() const;

  void setScale(float s);
  virtual float getScale() const;

  void setColor(const ngf::Color &color);
  const ngf::Color &getColor() const;

  void setTalkColor(ngf::Color color);
  ngf::Color getTalkColor() const;

  void setTalkOffset(const glm::ivec2 &offset);
  glm::ivec2 getTalkOffset() const;

  void setTrigger(int triggerNumber, Trigger *pTrigger);
  void removeTrigger(int triggerNumber);
  void trig(const std::string &name);

  virtual std::optional<float> getVolume() const { return std::nullopt; }
  virtual void drawForeground(ngf::RenderTarget &target, ngf::RenderStates states) const;

  virtual Room *getRoom() = 0;
  virtual const Room *getRoom() const = 0;
  virtual void setFps(int fps) = 0;

  virtual HSQOBJECT &getTable() = 0;
  virtual HSQOBJECT &getTable() const = 0;

  virtual bool hasParent() const { return false; }

  SoundTrigger *createSoundTrigger(Engine &engine, const std::vector<SoundDefinition *> &sounds);

  void alphaTo(float destination, ngf::TimeSpan time, InterpolationMethod method);
  void offsetTo(glm::vec2 destination, ngf::TimeSpan time, InterpolationMethod method);
  void moveTo(glm::vec2 destination, ngf::TimeSpan time, InterpolationMethod method);
  void rotateTo(float destination, ngf::TimeSpan time, InterpolationMethod method);
  void scaleTo(float destination, ngf::TimeSpan time, InterpolationMethod method);

  virtual void stopObjectMotors();

  void say(const std::string &text, bool mumble = false);
  void stopTalking();
  bool isTalking() const;

  int getDefaultVerb(int defaultVerbId) const;
  static Actor *getActor(const Entity *pEntity);

private:
  static void update(Motor &motor, const ngf::TimeSpan &elapsed);


protected:
  [[nodiscard]] ngf::Transform getTransform() const;
  ngf::Transform _transform;

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;
};
} // namespace ng
