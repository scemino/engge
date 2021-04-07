#ifdef _WIN32
// for Windows you'll need this to have M_PI_2 defined
#define _USE_MATH_DEFINES
#include <cmath>
#endif
#include <engge/Entities/Object.hpp>
#include <engge/Engine/Function.hpp>
#include <engge/System/Locator.hpp>
#include <engge/Engine/EntityManager.hpp>
#include <engge/Room/Room.hpp>
#include <engge/Graphics/Screen.hpp>
#include <engge/Scripting/ScriptEngine.hpp>
#include <engge/Audio/SoundTrigger.hpp>
#include <engge/Engine/Trigger.hpp>
#include <engge/Engine/Preferences.hpp>
#include "Util/Util.hpp"
#include <sstream>
#include <ngf/Graphics/Sprite.h>
#include <engge/Graphics/AnimDrawable.hpp>

namespace ng {
struct Object::Impl {
  std::vector<Animation> anims;
  Animation *pAnim{nullptr};
  std::wstring name;
  int zorder{0};
  ObjectType type{ObjectType::Object};
  ngf::irect hotspot;
  Room *pRoom{nullptr};
  int state{0};
  std::optional<std::shared_ptr<Trigger>> trigger;
  HSQOBJECT table{};
  bool hotspotVisible{false};
  bool triggerEnabled{true};
  Object *pParentObject{nullptr};
  int dependentState{0};
  Actor *owner{nullptr};
  int fps{0};
  std::vector<std::string> icons;
  ngf::TimeSpan elapsed;
  ngf::TimeSpan popElapsed;
  int iconIndex{0};
  ScreenSpace screenSpace{ScreenSpace::Room};
  bool temporary{false};
  bool jiggle{false};
  int pop{0};
  AnimControl animControl;

  Impl() {
    auto v = ScriptEngine::getVm();
    sq_resetobject(&table);
    sq_newtable(v);
    sq_getstackobj(v, -1, &table);
    sq_addref(v, &table);
    sq_pop(v, 1);
  }

  explicit Impl(HSQOBJECT obj) {
    auto v = ScriptEngine::getVm();
    sq_pushobject(v, obj);
    sq_getstackobj(v, -1, &table);
    sq_addref(v, &table);
    sq_pop(v, 1);
  }

