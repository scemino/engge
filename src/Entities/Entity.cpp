#include <utility>
#include "Entities/Entity.hpp"
#include "Room/Room.hpp"
#include "Scripting/ScriptEngine.hpp"
#include "Audio/SoundTrigger.hpp"
#include "Engine/Trigger.hpp"

namespace ng {
void Entity::update(const sf::Time &elapsed) {
  update(_offsetTo, elapsed);
  update(_scaleTo, elapsed);
  update(_rotateTo, elapsed);
  update(_moveTo, elapsed);
  update(_alphaTo, elapsed);
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
  _isLit = isLit;
}

bool Entity::isLit() const {
  return _isLit;
}

void Entity::setVisible(bool isVisible) {
  _isVisible = isVisible;
}

bool Entity::isVisible() const {
  return _isVisible;
}

void Entity::setUsePosition(std::optional<sf::Vector2f> pos) {
  _usePos = pos;
}

void Entity::setUseDirection(std::optional<UseDirection> direction) {
  _useDir = direction;
}

std::optional<UseDirection> Entity::getUseDirection() const {
  return _useDir;
}

void Entity::setPosition(const sf::Vector2f &pos) {
  _transform.setPosition(pos);
  _moveTo.isEnabled = false;
}

sf::Vector2f Entity::getPosition() const {
  return _transform.getPosition();
}

sf::Vector2f Entity::getRealPosition() const {
  return getPosition() + _offset;
}

void Entity::setOffset(const sf::Vector2f &offset) {
  _offset = offset;
  _offsetTo.isEnabled = false;
}

sf::Vector2f Entity::getOffset() const {
  return _offset;
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
  _color = color;
  _alphaTo.isEnabled = false;
}

const sf::Color &Entity::getColor() const {
  return _color;
}

void Entity::setScale(float s) {
  _transform.setScale(s, s);
  _scaleTo.isEnabled = false;
}

float Entity::getScale() const {
  return _transform.getScale().x;
}

sf::Transformable Entity::getTransform() const {
  auto transform = _transform;
  transform.move(_offset.x, _offset.y);
  return transform;
}

std::optional<sf::Vector2f> Entity::getUsePosition() const {
  return _usePos;
}

void Entity::setTrigger(int triggerNumber, Trigger *pTrigger) {
  _triggers[triggerNumber] = pTrigger;
}

void Entity::removeTrigger(int triggerNumber) {
  _triggers.erase(triggerNumber);
}

void Entity::trig(int triggerNumber) {
  auto it = _triggers.find(triggerNumber);
  if (it != _triggers.end()) {
    it->second->trig();
  }
}

void Entity::trigSound(const std::string &) {
}

void Entity::drawForeground(sf::RenderTarget &, sf::RenderStates) const {
}

SoundTrigger *Entity::createSoundTrigger(Engine &engine, const std::vector<SoundDefinition *> &sounds) {
  auto trigger = std::make_unique<SoundTrigger>(engine, sounds, this->getId());
  SoundTrigger *pTrigger = trigger.get();
  _soundTriggers.push_back(std::move(trigger));
  return pTrigger;
}

void Entity::setTouchable(bool isTouchable) {
  _isTouchable = isTouchable;
}

bool Entity::isTouchable() const {
  if (!isVisible())
    return false;
  return _isTouchable;
}

void Entity::setRenderOffset(const sf::Vector2i &offset) {
  _renderOffset = offset;
}

sf::Vector2i Entity::getRenderOffset() const {
  return _renderOffset;
}

void Entity::alphaTo(float destination, sf::Time time, InterpolationMethod method) {
  auto getAlpha = [this] { return static_cast<float>(getColor().a) / 255.f; };
  auto setAlpha = [this](const float &a) {
    _color.a = static_cast<sf::Uint8>(a * 255.f);
  };
  auto alphaTo = std::make_unique<ChangeProperty<float>>(getAlpha, setAlpha, destination, time, method);
  _alphaTo.function = std::move(alphaTo);
  _alphaTo.isEnabled = true;
}

void Entity::offsetTo(sf::Vector2f destination, sf::Time time, InterpolationMethod method) {
  auto get = [this] { return _offset; };
  auto set = [this](const sf::Vector2f &value) { _offset = value; };
  auto offsetTo = std::make_unique<ChangeProperty<sf::Vector2f>>(get, set, destination, time, method);
  _offsetTo.function = std::move(offsetTo);
  _offsetTo.isEnabled = true;
}

void Entity::moveTo(sf::Vector2f destination, sf::Time time, InterpolationMethod method) {
  auto get = [this] { return getPosition(); };
  auto set = [this](const sf::Vector2f &value) { _transform.setPosition(value); };
  auto moveTo = std::make_unique<ChangeProperty<sf::Vector2f>>(get, set, destination, time, method);
  _moveTo.function = std::move(moveTo);
  _moveTo.isEnabled = true;
}

void Entity::rotateTo(float destination, sf::Time time, InterpolationMethod method) {
  auto get = [this] { return getRotation(); };
  auto set = [this](const float &value) { _transform.setRotation(value); };
  auto rotateTo =
      std::make_unique<ChangeProperty<float>>(get, set, destination, time, method);
  _rotateTo.function = std::move(rotateTo);
  _rotateTo.isEnabled = true;
}

void Entity::scaleTo(float destination, sf::Time time, InterpolationMethod method) {
  auto get = [this] { return _transform.getScale().x; };
  auto set = [this](const float &s) { _transform.setScale(s, s); };
  auto scalteTo = std::make_unique<ChangeProperty<float>>(get, set, destination, time, method);
  _scaleTo.function = std::move(scalteTo);
  _scaleTo.isEnabled = true;
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
  _offsetTo.isEnabled = false;
  _scaleTo.isEnabled = false;
  _rotateTo.isEnabled = false;
  _moveTo.isEnabled = false;
  _alphaTo.isEnabled = false;
}

} // namespace ng