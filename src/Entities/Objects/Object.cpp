#define _USE_MATH_DEFINES
#include <cmath>
#include "engge/Entities/Objects/Object.hpp"
#include "engge/Entities/Objects/Animation.hpp"
#include "engge/Entities/Objects/AnimationFrame.hpp"
#include "engge/Engine/Function.hpp"
#include "engge/System/Locator.hpp"
#include "engge/Engine/EntityManager.hpp"
#include "engge/Room/Room.hpp"
#include "engge/Graphics/Screen.hpp"
#include "engge/Scripting/ScriptEngine.hpp"
#include "engge/Audio/SoundTrigger.hpp"
#include "engge/Engine/Trigger.hpp"
#include "engge/Engine/Preferences.hpp"
#include "../../System/_Util.hpp"
#include <sstream>

namespace ng {
struct Object::Impl {
  std::vector<std::unique_ptr<Animation>> _anims;
  std::optional<Animation *> _pAnim{std::nullopt};
  std::wstring _name;
  int _zorder{0};
  ObjectType _type{ObjectType::Object};
  sf::IntRect _hotspot;
  Room *_pRoom{nullptr};
  int _state{0};
  std::optional<std::shared_ptr<Trigger>> _trigger;
  HSQOBJECT _table{};
  bool _hotspotVisible{false};
  bool _triggerEnabled{true};
  Object *pParentObject{nullptr};
  int dependentState{0};
  Actor *_owner{nullptr};
  int _fps{0};
  std::vector<std::string> _icons;
  sf::Time _elapsed;
  sf::Time _popElapsed;
  int _index{0};
  ScreenSpace _screenSpace{ScreenSpace::Room};
  std::vector<Object *> _children;
  bool _temporary{false};
  bool _jiggle{false};
  int _pop{0};
  Object *_pParent{nullptr};

  Impl() {
    auto v = ScriptEngine::getVm();
    sq_resetobject(&_table);
    sq_newtable(v);
    sq_getstackobj(v, -1, &_table);
    sq_addref(v, &_table);
    sq_pop(v, 1);
  }

  explicit Impl(HSQOBJECT obj) {
    auto v = ScriptEngine::getVm();
    sq_pushobject(v, obj);
    sq_getstackobj(v, -1, &_table);
    sq_addref(v, &_table);
    sq_pop(v, 1);
  }

  ~Impl() {
    auto v = ScriptEngine::getVm();
    sq_release(v, &_table);
  }
};

Object::Object() : pImpl(std::make_unique<Impl>()) {
  _id = Locator<EntityManager>::get().getObjectId();
  ScriptEngine::set(this, "_id", _id);
}

Object::Object(HSQOBJECT obj) : pImpl(std::make_unique<Impl>(obj)) {
  _id = Locator<EntityManager>::get().getObjectId();
  ScriptEngine::set(this, "_id", _id);
}

Object::~Object() = default;

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

std::string Object::getIcon() const {
  if (!pImpl->_icons.empty())
    return pImpl->_icons.at(pImpl->_index);

  auto v = ScriptEngine::getVm();
  auto top = sq_gettop(v);
  sq_pushobject(v, getTable());
  sq_pushstring(v, _SC("icon"), -1);
  if (SQ_SUCCEEDED(sq_rawget(v, -2))) {
    if (sq_gettype(v, -1) == OT_STRING) {
      const SQChar *icon = nullptr;
      sq_getstring(v, -1, &icon);
      pImpl->_icons.emplace_back(icon);
    } else if (sq_gettype(v, -1) == OT_ARRAY) {
      SQInteger fps = 0;
      pImpl->_index = 0;
      const SQChar *icon = nullptr;
      sq_pushnull(v); // null iterator
      if (SQ_SUCCEEDED(sq_next(v, -2))) {
        sq_getinteger(v, -1, &fps);
        sq_pop(v, 2);
        pImpl->_fps = static_cast<int>(fps);
      }
      while (SQ_SUCCEEDED(sq_next(v, -2))) {
        sq_getstring(v, -1, &icon);
        pImpl->_icons.emplace_back(icon);
        sq_pop(v, 2);
      }
      sq_pop(v, 1); // pops the null iterator
    }
  }
  sq_settop(v, top);
  return pImpl->_icons.at(pImpl->_index);
}

void Object::setIcon(int fps, const std::vector<std::string> &icons) {
  pImpl->_icons.clear();
  pImpl->_fps = fps;
  pImpl->_index = 0;
  pImpl->_elapsed = sf::seconds(0);
  std::copy(icons.begin(), icons.end(), std::back_inserter(pImpl->_icons));
}

void Object::setOwner(Actor *pActor) { pImpl->_owner = pActor; }
Actor *Object::getOwner() const { return pImpl->_owner; }

HSQOBJECT &Object::getTable() { return pImpl->_table; }
HSQOBJECT &Object::getTable() const { return pImpl->_table; }
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

  auto &anim = **it;
  pImpl->_pAnim = &anim;
}

std::optional<Animation *> &Object::getAnimation() { return pImpl->_pAnim; }

