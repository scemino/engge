#include <engge/Entities/Actor.hpp>
#include <engge/Engine/Engine.hpp>
#include <engge/System/Locator.hpp>
#include <engge/System/Logger.hpp>
#include <engge/Engine/EntityManager.hpp>
#include <engge/Entities/Costume.hpp>
#include <engge/Entities/Object.hpp>
#include <engge/Room/Room.hpp>
#include <engge/Room/RoomScaling.hpp>
#include <engge/Scripting/ScriptEngine.hpp>
#include <glm/vec2.hpp>
#include <ngf/Graphics/RectangleShape.h>
#include "Graphics/PathDrawable.hpp"
#include "WalkingState.hpp"

namespace ng {

namespace {
bool frameContains(const SpriteSheetItem &frame, const glm::vec2 &pos) {
  ngf::Transform t;
  t.setOrigin(frame.sourceSize / 2);
  t.setPosition(frame.spriteSourceSize.getTopLeft());

  auto rect = ngf::frect::fromPositionSize({0, 0}, frame.frame.getSize());
  auto r = ngf::transform(t.getTransform(), rect);
  return r.contains(pos);
}

bool animContains(const Animation &anim, const glm::vec2 &pos) {
  if (!anim.frames.empty() && frameContains(anim.frames[anim.frameIndex], pos))
    return true;

  return std::any_of(anim.layers.cbegin(), anim.layers.cend(), [pos](const auto &layer) {
    if (!layer.visible)
      return false;
    return animContains(layer, pos);
  });
}
}

struct Actor::Impl {
  explicit Impl(Engine &engine)
      : _engine(engine), _costume(engine.getResourceManager()) {
  }

  void setActor(Actor *pActor) {
    _pActor = pActor;
    _walkingState.setActor(pActor);
    _costume.setActor(pActor);
  }

  void drawHotspot(ngf::RenderTarget &target, ngf::RenderStates states) const {
    if (!_hotspotVisible)
      return;

    auto rect = _pActor->getHotspot();

    ngf::RectangleShape s(rect.getSize());
    s.getTransform().setPosition(rect.getTopLeft());
    s.setOutlineThickness(1);
    s.setOutlineColor(ngf::Colors::Red);
    s.setColor(ngf::Colors::Transparent);
    s.draw(target, states);

    // draw actor position
    ngf::RectangleShape rectangle;
    rectangle.setColor(ngf::Colors::Red);
    rectangle.setSize(glm::vec2(2, 2));
    rectangle.getTransform().setOrigin(glm::vec2(1, 1));
    rectangle.draw(target, states);
  }