  ~Impl() {
    auto v = ScriptEngine::getVm();
    sq_release(v, &table);
  }
};

Object::Object() : pImpl(std::make_unique<Impl>()) {
  m_id = Locator<EntityManager>::get().getObjectId();
  ScriptEngine::set(this, "_id", m_id);
}

Object::Object(HSQOBJECT obj) : pImpl(std::make_unique<Impl>(obj)) {
  m_id = Locator<EntityManager>::get().getObjectId();
  ScriptEngine::set(this, "_id", m_id);
}

Object::~Object() = default;

void Object::setZOrder(int zorder) { pImpl->zorder = zorder; }

int Object::getZOrder() const { return pImpl->zorder; }

void Object::setType(ObjectType type) { pImpl->type = type; }
ObjectType Object::getType() const { return pImpl->type; }

void Object::setHotspot(const ngf::irect &hotspot) { pImpl->hotspot = hotspot; }
ngf::irect Object::getHotspot() const { return pImpl->hotspot; }

void Object::setIcon(const std::string &icon) {
  pImpl->icons.clear();
  pImpl->fps = 0;
  pImpl->iconIndex = 0;
  pImpl->elapsed = ngf::TimeSpan::seconds(0);
  pImpl->icons.push_back(icon);
}

std::string Object::getIcon() const {
  if (!pImpl->icons.empty())
    return pImpl->icons.at(pImpl->iconIndex);

  auto v = ScriptEngine::getVm();
  auto top = sq_gettop(v);
  sq_pushobject(v, getTable());
  sq_pushstring(v, _SC("icon"), -1);
  if (SQ_SUCCEEDED(sq_rawget(v, -2))) {
    if (sq_gettype(v, -1) == OT_STRING) {
      const SQChar *icon = nullptr;
      sq_getstring(v, -1, &icon);
      pImpl->icons.emplace_back(icon);
    } else if (sq_gettype(v, -1) == OT_ARRAY) {
      SQInteger fps = 0;
      pImpl->iconIndex = 0;
      const SQChar *icon = nullptr;
      sq_pushnull(v); // null iterator
      if (SQ_SUCCEEDED(sq_next(v, -2))) {
        sq_getinteger(v, -1, &fps);
        sq_pop(v, 2);
        pImpl->fps = static_cast<int>(fps);
      }
      while (SQ_SUCCEEDED(sq_next(v, -2))) {
        sq_getstring(v, -1, &icon);
        pImpl->icons.emplace_back(icon);
        sq_pop(v, 2);
      }
      sq_pop(v, 1); // pops the null iterator
    }
  }
  sq_settop(v, top);
  return pImpl->icons.at(pImpl->iconIndex);
}

void Object::setIcon(int fps, const std::vector<std::string> &icons) {
  pImpl->icons.clear();
  pImpl->fps = fps;
  pImpl->iconIndex = 0;
  pImpl->elapsed = ngf::TimeSpan::seconds(0);
  std::copy(icons.begin(), icons.end(), std::back_inserter(pImpl->icons));
}

void Object::setOwner(Actor *pActor) { pImpl->owner = pActor; }
Actor *Object::getOwner() const { return pImpl->owner; }

HSQOBJECT &Object::getTable() { return pImpl->table; }
HSQOBJECT &Object::getTable() const { return pImpl->table; }
bool Object::isInventoryObject() const { return getOwner() != nullptr; }

std::vector<Animation> &Object::getAnims() { return pImpl->anims; }

Room *Object::getRoom() { return pImpl->pRoom; }
const Room *Object::getRoom() const { return pImpl->pRoom; }
void Object::setRoom(Room *pRoom) { pImpl->pRoom = pRoom; }

void Object::addTrigger(const std::shared_ptr<Trigger> &trigger) { pImpl->trigger = trigger; }

void Object::removeTrigger() {
  if (pImpl->trigger.has_value()) {
    (*pImpl->trigger)->disable();
  }
}

Trigger *Object::getTrigger() { return pImpl->trigger.has_value() ? (*pImpl->trigger).get() : nullptr; }
void Object::enableTrigger(bool enabled) { pImpl->triggerEnabled = enabled; }

bool Object::isTouchable() const {
  if (getType() != ObjectType::Object)
    return false;
  if (pImpl->state == ObjectStateConstants::GONE)
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

void Object::setStateAnimIndex(int animIndex) {
  pImpl->state = animIndex;
  std::string name = "state" + std::to_string(animIndex);
  auto it = std::find_if(pImpl->anims.rbegin(), pImpl->anims.rend(), [&name](const auto &anim) {
    return anim.name == name;
  });
  if (it == pImpl->anims.rend()) {
    pImpl->pAnim = nullptr;
    pImpl->animControl.setAnimation(nullptr);
    return;
  }

  pImpl->pAnim = it.operator->();
  pImpl->animControl.setAnimation(pImpl->pAnim);
}

void Object::playAnim(const std::string &anim, bool loop) {
  setAnimation(anim);
  pImpl->animControl.play(loop);
}

void Object::playAnim(int animIndex, bool loop) {
  setStateAnimIndex(animIndex);
  pImpl->animControl.play(loop);
}

int Object::getState() const { return pImpl->state; }

void Object::setAnimation(const std::string &name) {
  auto it = std::find_if(pImpl->anims.begin(), pImpl->anims.end(),
                         [name](auto &animation) { return animation.name == name; });
  if (it == pImpl->anims.end()) {
    pImpl->pAnim = nullptr;
    pImpl->animControl.setAnimation(nullptr);
    return;
  }

  auto &anim = *it;
  pImpl->pAnim = &anim;
  pImpl->animControl.setAnimation(&anim);
}

Animation *Object::getAnimation() { return pImpl->pAnim; }
const Animation *Object::getAnimation() const { return pImpl->pAnim; }

AnimControl &Object::getAnimControl() { return pImpl->animControl; }

void Object::update(const ngf::TimeSpan &elapsed) {
  if (isInventoryObject()) {
    if (pImpl->pop > 0) {
      pImpl->popElapsed += elapsed;
      if (pImpl->popElapsed.getTotalSeconds() > 0.5f) {
        pImpl->pop--;
        pImpl->popElapsed -= ngf::TimeSpan::seconds(0.5);
      }
    }
    if (pImpl->fps == 0)
      return;
    pImpl->elapsed += elapsed;
    if (pImpl->elapsed.getTotalSeconds() > (1.f / static_cast<float>(pImpl->fps))) {
      pImpl->elapsed = ngf::TimeSpan::seconds(0);
      pImpl->iconIndex = static_cast<int>((pImpl->iconIndex + 1) % pImpl->icons.size());
    }
    return;
  }

  Entity::update(elapsed);
  if (pImpl->pParentObject) {
    setVisible(pImpl->pParentObject->getState() == pImpl->dependentState);
  }
  pImpl->animControl.update(elapsed);
  if (pImpl->triggerEnabled && pImpl->trigger.has_value()) {
    (*pImpl->trigger)->trig();
  }
}

void Object::showDebugHotspot(bool show) { pImpl->hotspotVisible = show; }

bool Object::isHotspotVisible() const { return pImpl->hotspotVisible; }

void Object::setScreenSpace(ScreenSpace screenSpace) { pImpl->screenSpace = screenSpace; }

ScreenSpace Object::getScreenSpace() const { return pImpl->screenSpace; }

void Object::draw(ngf::RenderTarget &target, ngf::RenderStates states) const {
  if (!isVisible())
    return;

  if (pImpl->screenSpace == ScreenSpace::Object)
    return;

  ngf::RenderStates initialStates = states;
  ngf::Transform t = getTransform();

  if (pImpl->pAnim) {
    auto pos = t.getPosition();
    auto scale = getScale();
    t.setPosition({pos.x, pImpl->pRoom->getScreenSize().y - pos.y - scale * getRenderOffset().y});
    states.transform = t.getTransform() * states.transform;

    AnimDrawable animDrawable;
    animDrawable.setAnim(pImpl->pAnim);
    animDrawable.setColor(getColor());
    animDrawable.draw(pos, target, states);
  }

  initialStates.transform = t.getTransform() * initialStates.transform;

  for (const auto *pChild : getChildren()) {
    pChild->draw(target, initialStates);
  }
}

void Object::dependentOn(Object *parentObject, int state) {
  pImpl->dependentState = state;
  pImpl->pParentObject = parentObject;
}

void Object::setFps(int fps) {
  if (pImpl->pAnim) {
    pImpl->pAnim->fps = fps;
  }
}

void Object::setTemporary(bool isTemporary) { pImpl->temporary = isTemporary; }

bool Object::isTemporary() const { return pImpl->temporary; }

void Object::setJiggle(bool enabled) { pImpl->jiggle = enabled; }

bool Object::getJiggle() const { return pImpl->jiggle; }

void Object::setPop(int count) {
  pImpl->popElapsed = ngf::TimeSpan::seconds(0);
  pImpl->pop = count;
}

int Object::getPop() const { return pImpl->pop; }

float Object::getPopScale() const {
  return 0.5f + 0.5f * sinf(static_cast<float>(-M_PI_2 + pImpl->popElapsed.getTotalSeconds() * 4 * M_PI));
}

std::wostream &operator<<(std::wostream &os, const Object &obj) {
  return os << towstring(obj.getName()) << L" (" << obj.getPosition().x << L"," << obj.getPosition().y << L":"
            << obj.getZOrder() << L")";
}

} // namespace ng
