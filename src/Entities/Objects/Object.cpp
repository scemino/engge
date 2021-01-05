#define _USE_MATH_DEFINES
#include <cmath>
#include <engge/Entities/Objects/Object.hpp>
#include <engge/Engine/Function.hpp>
#include <engge/System/Locator.hpp>
#include <engge/Engine/EntityManager.hpp>
#include <engge/Room/Room.hpp>
#include <engge/Graphics/Screen.hpp>
#include <engge/Scripting/ScriptEngine.hpp>
#include <engge/Audio/SoundTrigger.hpp>
#include <engge/Engine/Trigger.hpp>
#include <engge/Engine/Preferences.hpp>
#include "../../System/_Util.hpp"
#include <sstream>
#include <ngf/Graphics/RectangleShape.h>
#include <ngf/Graphics/Sprite.h>
#include <engge/Entities/Objects/AnimDrawable.hpp>

namespace ng {

struct Object::Impl {
  std::vector<ObjectAnimation> _anims;
  ObjectAnimation *_pAnim{nullptr};
  std::wstring _name;
  int _zorder{0};
  ObjectType _type{ObjectType::Object};
  ngf::irect _hotspot;
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
  ngf::TimeSpan _elapsed;
  ngf::TimeSpan _popElapsed;
  int _index{0};
  ScreenSpace _screenSpace{ScreenSpace::Room};
  std::vector<Object *> _children;
  bool _temporary{false};
  bool _jiggle{false};
  int _pop{0};
  Object *_pParent{nullptr};
  const ngf::Texture *_texture{nullptr};
  AnimControl _animControl;

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

void Object::setHotspot(const ngf::irect &hotspot) { pImpl->_hotspot = hotspot; }
const ngf::irect &Object::getHotspot() const { return pImpl->_hotspot; }

void Object::setIcon(const std::string &icon) {
  pImpl->_icons.clear();
  pImpl->_fps = 0;
  pImpl->_index = 0;
  pImpl->_elapsed = ngf::TimeSpan::seconds(0);
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
  pImpl->_elapsed = ngf::TimeSpan::seconds(0);
  std::copy(icons.begin(), icons.end(), std::back_inserter(pImpl->_icons));
}

void Object::setOwner(Actor *pActor) { pImpl->_owner = pActor; }
Actor *Object::getOwner() const { return pImpl->_owner; }

HSQOBJECT &Object::getTable() { return pImpl->_table; }
HSQOBJECT &Object::getTable() const { return pImpl->_table; }
bool Object::isInventoryObject() const { return getOwner() != nullptr; }

void Object::setTexture(const ngf::Texture *texture) {
  pImpl->_texture = texture;
}

std::vector<ObjectAnimation> &Object::getAnims() { return pImpl->_anims; }

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

ngf::irect Object::getRealHotspot() const {
  auto rect = getHotspot();
  auto rectf = ngf::frect::fromPositionSize(rect.getPosition(), rect.getSize());
  auto transform = getTransform().getTransform();
  auto result = ngf::transform(transform, rectf);
  return ngf::irect::fromPositionSize(result.getPosition(), result.getSize());
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
  pImpl->_animControl.play(loop);
}

void Object::playAnim(int animIndex, bool loop) {
  setStateAnimIndex(animIndex);
  pImpl->_animControl.play(loop);
}

int Object::getState() const { return pImpl->_state; }

void Object::setAnimation(const std::string &name) {
  auto it = std::find_if(pImpl->_anims.begin(), pImpl->_anims.end(),
                         [name](auto &animation) { return animation.name == name; });
  if (it == pImpl->_anims.end()) {
    pImpl->_pAnim = nullptr;
    pImpl->_animControl.setAnimation(nullptr);
    return;
  }

  auto &anim = *it;
  pImpl->_pAnim = &anim;
  pImpl->_animControl.setAnimation(&anim);
}

ObjectAnimation *&Object::getAnimation() { return pImpl->_pAnim; }

AnimControl& Object::getAnimControl() { return pImpl->_animControl; }

void Object::update(const ngf::TimeSpan &elapsed) {
  if (isInventoryObject()) {
    if (pImpl->_pop > 0) {
      pImpl->_popElapsed += elapsed;
      if (pImpl->_popElapsed.getTotalSeconds() > 0.5f) {
        pImpl->_pop--;
        pImpl->_popElapsed -= ngf::TimeSpan::seconds(0.5);
      }
    }
    if (pImpl->_fps == 0)
      return;
    pImpl->_elapsed += elapsed;
    if (pImpl->_elapsed.getTotalSeconds() > (1.f / static_cast<float>(pImpl->_fps))) {
      pImpl->_elapsed = ngf::TimeSpan::seconds(0);
      pImpl->_index = static_cast<int>((pImpl->_index + 1) % pImpl->_icons.size());
    }
    return;
  }

  Entity::update(elapsed);
  if (pImpl->pParentObject) {
    setVisible(pImpl->pParentObject->getState() == pImpl->dependentState);
  }
  pImpl->_animControl.update(elapsed);
  if (pImpl->_triggerEnabled && pImpl->_trigger.has_value()) {
    (*pImpl->_trigger)->trig();
  }
}

void Object::showDebugHotspot(bool show) { pImpl->_hotspotVisible = show; }

bool Object::isHotspotVisible() const { return pImpl->_hotspotVisible; }

void Object::setScreenSpace(ScreenSpace screenSpace) { pImpl->_screenSpace = screenSpace; }

ScreenSpace Object::getScreenSpace() const { return pImpl->_screenSpace; }

void Object::drawDebugHotspot(ngf::RenderTarget &target, ngf::RenderStates states) const {
  if (!pImpl->_hotspotVisible)
    return;

  auto rect = getHotspot();
  // TODO: y-axis is inverted
  //rect.top = -rect.height - rect.top;

  ngf::Color color;
  switch (getType()) {
  case ObjectType::Object:color = ngf::Colors::Red;
    break;
  case ObjectType::Spot:color = ngf::Colors::Green;
    break;
  case ObjectType::Trigger:color = ngf::Colors::Magenta;
    break;
  case ObjectType::Prop:color = ngf::Colors::Blue;
    break;
  }

  ngf::RectangleShape s(rect.getSize());
  s.getTransform().setPosition(rect.getTopLeft());
  s.setOutlineThickness(1);
  s.setOutlineColor(color);
  s.setColor(ngf::Colors::Transparent);
  s.draw(target, states);

  auto usePos = getUsePosition().value_or(glm::vec2());
  usePos.y = -usePos.y;
  ngf::RectangleShape vl(glm::vec2(1, 7));
  vl.getTransform().setPosition({usePos.x, usePos.y - 3});
  vl.setColor(color);
  vl.draw(target, states);

  ngf::RectangleShape hl(glm::vec2(7, 1));
  hl.getTransform().setPosition({usePos.x - 3, usePos.y});
  hl.setColor(color);
  hl.draw(target, states);

  auto useDir = getUseDirection().value_or(UseDirection::Front);
  switch (useDir) {
  case UseDirection::Front: {
    ngf::RectangleShape dirShape(glm::vec2(3, 1));
    dirShape.getTransform().setPosition({usePos.x - 1, usePos.y + 2});
    dirShape.setColor(color);
    dirShape.draw(target, states);
  }
    break;
  case UseDirection::Back: {
    ngf::RectangleShape dirShape(glm::vec2(3, 1));
    dirShape.getTransform().setPosition({usePos.x - 1, usePos.y - 2});
    dirShape.setColor(color);
    dirShape.draw(target, states);
  }
    break;
  case UseDirection::Left: {
    ngf::RectangleShape dirShape(glm::vec2(1, 3));
    dirShape.getTransform().setPosition({usePos.x - 2, usePos.y - 1});
    dirShape.setColor(color);
    dirShape.draw(target, states);
  }
    break;
  case UseDirection::Right: {
    ngf::RectangleShape dirShape(glm::vec2(1, 3));
    dirShape.getTransform().setPosition({usePos.x + 2, usePos.y - 1});
    dirShape.setColor(color);
    dirShape.draw(target, states);
  }
    break;
  }
}

void Object::drawForeground(ngf::RenderTarget &target, ngf::RenderStates states) const {
  Entity::drawForeground(target, states);
  if (pImpl->_screenSpace != ScreenSpace::Object)
    return;

  const auto view = target.getView();
  target.setView(ngf::View(ngf::frect::fromPositionSize({0, 0}, {Screen::Width, Screen::Height})));

  ngf::RenderStates s;
  auto transformable = getTransform();
  transformable.setPosition({transformable.getPosition().x,
                             target.getView().getSize().y - transformable.getPosition().y});
  s.transform *= transformable.getTransform();

  if (pImpl->_pAnim) {
    draw(target, s);
  }

  ngf::RenderStates statesHotspot;
  transformable = getTransform();
  transformable.setPosition({transformable.getPosition().x,
                             target.getView().getSize().y - transformable.getPosition().y});
  transformable.setScale({1.f, 1.f});
  statesHotspot.transform *= transformable.getTransform();

  drawHotspot(target, statesHotspot);
  drawDebugHotspot(target, statesHotspot);

  target.setView(view);
}

void Object::draw(ngf::RenderTarget &target, ngf::RenderStates states) const {
  ngf::RenderStates initialStates = states;
  if (!isVisible())
    return;

  if (pImpl->_screenSpace == ScreenSpace::Object)
    return;

  if (pImpl->_pAnim) {
    ngf::Transform t = getTransform();
    auto pos = t.getPosition();
    t.setPosition({pos.x, pImpl->_pRoom->getScreenSize().y - pos.y});
    states.transform *= t.getTransform();

    AnimDrawable animDrawable;
    animDrawable.setAnim(pImpl->_pAnim);
    animDrawable.setColor(getColor());
    animDrawable.draw(pos, target, states);
  }

  for (const auto &pChild : pImpl->_children) {
    pChild->draw(target, initialStates);
  }

  ngf::RenderStates statesHotspot = initialStates;
  auto transformable = getTransform();
  transformable.setPosition({transformable.getPosition().x,
                             target.getView().getSize().y - transformable.getPosition().y});
  transformable.setScale({1.f, 1.f});
  statesHotspot.transform *= transformable.getTransform();

  drawHotspot(target, statesHotspot);
  drawDebugHotspot(target, statesHotspot);
}

void Object::dependentOn(Object *parentObject, int state) {
  pImpl->dependentState = state;
  pImpl->pParentObject = parentObject;
}

void Object::setFps(int fps) {
  if (pImpl->_pAnim) {
    pImpl->_pAnim->fps = fps;
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
  pImpl->_popElapsed = ngf::TimeSpan::seconds(0);
  pImpl->_pop = count;
}

int Object::getPop() const { return pImpl->_pop; }

float Object::getPopScale() const {
  return 0.5f + 0.5f * sinf(static_cast<float>(-M_PI_2 + pImpl->_popElapsed.getTotalSeconds() * 4 * M_PI));
}

void Object::drawHotspot(ngf::RenderTarget &target, ngf::RenderStates states) const {
  if (!isTouchable())
    return;

  const auto showHotspot =
      Locator<Preferences>::get().getTempPreference(TempPreferenceNames::ShowHotspot,
                                                    TempPreferenceDefaultValues::ShowHotspot);
  if (!showHotspot)
    return;

  auto &gameSheet = Locator<ResourceManager>::get().getSpriteSheet("GameSheet");
  ngf::Sprite s(gameSheet.getTexture(), gameSheet.getRect("hotspot_marker"));
  s.setColor(ngf::Color(255, 165, 0));
  s.getTransform().setScale({0.25f, 0.25f});
  s.getTransform().setOrigin({15.f, 15.f});
  s.draw(target, states);
}

std::wostream &operator<<(std::wostream &os, const Object &obj) {
  return os << towstring(obj.getName()) << L" (" << obj.getRealPosition().x << L"," << obj.getRealPosition().y << L":"
            << obj.getZOrder() << L")";
}

} // namespace ng
