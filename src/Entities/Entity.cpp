#include <memory>
#include <optional>
#include <utility>
#include <glm/vec2.hpp>
#include <engge/Entities/Entity.hpp>
#include <engge/Scripting/ScriptEngine.hpp>
#include <engge/Audio/SoundTrigger.hpp>
#include <engge/Engine/Camera.hpp>
#include <engge/Engine/Trigger.hpp>
#include "JiggleFunction.hpp"
#include "ShakeFunction.hpp"
#include "TalkingState.hpp"

namespace ng {
struct Motor {
  bool isEnabled{false};
  std::unique_ptr<Function> function;
};

struct Entity::Impl {
  std::string m_key;
  Engine &m_engine;
  std::map<int, Trigger *> m_triggers;
  std::vector<std::unique_ptr<SoundTrigger>> m_soundTriggers;
  std::optional<glm::vec2> m_usePos;
  std::optional<UseDirection> m_useDir;
  glm::vec2 m_offset{0, 0};
  glm::vec2 m_shakeOffset{0, 0};
  float m_jiggleOffset{0.f};
  bool m_isLit{false};
  glm::ivec2 m_renderOffset{0, 0};
  Motor m_offsetTo, m_scaleTo, m_rotateTo, m_moveTo, m_alphaTo, m_shake, m_jiggle;
  bool m_objectBumperCycle{true};
  ngf::Color m_talkColor;
  glm::ivec2 m_talkOffset{0, 90};
  TalkingState m_talkingState;
  ngf::Transform m_transform;
  Entity *m_pParent{nullptr};
  std::vector<Entity *> m_children;

  Impl() : m_engine(ng::Locator<ng::Engine>::get()) {
    m_talkingState.setEngine(&m_engine);
  }

