#include <memory>
#include <optional>
#include <utility>
#include <glm/vec2.hpp>
#include "engge/Entities/Entity.hpp"
#include "engge/Scripting/ScriptEngine.hpp"
#include "engge/Audio/SoundTrigger.hpp"
#include "engge/Engine/Camera.hpp"
#include "engge/Engine/Trigger.hpp"
#include "Actor/_TalkingState.hpp"

namespace ng {
struct Entity::Impl {
  Engine &_engine;
  std::map<int, Trigger *> _triggers;
  std::vector<std::unique_ptr<SoundTrigger>> _soundTriggers;
  std::optional<glm::vec2> _usePos;
  std::optional<UseDirection> _useDir;
  glm::vec2 _offset{0, 0};
  bool _isLit{true};
  bool _isVisible{true};
  bool _isTouchable{true};
  glm::ivec2 _renderOffset{0, 0};
  Motor _offsetTo, _scaleTo, _rotateTo, _moveTo, _alphaTo;
  ngf::Color _color{ngf::Colors::White};
  bool _objectBumperCycle{true};
  ngf::Color _talkColor;
  glm::ivec2 _talkOffset{0, 0};
  _TalkingState _talkingState;
  std::string _key;

  Impl() : _engine(ng::Locator<ng::Engine>::get()) {
    _talkingState.setEngine(&_engine);
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

Entity::Entity() : pImpl(std::make_unique<Entity::Impl>()) {
}

Entity::~Entity() = default;

void Entity::objectBumperCycle(bool enabled) { pImpl->_objectBumperCycle = enabled; }

bool Entity::objectBumperCycle() const { return pImpl->_objectBumperCycle; }

void Entity::update(const ngf::TimeSpan &elapsed) {
  pImpl->_talkingState.update(elapsed);
  update(pImpl->_offsetTo, elapsed);
  update(pImpl->_scaleTo, elapsed);
  update(pImpl->_rotateTo, elapsed);
  update(pImpl->_moveTo, elapsed);
  update(pImpl->_alphaTo, elapsed);
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
  pImpl->_isLit = isLit;
}

bool Entity::isLit() const {
  return pImpl->_isLit;
}

void Entity::setVisible(bool isVisible) {
  pImpl->_isVisible = isVisible;
}

bool Entity::isVisible() const {
  return pImpl->_isVisible;
}

void Entity::setUsePosition(std::optional<glm::vec2> pos) {
  pImpl->_usePos = pos;
}

void Entity::setUseDirection(std::optional<UseDirection> direction) {
  pImpl->_useDir = direction;
}

std::optional<UseDirection> Entity::getUseDirection() const {
  return pImpl->_useDir;
}

void Entity::setPosition(const glm::vec2 &pos) {
  _transform.setPosition(pos);
  pImpl->_moveTo.isEnabled = false;
}

glm::vec2 Entity::getPosition() const {
  return _transform.getPosition();
}

glm::vec2 Entity::getRealPosition() const {
  return getPosition() + pImpl->_offset;
}

void Entity::setOffset(const glm::vec2 &offset) {
  pImpl->_offset = offset;
  pImpl->_offsetTo.isEnabled = false;
}

glm::vec2 Entity::getOffset() const {
  return pImpl->_offset;
}

void Entity::setRotation(float angle) { _transform.setRotation(angle); }
float Entity::getRotation() const {
  // SFML give rotation in degree between [0, 360]
  float angle = _transform.getRotation();
  // convert it to [-180, 180]
  if (angle > 180)
    angle -= 360;
  return angle;
}

void Entity::setColor(const ngf::Color &color) {
  pImpl->_color = color;
  pImpl->_alphaTo.isEnabled = false;
}

const ngf::Color &Entity::getColor() const {
  return pImpl->_color;
}

void Entity::setScale(float s) {
  _transform.setScale({s, s});
  pImpl->_scaleTo.isEnabled = false;
}

float Entity::getScale() const {
  return _transform.getScale().x;
}

ngf::Transform Entity::getTransform() const {
  auto transform = _transform;
  transform.move(pImpl->_offset);
  return transform;
}

std::optional<glm::vec2> Entity::getUsePosition() const {
  return pImpl->_usePos;
}

void Entity::setTrigger(int triggerNumber, Trigger *pTrigger) {
  pImpl->_triggers[triggerNumber] = pTrigger;
}

void Entity::removeTrigger(int triggerNumber) {
  pImpl->_triggers.erase(triggerNumber);
}

void Entity::trig(int triggerNumber) {
  auto it = pImpl->_triggers.find(triggerNumber);
  if (it != pImpl->_triggers.end()) {
    it->second->trig();
  }
}

void Entity::trigSound(const std::string &) {
}

void Entity::drawForeground(ngf::RenderTarget &target, ngf::RenderStates s) const {
  if (!pImpl->_talkingState.isTalking())
    return;

  pImpl->_talkingState.draw(target, s);
}

SoundTrigger *Entity::createSoundTrigger(Engine &engine, const std::vector<SoundDefinition *> &sounds) {
  auto trigger = std::make_unique<SoundTrigger>(engine, sounds, this->getId());
  SoundTrigger *pTrigger = trigger.get();
  pImpl->_soundTriggers.push_back(std::move(trigger));
  return pTrigger;
}

void Entity::setKey(const std::string &key) { pImpl->_key = key; }

const std::string &Entity::getKey() const { return pImpl->_key; }

uint32_t Entity::getFlags() const {
  int flags = 0;
  ScriptEngine::rawGet(this, "flags", flags);
  return (uint32_t) flags;
}

void Entity::setTouchable(bool isTouchable) {
  pImpl->_isTouchable = isTouchable;
}

bool Entity::isTouchable() const {
  if (!isVisible())
    return false;
  return pImpl->_isTouchable;
}

void Entity::setRenderOffset(const glm::ivec2 &offset) {
  pImpl->_renderOffset = offset;
}

glm::ivec2 Entity::getRenderOffset() const {
  return pImpl->_renderOffset;
}

void Entity::alphaTo(float destination, ngf::TimeSpan time, InterpolationMethod method) {
  auto getAlpha = [this] { return static_cast<float>(getColor().a) / 255.f; };
  auto setAlpha = [this](const float &a) {
    pImpl->_color.a = static_cast<sf::Uint8>(a * 255.f);
  };
  auto alphaTo = std::make_unique<ChangeProperty<float>>(getAlpha, setAlpha, destination, time, method);
  pImpl->_alphaTo.function = std::move(alphaTo);
  pImpl->_alphaTo.isEnabled = true;
}

void Entity::offsetTo(glm::vec2 destination, ngf::TimeSpan time, InterpolationMethod method) {
  auto get = [this] { return pImpl->_offset; };
  auto set = [this](const glm::vec2 &value) { pImpl->_offset = value; };
  auto offsetTo = std::make_unique<ChangeProperty<glm::vec2>>(get, set, destination, time, method);
  pImpl->_offsetTo.function = std::move(offsetTo);
  pImpl->_offsetTo.isEnabled = true;
}

void Entity::moveTo(glm::vec2 destination, ngf::TimeSpan time, InterpolationMethod method) {
  auto get = [this] { return getPosition(); };
  auto set = [this](const glm::vec2 &value) { _transform.setPosition(value); };
  auto moveTo = std::make_unique<ChangeProperty<glm::vec2>>(get, set, destination, time, method);
  pImpl->_moveTo.function = std::move(moveTo);
  pImpl->_moveTo.isEnabled = true;
}

void Entity::rotateTo(float destination, ngf::TimeSpan time, InterpolationMethod method) {
  auto get = [this] { return getRotation(); };
  auto set = [this](const float &value) { _transform.setRotation(value); };
  auto rotateTo =
      std::make_unique<ChangeProperty<float>>(get, set, destination, time, method);
  pImpl->_rotateTo.function = std::move(rotateTo);
  pImpl->_rotateTo.isEnabled = true;
}

void Entity::scaleTo(float destination, ngf::TimeSpan time, InterpolationMethod method) {
  auto get = [this] { return _transform.getScale().x; };
  auto set = [this](const float &s) { _transform.setScale({s, s}); };
  auto scalteTo = std::make_unique<ChangeProperty<float>>(get, set, destination, time, method);
  pImpl->_scaleTo.function = std::move(scalteTo);
  pImpl->_scaleTo.isEnabled = true;
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
  pImpl->_offsetTo.isEnabled = false;
  pImpl->_scaleTo.isEnabled = false;
  pImpl->_rotateTo.isEnabled = false;
  pImpl->_moveTo.isEnabled = false;
  pImpl->_alphaTo.isEnabled = false;
}

void Entity::setTalkColor(ngf::Color color) { pImpl->_talkColor = color; }

ngf::Color Entity::getTalkColor() const { return pImpl->_talkColor; }

void Entity::setTalkOffset(const glm::ivec2 &offset) { pImpl->_talkOffset = offset; }

glm::ivec2 Entity::getTalkOffset() const { return pImpl->_talkOffset; }

void Entity::say(const std::string &text, bool mumble) {
  pImpl->_talkingState.loadLip(text, this, mumble);
  glm::vec2 pos;
  auto screenSize = pImpl->_engine.getRoom()->getScreenSize();
  if (getRoom() == pImpl->_engine.getRoom()) {
    auto at = pImpl->_engine.getCamera().getAt();
    pos = getRealPosition();
    pos = {pos.x - at.x + pImpl->_talkOffset.x, screenSize.y - pos.y - at.y - pImpl->_talkOffset.y};
  } else {
    // TODO: the position in this case is wrong, don't know what to do yet
    pos = (glm::vec2) pImpl->_talkOffset;
    pos = {pos.x, screenSize.y + pos.y};
  }
  pos = toDefaultView((glm::ivec2) pos, pImpl->_engine.getRoom()->getScreenSize());
  pImpl->_talkingState.setPosition(pos);
}

void Entity::stopTalking() { pImpl->_talkingState.stop(); }

bool Entity::isTalking() const { return pImpl->_talkingState.isTalking(); }

int Entity::getDefaultVerb(int defaultVerbId) const {
  auto result = pImpl->getDefaultVerb(this);
  if (result.has_value())
    return result.value();

  result = pImpl->getDefaultVerb(getActor(this));
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

} // namespace ng