void Object::update(const sf::Time &elapsed) {
  if (isInventoryObject()) {
    if (pImpl->_pop > 0) {
      pImpl->_popElapsed += elapsed;
      if (pImpl->_popElapsed.asSeconds() > 0.5f) {
        pImpl->_pop--;
        pImpl->_popElapsed -= sf::seconds(0.5);
      }
    }
    if (pImpl->_fps == 0)
      return;
    pImpl->_elapsed += elapsed;
    if (pImpl->_elapsed.asSeconds() > (1.f / static_cast<float>(pImpl->_fps))) {
      pImpl->_elapsed = sf::seconds(0);
      pImpl->_index = static_cast<int>((pImpl->_index + 1) % pImpl->_icons.size());
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

void Object::showDebugHotspot(bool show) { pImpl->_hotspotVisible = show; }

bool Object::isHotspotVisible() const { return pImpl->_hotspotVisible; }

void Object::setScreenSpace(ScreenSpace screenSpace) { pImpl->_screenSpace = screenSpace; }

ScreenSpace Object::getScreenSpace() const { return pImpl->_screenSpace; }

void Object::drawDebugHotspot(sf::RenderTarget &target, sf::RenderStates states) const {
  if (!pImpl->_hotspotVisible)
    return;

  auto rect = getHotspot();
  // y-axis is inverted
  rect.top = -rect.height - rect.top;

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

  auto usePos = getUsePosition().value_or(sf::Vector2f());
  usePos.y = -usePos.y;
  sf::RectangleShape vl(sf::Vector2f(1, 7));
  vl.setPosition(usePos.x, usePos.y - 3);
  vl.setFillColor(color);
  target.draw(vl, states);

  sf::RectangleShape hl(sf::Vector2f(7, 1));
  hl.setPosition(usePos.x - 3, usePos.y);
  hl.setFillColor(color);
  target.draw(hl, states);

  auto useDir = getUseDirection().value_or(UseDirection::Front);
  switch (useDir) {
  case UseDirection::Front: {
    sf::RectangleShape dirShape(sf::Vector2f(3, 1));
    dirShape.setPosition(usePos.x - 1, usePos.y + 2);
    dirShape.setFillColor(color);
    target.draw(dirShape, states);
  }
    break;
  case UseDirection::Back: {
    sf::RectangleShape dirShape(sf::Vector2f(3, 1));
    dirShape.setPosition(usePos.x - 1, usePos.y - 2);
    dirShape.setFillColor(color);
    target.draw(dirShape, states);
  }
    break;
  case UseDirection::Left: {
    sf::RectangleShape dirShape(sf::Vector2f(1, 3));
    dirShape.setPosition(usePos.x - 2, usePos.y - 1);
    dirShape.setFillColor(color);
    target.draw(dirShape, states);
  }
    break;
  case UseDirection::Right: {
    sf::RectangleShape dirShape(sf::Vector2f(1, 3));
    dirShape.setPosition(usePos.x + 2, usePos.y - 1);
    dirShape.setFillColor(color);
    target.draw(dirShape, states);
  }
    break;
  }
}

void Object::drawForeground(sf::RenderTarget &target, sf::RenderStates states) const {
  Entity::drawForeground(target, states);
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

  sf::RenderStates statesHotspot;
  transformable = getTransform();
  transformable.setPosition(transformable.getPosition().x,
                            target.getView().getSize().y - transformable.getPosition().y);
  transformable.setScale(1.f, 1.f);
  statesHotspot.transform *= transformable.getTransform();

  drawHotspot(target, statesHotspot);
  drawDebugHotspot(target, statesHotspot);

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

  for (const auto &pChild : pImpl->_children) {
    target.draw(*pChild, initialStates);
  }

  sf::RenderStates statesHotspot = initialStates;
  transformable = getTransform();
  transformable.setPosition(transformable.getPosition().x,
                            target.getView().getSize().y - transformable.getPosition().y);
  transformable.setScale(1.f, 1.f);
  statesHotspot.transform *= transformable.getTransform();

  drawHotspot(target, statesHotspot);
  drawDebugHotspot(target, statesHotspot);
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

void Object::setPop(int count) {
  pImpl->_popElapsed = sf::seconds(0);
  pImpl->_pop = count;
}

int Object::getPop() const { return pImpl->_pop; }

float Object::getPopScale() const {
  return 0.5f + 0.5f*sinf(static_cast<float>(-M_PI_2 + pImpl->_popElapsed.asSeconds()*4*M_PI));
}

void Object::drawHotspot(sf::RenderTarget &target, sf::RenderStates states) const {
  if (!isTouchable())
    return;

  const auto showHotspot =
      Locator<Preferences>::get().getTempPreference(TempPreferenceNames::ShowHotspot,
                                                    TempPreferenceDefaultValues::ShowHotspot);
  if (!showHotspot)
    return;

  auto &gameSheet = Locator<ResourceManager>::get().getSpriteSheet("GameSheet");
  sf::Sprite s(gameSheet.getTexture(), gameSheet.getRect("hotspot_marker"));
  s.setColor(sf::Color(255, 165, 0));
  s.setScale(0.25f, 0.25f);
  s.setOrigin(15.f, 15.f);
  target.draw(s, states);
}

std::wostream &operator<<(std::wostream &os, const Object &obj) {
  return os << towstring(obj.getName()) << L" (" << obj.getRealPosition().x << L"," << obj.getRealPosition().y << L":"
            << obj.getZOrder() << L")";
}

} // namespace ng