  Engine &_engine;
  Actor *_pActor{nullptr};
  Costume _costume;
  bool _useWalkboxes{true};
  Room *_pRoom{nullptr};
  ngf::irect _hotspot{};
  std::vector<Object *> _objects;
  WalkingState _walkingState;
  glm::ivec2 _speed{30, 15};
  std::optional<float> _volume;
  std::shared_ptr<PathDrawable> _path;
  HSQOBJECT _table{};
  bool _hotspotVisible{false};
  int _inventoryOffset{0};
  int _fps{10};
};

std::wstring Actor::getTranslatedName() const {
  return Engine::getText(getName());
}

std::string Actor::getIcon() const {
  const char *icon = nullptr;
  ScriptEngine::rawGet(pImpl->_table, "icon", icon);
  if (!icon)
    return "";
  return icon;
}

void Actor::useWalkboxes(bool useWalkboxes) { pImpl->_useWalkboxes = useWalkboxes; }

bool Actor::useWalkboxes() const { return pImpl->_useWalkboxes; }

Costume &Actor::getCostume() { return pImpl->_costume; }

Costume &Actor::getCostume() const { return pImpl->_costume; }

Room *Actor::getRoom() { return pImpl->_pRoom; }

void Actor::setHotspot(const ngf::irect &hotspot) { pImpl->_hotspot = hotspot; }

ngf::irect Actor::getHotspot() const { return pImpl->_hotspot; }

void Actor::showHotspot(bool show) { pImpl->_hotspotVisible = show; }

bool Actor::isHotspotVisible() const { return pImpl->_hotspotVisible; }

bool Actor::contains(const glm::vec2 &pos) const {
  auto pAnim = pImpl->_costume.getAnimation();
  if (!pAnim)
    return false;

  auto scale = getScale();
  auto transformable = getTransform();
  transformable.setScale({scale, scale});
  transformable.move({getRenderOffset().x * scale, getRenderOffset().y * scale});
  auto t = glm::inverse(transformable.getTransform());
  auto pos2 = ngf::transform(t, pos);
  return animContains(*pAnim, pos2);
}

void Actor::pickupObject(Object *pObject) {
  pObject->setOwner(this);
  pImpl->_objects.push_back(pObject);

  if (ScriptEngine::rawExists(pObject, "onPickup")) {
    ScriptEngine::objCall(pObject, "onPickup", this);
  }
}

void Actor::pickupReplacementObject(Object *pObject1, Object *pObject2) {
  pObject2->setOwner(this);
  std::replace(pImpl->_objects.begin(), pImpl->_objects.end(), pObject1, pObject2);
  pObject1->setOwner(nullptr);
}

void Actor::giveTo(Object *pObject, Actor *pActor) {
  if (!pObject || !pActor)
    return;
  pObject->setOwner(pActor);
  auto srcIt = std::find(pImpl->_objects.begin(), pImpl->_objects.end(), pObject);
  std::move(srcIt, srcIt + 1, std::inserter(pActor->pImpl->_objects, pActor->pImpl->_objects.end()));
  pImpl->_objects.erase(srcIt);
}

void Actor::removeInventory(Object *pObject) {
  if (!pObject)
    return;
  pObject->setOwner(nullptr);
  pImpl->_objects.erase(std::remove(pImpl->_objects.begin(),
                                    pImpl->_objects.end(),
                                    pObject), pImpl->_objects.end());
}

void Actor::clearInventory() {
  for (auto &&obj : pImpl->_objects) {
    obj->setOwner(nullptr);
  }
  pImpl->_objects.clear();
}

const std::vector<Object *> &Actor::getObjects() const { return pImpl->_objects; }

void Actor::setWalkSpeed(const glm::ivec2 &speed) { pImpl->_speed = speed; }

const glm::ivec2 &Actor::getWalkSpeed() const { return pImpl->_speed; }

void Actor::stopWalking() { pImpl->_walkingState.stop(); }

bool Actor::isWalking() const { return pImpl->_walkingState.isWalking(); }

void Actor::setVolume(float volume) { pImpl->_volume = volume; }
std::optional<float> Actor::getVolume() const { return pImpl->_volume; }

HSQOBJECT &Actor::getTable() { return pImpl->_table; }
HSQOBJECT &Actor::getTable() const { return pImpl->_table; }

bool Actor::isInventoryObject() const { return false; }

Actor::Actor(Engine &engine) : pImpl(std::make_unique<Impl>(engine)) {
  pImpl->setActor(this);
  _id = Locator<EntityManager>::get().getActorId();
}

Actor::~Actor() = default;

const Room *Actor::getRoom() const { return pImpl->_pRoom; }

int Actor::getZOrder() const { return static_cast<int>(getPosition().y); }

void Actor::setRoom(Room *pRoom) {
  if (pImpl->_pRoom) {
    pImpl->_pRoom->removeEntity(this);
  }
  pImpl->_pRoom = pRoom;
  pImpl->_pRoom->setAsParallaxLayer(this, 0);
}

void Actor::setCostume(const std::string &name, const std::string &sheet) {
  std::string path;
  path.append(name).append(".json");
  pImpl->_costume.loadCostume(path, sheet);
}

float Actor::getScale() const {
  auto pRoom = pImpl->_pRoom;
  if (!pRoom)
    return 1.f;
  return pRoom->getRoomScaling().getScaling(getPosition().y);
}

void Actor::draw(ngf::RenderTarget &target, ngf::RenderStates states) const {
  if (!isVisible())
    return;

  auto scale = getScale();
  auto transformable = getTransform();
  transformable.setScale({scale, scale});
  transformable.setPosition({transformable.getPosition().x + scale * getRenderOffset().x,
                             pImpl->_pRoom->getScreenSize().y - transformable.getPosition().y
                                 - scale * getRenderOffset().y});
  states.transform = transformable.getTransform() * states.transform;
  pImpl->_costume.draw(target, states);
}

void Actor::drawForeground(ngf::RenderTarget &target, ngf::RenderStates states) const {
  Entity::drawForeground(target, states);
  if (pImpl->_path && pImpl->_pRoom && pImpl->_engine.getWalkboxesFlags()) {
    pImpl->_path->draw(target, states);
  }

  auto scale = getScale();
  auto transformable = getTransform();
  transformable.setScale({scale, scale});
  transformable.setPosition({transformable.getPosition().x,
                             pImpl->_pRoom->getScreenSize().y - transformable.getPosition().y});
  states.transform = transformable.getTransform() * states.transform;
  pImpl->drawHotspot(target, states);
}

void Actor::update(const ngf::TimeSpan &elapsed) {
  Entity::update(elapsed);

  pImpl->_costume.update(elapsed);
  pImpl->_walkingState.update(elapsed);
}

std::vector<glm::vec2> Actor::walkTo(const glm::vec2 &destination, std::optional<Facing> facing) {
  if (pImpl->_pRoom == nullptr)
    return {getPosition()};

  std::vector<glm::vec2> path;
  if (pImpl->_useWalkboxes) {
    path = pImpl->_pRoom->calculatePath(getPosition(), destination);
    if (path.size() < 2) {
      pImpl->_path = nullptr;
      return path;
    }
  } else {
    path.push_back(getPosition());
    path.push_back(destination);
  }

  pImpl->_path = std::make_unique<PathDrawable>(path);
  if (ScriptEngine::rawExists(this, "preWalking")) {
    ScriptEngine::rawCall(this, "preWalking");
  }
  pImpl->_walkingState.setDestination(path, facing);
  return path;
}

void Actor::setFps(int fps) {
  pImpl->_fps = fps;
}

int Actor::getFps() const {
  return pImpl->_fps;
}

void Actor::setInventoryOffset(int offset) { pImpl->_inventoryOffset = offset; }

int Actor::getInventoryOffset() const {
  if ((pImpl->_inventoryOffset * 4) > static_cast<int>(getObjects().size())) {
    pImpl->_inventoryOffset = 0;
  }
  return pImpl->_inventoryOffset;
}

} // namespace ng
