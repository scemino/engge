#include "Entities/Objects/Object.hpp"
#include "Entities/Objects/Animation.hpp"
#include "Entities/Objects/AnimationFrame.hpp"
#include "Engine/Function.hpp"
#include "System/Locator.hpp"
#include "Engine/ResourceManager.hpp"
#include "Room/Room.hpp"
#include "Graphics/Screen.hpp"
#include "Scripting/ScriptEngine.hpp"
#include "Audio/SoundTrigger.hpp"
#include "Engine/Trigger.hpp"
#include "../../System/_Util.hpp"
#include <sstream>

namespace ng {
struct Object::Impl {
  std::vector<std::unique_ptr<Animation>> _anims;
  std::optional<Animation *> _pAnim{std::nullopt};
  std::wstring _name;
  int _zorder{0};
  ObjectType _type{ObjectType::Object};
  sf::Vector2f _usePos;
  sf::IntRect _hotspot;
  Room *_pRoom{nullptr};
  int _state{0};
  std::optional<std::shared_ptr<Trigger>> _trigger;
  HSQOBJECT _pTable{};
  bool _hotspotVisible{false};
  bool _triggerEnabled{true};
  Object *pParentObject{nullptr};
  int dependentState{0};
  Actor *_owner{nullptr};
  int _fps{0};
  std::vector<std::string> _icons;
  sf::Time _elapsed;
  int _index{0};
  ScreenSpace _screenSpace{ScreenSpace::Room};
  std::vector<Object *> _children;
  bool _temporary{false};
  bool _jiggle{false};
  std::string _key;
  Object *_pParent{nullptr};
};

Object::Object() : pImpl(std::make_unique<Impl>()) {
  _id = Locator<ResourceManager>::get().getObjectId();
}

Object::~Object() = default;

void Object::setKey(const std::string &key) { pImpl->_key = key; }

const std::string &Object::getKey() const { return pImpl->_key; }

void Object::setZOrder(int zorder) { pImpl->_zorder = zorder; }

int Object::getZOrder() const { return pImpl->_zorder; }

void Object::setType(ObjectType type) { pImpl->_type = type; }
ObjectType Object::getType() const { return pImpl->_type; }

void Object::setHotspot(const sf::IntRect &hotspot) { pImpl->_hotspot = hotspot; }
const sf::IntRect &Object::getHotspot() const { return pImpl->_hotspot; }

void Object::setIcon(const std::string &icon) {
  pImpl->_icons.clear();
  pImpl->_fps = 0;
  pImpl->_index = 0;
  pImpl->_elapsed = sf::seconds(0);
  pImpl->_icons.push_back(icon);
}

std::string Object::getIcon() const { return pImpl->_icons.at(pImpl->_index); }

void Object::setIcon(int fps, const std::vector<std::string> &icons) {
  pImpl->_icons.clear();
  pImpl->_fps = fps;
  pImpl->_index = 0;
  pImpl->_elapsed = sf::seconds(0);
  std::copy(icons.begin(), icons.end(), std::back_inserter(pImpl->_icons));
}

void Object::setOwner(Actor *pActor) { pImpl->_owner = pActor; }
Actor *Object::getOwner() const { return pImpl->_owner; }

HSQOBJECT &Object::getTable() { return pImpl->_pTable; }
HSQOBJECT &Object::getTable() const { return pImpl->_pTable; }
bool Object::isInventoryObject() const { return getOwner() != nullptr; }

std::vector<std::unique_ptr<Animation>> &Object::getAnims() { return pImpl->_anims; }

Room *Object::getRoom() { return pImpl->_pRoom; }
const Room *Object::getRoom() const { return pImpl->_pRoom; }
void Object::setRoom(Room *pRoom) { pImpl->_pRoom = pRoom; }

void Object::addTrigger(const std::shared_ptr<Trigger> &trigger) { pImpl->_trigger = trigger; }

void Object::removeTrigger() {
  if (pImpl->_trigger.has_value()) {
    (*pImpl->_trigger)->disable();
  }
}

Trigger *Object::getTrigger() { return pImpl->_trigger.has_value() ? (*pImpl->_trigger).get() : nullptr; }
void Object::enableTrigger(bool enabled) { pImpl->_triggerEnabled = enabled; }

bool Object::isTouchable() const {
  if (!isVisible())
    return false;
  if (getType() != ObjectType::Object)
    return false;
  return Entity::isTouchable();
}

sf::IntRect Object::getRealHotspot() const {
  auto rect = getHotspot();
  auto transform = getTransform().getTransform();
  return (sf::IntRect) transform.transformRect((sf::FloatRect) rect);
}

bool Object::isVisible() const {
  if (pImpl->_state == ObjectStateConstants::GONE)
    return false;
  return Entity::isVisible();
}

void Object::setStateAnimIndex(int animIndex) {
  std::ostringstream s;
  s << "state" << animIndex;
  pImpl->_state = animIndex;

  setVisible(animIndex != ObjectStateConstants::GONE);
  setAnimation(s.str());
}

void Object::playAnim(const std::string &anim, bool loop) {
  setAnimation(anim);
  (*pImpl->_pAnim)->play(loop);
}

void Object::playAnim(int animIndex, bool loop) {
  setStateAnimIndex(animIndex);
  (*pImpl->_pAnim)->play(loop);
}

int Object::getState() const { return pImpl->_state; }

void Object::setAnimation(const std::string &name) {
  auto it = std::find_if(pImpl->_anims.begin(), pImpl->_anims.end(),
                         [name](std::unique_ptr<Animation> &animation) { return animation->getName() == name; });
  if (it == pImpl->_anims.end()) {
    pImpl->_pAnim = std::nullopt;
    return;
  }

  auto &anim = *(it->get());
  pImpl->_pAnim = &anim;
}

std::optional<Animation *> &Object::getAnimation() { return pImpl->_pAnim; }

void Object::update(const sf::Time &elapsed) {
  if (isInventoryObject()) {
    if (pImpl->_fps == 0)
      return;
    pImpl->_elapsed += elapsed;
    if (pImpl->_elapsed.asSeconds() > (1.f / pImpl->_fps)) {
      pImpl->_elapsed = sf::seconds(0);
      pImpl->_index = (pImpl->_index + 1) % pImpl->_icons.size();
    }
    return;
  }

  Entity::update(elapsed);
  if (pImpl->pParentObject) {
    setVisible(pImpl->pParentObject->getState() == pImpl->dependentState);
  }
  if (pImpl->_pAnim) {
    (*pImpl->_pAnim)->update(elapsed);
  }
  if (pImpl->_triggerEnabled && pImpl->_trigger.has_value()) {
    (*pImpl->_trigger)->trig();
  }
}

void Object::showHotspot(bool show) { pImpl->_hotspotVisible = show; }

bool Object::isHotspotVisible() const { return pImpl->_hotspotVisible; }

void Object::setScreenSpace(ScreenSpace screenSpace) { pImpl->_screenSpace = screenSpace; }

ScreenSpace Object::getScreenSpace() const { return pImpl->_screenSpace; }

void Object::drawHotspot(sf::RenderTarget &target, sf::RenderStates states) const {
  if (!pImpl->_hotspotVisible)
    return;

  auto rect = getHotspot();

  sf::Color color;
  switch (getType()) {
  case ObjectType::Object:color = sf::Color::Red;
    break;
  case ObjectType::Spot:color = sf::Color::Green;
    break;
  case ObjectType::Trigger:color = sf::Color::Magenta;
    break;
  case ObjectType::Prop:color = sf::Color::Blue;
    break;
  }

  sf::RectangleShape s(sf::Vector2f(rect.width, rect.height));
  s.setPosition(rect.left, rect.top);
  s.setOutlineThickness(1);
  s.setOutlineColor(color);
  s.setFillColor(sf::Color::Transparent);
  target.draw(s, states);

  sf::RectangleShape vl(sf::Vector2f(1, 7));
  vl.setPosition(pImpl->_usePos.x, pImpl->_usePos.y - 3);
  vl.setFillColor(color);
  target.draw(vl, states);

  sf::RectangleShape hl(sf::Vector2f(7, 1));
  hl.setPosition(pImpl->_usePos.x - 3, pImpl->_usePos.y);
  hl.setFillColor(color);
  target.draw(hl, states);

  auto useDir = getUseDirection().value_or(UseDirection::Front);
  switch (useDir) {
  case UseDirection::Front: {
    sf::RectangleShape dirShape(sf::Vector2f(3, 1));
    dirShape.setPosition(pImpl->_usePos.x - 1, pImpl->_usePos.y + 2);
    dirShape.setFillColor(color);
    target.draw(dirShape, states);
  }
    break;
  case UseDirection::Back: {
    sf::RectangleShape dirShape(sf::Vector2f(3, 1));
    dirShape.setPosition(pImpl->_usePos.x - 1, pImpl->_usePos.y - 2);
    dirShape.setFillColor(color);
    target.draw(dirShape, states);
  }
    break;
  case UseDirection::Left: {
    sf::RectangleShape dirShape(sf::Vector2f(1, 3));
    dirShape.setPosition(pImpl->_usePos.x - 2, pImpl->_usePos.y - 1);
    dirShape.setFillColor(color);
    target.draw(dirShape, states);
  }
    break;
  case UseDirection::Right: {
    sf::RectangleShape dirShape(sf::Vector2f(1, 3));
    dirShape.setPosition(pImpl->_usePos.x + 2, pImpl->_usePos.y - 1);
    dirShape.setFillColor(color);
    target.draw(dirShape, states);
  }
    break;
  }
}

void Object::drawForeground(sf::RenderTarget &target, sf::RenderStates) const {
  if (pImpl->_screenSpace != ScreenSpace::Object)
    return;

  const auto view = target.getView();
  target.setView(sf::View(sf::FloatRect(0, 0, Screen::Width, Screen::Height)));

  sf::RenderStates s;
  auto transformable = getTransform();
  transformable.setPosition(transformable.getPosition().x,
                            target.getView().getSize().y - transformable.getPosition().y);
  s.transform *= transformable.getTransform();

  if (pImpl->_pAnim) {
    (*pImpl->_pAnim)->setColor(getColor());
    target.draw(*(*pImpl->_pAnim), s);
  }

  drawHotspot(target, s);
  target.setView(view);
}

void Object::draw(sf::RenderTarget &target, sf::RenderStates states) const {
  sf::RenderStates initialStates = states;
  if (!isVisible())
    return;

  if (pImpl->_screenSpace == ScreenSpace::Object)
    return;

  auto transformable = getTransform();
  transformable.setPosition(transformable.getPosition().x,
                            target.getView().getSize().y - transformable.getPosition().y);
  states.transform *= transformable.getTransform();

  if (pImpl->_pAnim) {
    (*pImpl->_pAnim)->setColor(getColor());
    target.draw(*(*pImpl->_pAnim), states);
  }

  drawHotspot(target, states);

  for(const auto& pChild : pImpl->_children) {
    target.draw(*pChild, initialStates);
  }
}

void Object::dependentOn(Object *parentObject, int state) {
  pImpl->dependentState = state;
  pImpl->pParentObject = parentObject;
}

void Object::setFps(int fps) {
  if (pImpl->_pAnim.has_value()) {
    (*pImpl->_pAnim)->setFps(fps);
  }
}

void Object::setParent(Object *pParent) {
  auto pOldParent = pImpl->_pParent;
  if (pOldParent) {
    pOldParent->pImpl->_children.erase(std::remove_if(pOldParent->pImpl->_children.begin(),
                                                      pOldParent->pImpl->_children.end(),
                                                      [this](const auto *pChild) {
                                                        return pChild == this;
                                                      }), pOldParent->pImpl->_children.end());
  }
  pImpl->_pParent = pParent;
  if (pParent) {
    pParent->pImpl->_children.push_back(this);
  }
}

Object *Object::getParent() { return pImpl->_pParent; }

bool Object::hasParent() const { return pImpl->_pParent != nullptr; }

void Object::stopObjectMotors() {
  Entity::stopObjectMotors();
  for (auto &&child : pImpl->_children) {
    child->stopObjectMotors();
  }
}

void Object::setTemporary(bool isTemporary) { pImpl->_temporary = isTemporary; }

bool Object::isTemporary() const { return pImpl->_temporary; }

void Object::setJiggle(bool enabled) { pImpl->_jiggle = enabled; }

bool Object::getJiggle() const { return pImpl->_jiggle; }

std::wostream &operator<<(std::wostream &os, const Object &obj) {
  return os << towstring(obj.getName()) << L" (" << obj.getRealPosition().x << L"," << obj.getRealPosition().y << L":"
            << obj.getZOrder() << L")";
}

} // namespace ng
