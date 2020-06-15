#include <utility>
#include <memory>
#include "Entities/Entity.hpp"
#include "Scripting/ScriptEngine.hpp"
#include "Audio/SoundTrigger.hpp"
#include "Engine/Camera.hpp"
#include "Engine/Trigger.hpp"
#include "Actor/_TalkingState.hpp"

namespace ng {
struct Entity::Impl {
  Engine &_engine;
  std::map<int, Trigger *> _triggers;
  std::vector<std::unique_ptr<SoundTrigger>> _soundTriggers;
  std::optional<sf::Vector2f> _usePos;
  std::optional<UseDirection> _useDir;
  sf::Vector2f _offset;
  bool _isLit{true};
  bool _isVisible{true};
  bool _isTouchable{true};
  sf::Vector2i _renderOffset;
  Motor _offsetTo, _scaleTo, _rotateTo, _moveTo, _alphaTo;
  sf::Color _color{sf::Color::White};
  bool _objectBumperCycle{true};
  sf::Color _talkColor;
  sf::Vector2i _talkOffset;
  _TalkingState _talkingState;
  std::string _key;

  Impl() : _engine(ng::Locator<ng::Engine>::get()) {
    _talkingState.setEngine(&_engine);
  }
};

Entity::Entity() : pImpl(std::make_unique<Entity::Impl>()) {
}

Entity::~Entity() = default;

void Entity::objectBumperCycle(bool enabled) { pImpl->_objectBumperCycle = enabled; }

bool Entity::objectBumperCycle() const { return pImpl->_objectBumperCycle; }

void Entity::update(const sf::Time &elapsed) {
  pImpl->_talkingState.update(elapsed);
  update(pImpl->_offsetTo, elapsed);
  update(pImpl->_scaleTo, elapsed);
  update(pImpl->_rotateTo, elapsed);
  update(pImpl->_moveTo, elapsed);
  update(pImpl->_alphaTo, elapsed);
}

void Entity::update(Motor &motor, const sf::Time &elapsed) {
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

void Entity::setUsePosition(std::optional<sf::Vector2f> pos) {
  pImpl->_usePos = pos;
}

void Entity::setUseDirection(std::optional<UseDirection> direction) {
  pImpl->_useDir = direction;
}

std::optional<UseDirection> Entity::getUseDirection() const {
  return pImpl->_useDir;
}

void Entity::setPosition(const sf::Vector2f &pos) {
  _transform.setPosition(pos);
  pImpl->_moveTo.isEnabled = false;
}

sf::Vector2f Entity::getPosition() const {
  return _transform.getPosition();
}

sf::Vector2f Entity::getRealPosition() const {
  return getPosition() + pImpl->_offset;
}

void Entity::setOffset(const sf::Vector2f &offset) {
  pImpl->_offset = offset;
  pImpl->_offsetTo.isEnabled = false;
}

sf::Vector2f Entity::getOffset() const {
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

void Entity::setColor(const sf::Color &color) {
  pImpl->_color = color;
  pImpl->_alphaTo.isEnabled = false;
}

const sf::Color &Entity::getColor() const {
  return pImpl->_color;
}

void Entity::setScale(float s) {
  _transform.setScale(s, s);
  pImpl->_scaleTo.isEnabled = false;
}

float Entity::getScale() const {
  return _transform.getScale().x;
}

sf::Transformable Entity::getTransform() const {
  auto transform = _transform;
  transform.move(pImpl->_offset.x, pImpl->_offset.y);
  return transform;
}

std::optional<sf::Vector2f> Entity::getUsePosition() const {
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

void Entity::drawForeground(sf::RenderTarget &target, sf::RenderStates s) const {
  if (!pImpl->_talkingState.isTalking())
    return;

  target.draw(pImpl->_talkingState, s);
}

SoundTrigger *Entity::createSoundTrigger(Engine &engine, const std::vector<SoundDefinition *> &sounds) {
  auto trigger = std::make_unique<SoundTrigger>(engine, sounds, this->getId());
  SoundTrigger *pTrigger = trigger.get();
  pImpl->_soundTriggers.push_back(std::move(trigger));
  return pTrigger;
}

void Entity::setKey(const std::string &key) { pImpl->_key = key; }

const std::string &Entity::getKey() const { return pImpl->_key; }

void Entity::setTouchable(bool isTouchable) {
  pImpl->_isTouchable = isTouchable;
}

bool Entity::isTouchable() const {
  if (!isVisible())
    return false;
  return pImpl->_isTouchable;
}

void Entity::setRenderOffset(const sf::Vector2i &offset) {
  pImpl->_renderOffset = offset;
}

sf::Vector2i Entity::getRenderOffset() const {
  return pImpl->_renderOffset;
}

void Entity::alphaTo(float destination, sf::Time time, InterpolationMethod method) {
  auto getAlpha = [this] { return static_cast<float>(getColor().a) / 255.f; };
  auto setAlpha = [this](const float &a) {
    pImpl->_color.a = static_cast<sf::Uint8>(a * 255.f);
  };
  auto alphaTo = std::make_unique<ChangeProperty<float>>(getAlpha, setAlpha, destination, time, method);
  pImpl->_alphaTo.function = std::move(alphaTo);
  pImpl->_alphaTo.isEnabled = true;
}

void Entity::offsetTo(sf::Vector2f destination, sf::Time time, InterpolationMethod method) {
  auto get = [this] { return pImpl->_offset; };
  auto set = [this](const sf::Vector2f &value) { pImpl->_offset = value; };
  auto offsetTo = std::make_unique<ChangeProperty<sf::Vector2f>>(get, set, destination, time, method);
  pImpl->_offsetTo.function = std::move(offsetTo);
  pImpl->_offsetTo.isEnabled = true;
}

void Entity::moveTo(sf::Vector2f destination, sf::Time time, InterpolationMethod method) {
  auto get = [this] { return getPosition(); };
  auto set = [this](const sf::Vector2f &value) { _transform.setPosition(value); };
  auto moveTo = std::make_unique<ChangeProperty<sf::Vector2f>>(get, set, destination, time, method);
  pImpl->_moveTo.function = std::move(moveTo);
  pImpl->_moveTo.isEnabled = true;
}

void Entity::rotateTo(float destination, sf::Time time, InterpolationMethod method) {
  auto get = [this] { return getRotation(); };
  auto set = [this](const float &value) { _transform.setRotation(value); };
  auto rotateTo =
      std::make_unique<ChangeProperty<float>>(get, set, destination, time, method);
  pImpl->_rotateTo.function = std::move(rotateTo);
  pImpl->_rotateTo.isEnabled = true;
}

void Entity::scaleTo(float destination, sf::Time time, InterpolationMethod method) {
  auto get = [this] { return _transform.getScale().x; };
  auto set = [this](const float &s) { _transform.setScale(s, s); };
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

void Entity::setTalkColor(sf::Color color) { pImpl->_talkColor = color; }

sf::Color Entity::getTalkColor() const { return pImpl->_talkColor; }

void Entity::setTalkOffset(const sf::Vector2i &offset) { pImpl->_talkOffset = offset; }

void Entity::say(const std::string &text, bool mumble) {
  pImpl->_talkingState.loadLip(text, this, mumble);
  sf::Vector2f pos;
  auto screenSize = pImpl->_engine.getRoom()->getScreenSize();
  if (getRoom() == pImpl->_engine.getRoom()) {
    auto at = pImpl->_engine.getCamera().getAt();
    pos = getRealPosition();
    pos = {pos.x - at.x + pImpl->_talkOffset.x, screenSize.y - pos.y - at.y - pImpl->_talkOffset.y};
  } else {
    // TODO: the position in this case is wrong, don't know what to do yet
    pos = (sf::Vector2f) pImpl->_talkOffset;
    pos = {pos.x, screenSize.y + pos.y};
  }
  pos = toDefaultView((sf::Vector2i) pos, pImpl->_engine.getRoom()->getScreenSize());
  pImpl->_talkingState.setPosition(pos);
}

void Entity::stopTalking() { pImpl->_talkingState.stop(); }

bool Entity::isTalking() const { return pImpl->_talkingState.isTalking(); }

} // namespace ng