  static std::optional<int> getDefaultVerb(const Entity *pEntity) {
    if (!pEntity)
      return std::nullopt;

    const char *dialog = nullptr;
    if (ScriptEngine::rawGet(pEntity, "dialog", dialog))
      return std::make_optional(VerbConstants::VERB_TALKTO);

    int value = 0;
    if (ScriptEngine::rawGet(pEntity, "defaultVerb", value))
      return std::make_optional(value);
    return std::nullopt;
  }
};

Entity::Entity() : m_pImpl(std::make_unique<Entity::Impl>()) {
}

Entity::~Entity() = default;

void Entity::objectBumperCycle(bool enabled) { m_pImpl->m_objectBumperCycle = enabled; }

bool Entity::objectBumperCycle() const { return m_pImpl->m_objectBumperCycle; }

void Entity::update(const ngf::TimeSpan &elapsed) {
  m_pImpl->m_talkingState.update(elapsed);
  update(m_pImpl->m_offsetTo, elapsed);
  update(m_pImpl->m_scaleTo, elapsed);
  update(m_pImpl->m_rotateTo, elapsed);
  update(m_pImpl->m_moveTo, elapsed);
  update(m_pImpl->m_alphaTo, elapsed);
  update(m_pImpl->m_shake, elapsed);
  update(m_pImpl->m_jiggle, elapsed);
}

void Entity::update(Motor &motor, const ngf::TimeSpan &elapsed) {
  if (motor.isEnabled) {
    (*motor.function)(elapsed);
    if (motor.isEnabled && motor.function->isElapsed()) {
      motor.isEnabled = false;
    }
  }
}

void Entity::setLit(bool isLit) {
  m_pImpl->m_isLit = isLit;
}

bool Entity::isLit() const {
  return m_pImpl->m_isLit;
}

void Entity::setVisible(bool isVisible) {
  ScriptEngine::set(getTable(), "_hidden", !isVisible);
}

bool Entity::isVisible() const {
  auto hidden = false;
  ScriptEngine::rawGet(getTable(), "_hidden", hidden);
  return !hidden;
}

void Entity::setUsePosition(std::optional<glm::vec2> pos) {
  m_pImpl->m_usePos = std::move(pos);
}

void Entity::setUseDirection(std::optional<UseDirection> direction) {
  m_pImpl->m_useDir = direction;
}

std::optional<UseDirection> Entity::getUseDirection() const {
  return m_pImpl->m_useDir;
}

void Entity::setPosition(const glm::vec2 &pos) {
  m_pImpl->m_transform.setPosition(pos);
  m_pImpl->m_moveTo.isEnabled = false;
}

glm::vec2 Entity::getPosition() const {
  return m_pImpl->m_transform.getPosition();
}

glm::vec2 Entity::getRealPosition() const {
  return m_pImpl->m_transform.getPosition() + m_pImpl->m_offset;
}

void Entity::setOffset(const glm::vec2 &offset) {
  m_pImpl->m_offset = offset;
  m_pImpl->m_offsetTo.isEnabled = false;
}

glm::vec2 Entity::getOffset() const {
  return m_pImpl->m_offset + m_pImpl->m_shakeOffset;
}

void Entity::setRotation(float angle) {
  m_pImpl->m_transform.setRotation(angle);
  m_pImpl->m_rotateTo.isEnabled = false;
}

float Entity::getRotation() const {
  // rotation is in degree between [0, 360]
  float angle = m_pImpl->m_transform.getRotation();
  // convert it to [-180, 180]
  if (angle > 180)
    angle -= 360;
  return angle;
}

void Entity::setColor(const ngf::Color &color) {
  ScriptEngine::set(getTable(), "_color", toInteger(color));
  m_pImpl->m_alphaTo.isEnabled = false;
}

ngf::Color Entity::getColor() const {
  auto color = toInteger(ngf::Colors::White);
  ScriptEngine::rawGet(getTable(), "_color", color);
  return fromRgba(color);
}

void Entity::setScale(float s) {
  m_pImpl->m_transform.setScale({s, s});
  m_pImpl->m_scaleTo.isEnabled = false;
}

float Entity::getScale() const {
  return m_pImpl->m_transform.getScale().x;
}

ngf::Transform Entity::getTransform() const {
  auto transform = m_pImpl->m_transform;
  transform.move(getOffset());
  transform.rotate(m_pImpl->m_jiggleOffset);
  return transform;
}

std::optional<glm::vec2> Entity::getUsePosition() const {
  return m_pImpl->m_usePos;
}

void Entity::setTrigger(int triggerNumber, Trigger *pTrigger) {
  m_pImpl->m_triggers[triggerNumber] = pTrigger;
}

void Entity::removeTrigger(int triggerNumber) {
  m_pImpl->m_triggers.erase(triggerNumber);
}

void Entity::trig(const std::string &name) {
  char *end;
  auto id = std::strtol(name.data() + 1, &end, 10);
  if (end == name.data() + 1) {
    // trig sound
    auto soundId = EntityManager::getSoundDefinition(ScriptEngine::getVm(), name.data() + 1);
    if (!soundId)
      return;
    m_pImpl->m_engine.getSoundManager().playSound(soundId);
  } else {
    // find a corresponding trigger
    auto it = m_pImpl->m_triggers.find(id);
    if (it != m_pImpl->m_triggers.end()) {
      it->second->trig();
    }
  }
}

void Entity::drawForeground(ngf::RenderTarget &target, ngf::RenderStates s) const {
  if (!m_pImpl->m_talkingState.isTalking())
    return;

  m_pImpl->m_talkingState.draw(target, s);
}

SoundTrigger *Entity::createSoundTrigger(Engine &engine, const std::vector<std::shared_ptr<SoundDefinition>> &sounds) {
  auto trigger = std::make_unique<SoundTrigger>(engine, sounds, this->getId());
  SoundTrigger *pTrigger = trigger.get();
  m_pImpl->m_soundTriggers.push_back(std::move(trigger));
  return pTrigger;
}

void Entity::setKey(const std::string &key) { m_pImpl->m_key = key; }

const std::string &Entity::getKey() const { return m_pImpl->m_key; }

uint32_t Entity::getFlags() const {
  int flags = 0;
  ScriptEngine::rawGet(this, "flags", flags);
  return (uint32_t) flags;
}

void Entity::setTouchable(bool isTouchable) {
  ScriptEngine::set(getTable(), "_touchable", isTouchable);
}

bool Entity::isTouchable() const {
  if (!isVisible())
    return false;
  int touchable = 1;
  if (!ScriptEngine::rawGet(getTable(), "_touchable", touchable)) {
    ScriptEngine::rawGet(getTable(), "initTouchable", touchable);
  }
  return touchable != 0;
}

void Entity::setRenderOffset(const glm::ivec2 &offset) {
  m_pImpl->m_renderOffset = offset;
}

glm::ivec2 Entity::getRenderOffset() const {
  return m_pImpl->m_renderOffset;
}

void Entity::shake(float amount) {
  auto setShake = [this](const auto &offset) { m_pImpl->m_shakeOffset = offset; };
  auto shake = std::make_unique<ShakeFunction>(setShake, amount);
  m_pImpl->m_shake.function = std::move(shake);
  m_pImpl->m_shake.isEnabled = true;
}

void Entity::jiggle(float amount) {
  auto setJiggle = [this](const auto &offset) { m_pImpl->m_jiggleOffset = offset; };
  auto jiggle = std::make_unique<JiggleFunction>(setJiggle, amount);
  m_pImpl->m_jiggle.function = std::move(jiggle);
  m_pImpl->m_jiggle.isEnabled = true;
}

void Entity::alphaTo(float destination, ngf::TimeSpan time, InterpolationMethod method) {
  auto getAlpha = [this] { return getColor().a; };
  auto setAlpha = [this](const float &a) {
    auto color = getColor();
    color.a = a;
    ScriptEngine::set(getTable(), "_color", toInteger(color));
  };
  auto alphaTo = std::make_unique<ChangeProperty<float>>(getAlpha, setAlpha, destination, time, method);
  m_pImpl->m_alphaTo.function = std::move(alphaTo);
  m_pImpl->m_alphaTo.isEnabled = true;
}

void Entity::offsetTo(glm::vec2 destination, ngf::TimeSpan time, InterpolationMethod method) {
  auto get = [this] { return m_pImpl->m_offset; };
  auto set = [this](const glm::vec2 &value) { m_pImpl->m_offset = value; };
  auto offsetTo = std::make_unique<ChangeProperty<glm::vec2>>(get, set, destination, time, method);
  m_pImpl->m_offsetTo.function = std::move(offsetTo);
  m_pImpl->m_offsetTo.isEnabled = true;
}

void Entity::moveTo(glm::vec2 destination, ngf::TimeSpan time, InterpolationMethod method) {
  auto get = [this] { return m_pImpl->m_transform.getPosition(); };
  auto set = [this](const glm::vec2 &value) { m_pImpl->m_transform.setPosition(value); };
  auto moveTo = std::make_unique<ChangeProperty<glm::vec2>>(get, set, destination, time, method);
  m_pImpl->m_moveTo.function = std::move(moveTo);
  m_pImpl->m_moveTo.isEnabled = true;
}

void Entity::rotateTo(float destination, ngf::TimeSpan time, InterpolationMethod method) {
  auto get = [this] { return m_pImpl->m_transform.getRotation(); };
  auto set = [this](const float &value) { m_pImpl->m_transform.setRotation(value); };
  auto rotateTo =
      std::make_unique<ChangeProperty<float>>(get, set, destination, time, method);
  m_pImpl->m_rotateTo.function = std::move(rotateTo);
  m_pImpl->m_rotateTo.isEnabled = true;
}

void Entity::scaleTo(float destination, ngf::TimeSpan time, InterpolationMethod method) {
  auto get = [this] { return m_pImpl->m_transform.getScale().x; };
  auto set = [this](const float &s) { m_pImpl->m_transform.setScale({s, s}); };
  auto scalteTo = std::make_unique<ChangeProperty<float>>(get, set, destination, time, method);
  m_pImpl->m_scaleTo.function = std::move(scalteTo);
  m_pImpl->m_scaleTo.isEnabled = true;
}

void Entity::setName(const std::string &name) {
  ScriptEngine::set(getTable(), "name", name.c_str());
}
std::string Entity::getName() const {
  const char *name = nullptr;
  ScriptEngine::get(getTable(), "name", name);
  if (!name)
    return std::string();
  return name;
}

void Entity::stopObjectMotors() {
  m_pImpl->m_offsetTo.isEnabled = false;
  m_pImpl->m_scaleTo.isEnabled = false;
  m_pImpl->m_rotateTo.isEnabled = false;
  m_pImpl->m_moveTo.isEnabled = false;
  m_pImpl->m_alphaTo.isEnabled = false;
  m_pImpl->m_shake.isEnabled = false;
  m_pImpl->m_jiggle.isEnabled = false;
  for (auto &&child : m_pImpl->m_children) {
    child->stopObjectMotors();
  }
}

void Entity::setTalkColor(ngf::Color color) { m_pImpl->m_talkColor = color; }

ngf::Color Entity::getTalkColor() const { return m_pImpl->m_talkColor; }

void Entity::setTalkOffset(const glm::ivec2 &offset) { m_pImpl->m_talkOffset = offset; }

glm::ivec2 Entity::getTalkOffset() const { return m_pImpl->m_talkOffset; }

void Entity::say(const std::string &text, bool mumble) {
  m_pImpl->m_talkingState.loadLip(text, this, mumble);
  glm::vec2 pos;
  auto screenSize = m_pImpl->m_engine.getRoom()->getScreenSize();
  if (getRoom() == m_pImpl->m_engine.getRoom()) {
    auto at = m_pImpl->m_engine.getCamera().getRect().getTopLeft();
    pos = getPosition();
    pos = {pos.x - at.x + m_pImpl->m_talkOffset.x + m_pImpl->m_renderOffset.x,
           screenSize.y - pos.y - at.y - m_pImpl->m_talkOffset.y - m_pImpl->m_renderOffset.y};
  } else {
    // TODO: the position in this case is wrong, don't know what to do yet
    pos = (glm::vec2) m_pImpl->m_talkOffset;
  }
  pos = toDefaultView((glm::ivec2) pos, m_pImpl->m_engine.getRoom()->getScreenSize());
  m_pImpl->m_talkingState.setPosition(pos);
}

void Entity::stopTalking() { m_pImpl->m_talkingState.stop(); }

bool Entity::isTalking() const { return m_pImpl->m_talkingState.isTalking(); }

int Entity::getDefaultVerb(int defaultVerbId) const {
  auto result = m_pImpl->getDefaultVerb(this);
  if (result.has_value())
    return result.value();

  result = m_pImpl->getDefaultVerb(getActor(this));
  return result.value_or(defaultVerbId);
}

Actor *Entity::getActor(const Entity *pEntity) {
  // if an actor has the same name then get its flags
  auto &actors = Locator<Engine>::get().getActors();
  auto itActor = std::find_if(actors.begin(), actors.end(), [pEntity](auto &pActor) -> bool {
    return pActor->getName() == pEntity->getName();
  });
  if (itActor != actors.end()) {
    return itActor->get();
  }
  return nullptr;
}

Entity *Entity::getParent() {
  return m_pImpl->m_pParent;
}

const Entity *Entity::getParent() const {
  return m_pImpl->m_pParent;
}

bool Entity::hasParent() const { return m_pImpl->m_pParent != nullptr; }

void Entity::setParent(Entity *pParent) {
  auto pOldParent = m_pImpl->m_pParent;
  if (pOldParent) {
    pOldParent->m_pImpl->m_children.erase(std::remove_if(pOldParent->m_pImpl->m_children.begin(),
                                                         pOldParent->m_pImpl->m_children.end(),
                                                         [this](const auto *pChild) {
                                                           return pChild == this;
                                                         }), pOldParent->m_pImpl->m_children.end());
  }
  m_pImpl->m_pParent = pParent;
  if (pParent) {
    pParent->m_pImpl->m_children.push_back(this);
  }
}

std::vector<Entity *> Entity::getChildren() const {
  return m_pImpl->m_children;
}

void Entity::setVolume(float volume) {
  ScriptEngine::set(getTable(), "_volume", std::clamp(volume, 0.f, 1.f));
}

float Entity::getVolume() const {
  float volume = 1.f;
  ScriptEngine::rawGet(getTable(), "_volume", volume);
  return volume;
}

} // namespace ng