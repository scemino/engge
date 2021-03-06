#include <engge/Entities/Actor.hpp>
#include <engge/Engine/Engine.hpp>
#include <engge/System/Locator.hpp>
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
  if (!anim.frames.empty() && frameContains(anim.frames.at(anim.frameIndex), pos))
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

  Engine &_engine;
  Actor *_pActor{nullptr};
  Costume _costume;
  bool _useWalkboxes{true};
  Room *_pRoom{nullptr};
  ngf::irect _hotspot{};
  std::vector<Object *> _objects;
  WalkingState _walkingState;
  glm::ivec2 _speed{30, 15};
  std::unique_ptr<PathDrawable> _path;
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
  ScriptEngine::rawGet(m_pImpl->_table, "icon", icon);
  if (!icon)
    return "";
  return icon;
}

void Actor::useWalkboxes(bool useWalkboxes) { m_pImpl->_useWalkboxes = useWalkboxes; }

bool Actor::useWalkboxes() const { return m_pImpl->_useWalkboxes; }

std::unique_ptr<PathDrawable> Actor::getPath() { return std::move(m_pImpl->_path); }

Costume &Actor::getCostume() { return m_pImpl->_costume; }

Costume &Actor::getCostume() const { return m_pImpl->_costume; }

Room *Actor::getRoom() { return m_pImpl->_pRoom; }

void Actor::setHotspot(const ngf::irect &hotspot) { m_pImpl->_hotspot = hotspot; }

ngf::irect Actor::getHotspot() const { return m_pImpl->_hotspot; }

void Actor::showHotspot(bool show) { m_pImpl->_hotspotVisible = show; }

bool Actor::isHotspotVisible() const { return m_pImpl->_hotspotVisible; }

bool Actor::contains(const glm::vec2 &pos) const {
  auto pAnim = m_pImpl->_costume.getAnimation();
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
  m_pImpl->_objects.push_back(pObject);

  ScriptEngine::call("onPickup", pObject, this);

  if (ScriptEngine::rawExists(pObject, "onPickUp")) {
    ScriptEngine::rawCall(pObject, "onPickUp", this);
  }
}

void Actor::pickupReplacementObject(Object *pObject1, Object *pObject2) {
  pObject2->setOwner(this);
  std::replace(m_pImpl->_objects.begin(), m_pImpl->_objects.end(), pObject1, pObject2);
  pObject1->setOwner(nullptr);
}

void Actor::giveTo(Object *pObject, Actor *pActor) {
  if (!pObject || !pActor)
    return;
  pObject->setOwner(pActor);
  auto srcIt = std::find(m_pImpl->_objects.begin(), m_pImpl->_objects.end(), pObject);
  std::move(srcIt, srcIt + 1, std::inserter(pActor->m_pImpl->_objects, pActor->m_pImpl->_objects.end()));
  m_pImpl->_objects.erase(srcIt);
}

void Actor::removeInventory(Object *pObject) {
  if (!pObject)
    return;
  pObject->setOwner(nullptr);
  m_pImpl->_objects.erase(std::remove(m_pImpl->_objects.begin(),
                                      m_pImpl->_objects.end(),
                                      pObject), m_pImpl->_objects.end());
}

void Actor::clearInventory() {
  for (auto &&obj : m_pImpl->_objects) {
    obj->setOwner(nullptr);
  }
  m_pImpl->_objects.clear();
}

const std::vector<Object *> &Actor::getObjects() const { return m_pImpl->_objects; }

void Actor::setWalkSpeed(const glm::ivec2 &speed) { m_pImpl->_speed = speed; }

const glm::ivec2 &Actor::getWalkSpeed() const { return m_pImpl->_speed; }

void Actor::stopWalking() { m_pImpl->_walkingState.stop(); }

bool Actor::isWalking() const { return m_pImpl->_walkingState.isWalking(); }

HSQOBJECT &Actor::getTable() { return m_pImpl->_table; }
HSQOBJECT &Actor::getTable() const { return m_pImpl->_table; }

bool Actor::isInventoryObject() const { return false; }

Actor::Actor(Engine &engine) : m_pImpl(std::make_unique<Impl>(engine)) {
  m_pImpl->setActor(this);
  m_id = Locator<EntityManager>::get().getActorId();
}

Actor::~Actor() = default;

const Room *Actor::getRoom() const { return m_pImpl->_pRoom; }

int Actor::getZOrder() const { return static_cast<int>(getPosition().y); }

void Actor::setRoom(Room *pRoom) {
  if (m_pImpl->_pRoom) {
    m_pImpl->_pRoom->removeEntity(this);
  }
  m_pImpl->_pRoom = pRoom;
  m_pImpl->_pRoom->setAsParallaxLayer(this, 0);
}

void Actor::setCostume(const std::string &name, const std::string &sheet) {
  std::string path;
  path.append(name).append(".json");
  m_pImpl->_costume.loadCostume(path, sheet);
}

float Actor::getScale() const {
  auto pRoom = m_pImpl->_pRoom;
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
                             m_pImpl->_pRoom->getScreenSize().y - transformable.getPosition().y
                                 - scale * getRenderOffset().y});
  states.transform = transformable.getTransform() * states.transform;
  m_pImpl->_costume.draw(target, states);
}

void Actor::update(const ngf::TimeSpan &elapsed) {
  Entity::update(elapsed);

  m_pImpl->_costume.update(elapsed);
  m_pImpl->_walkingState.update(elapsed);
}

std::vector<glm::vec2> Actor::walkTo(const glm::vec2 &destination, std::optional<Facing> facing) {
  if (m_pImpl->_pRoom == nullptr)
    return {getPosition()};

  std::vector<glm::vec2> path;
  if (m_pImpl->_useWalkboxes) {
    path = m_pImpl->_pRoom->calculatePath(getPosition(), destination);
    if (path.size() < 2) {
      m_pImpl->_path = nullptr;
      return path;
    }
  } else {
    path.push_back(getPosition());
    path.push_back(destination);
  }

  m_pImpl->_path = std::make_unique<PathDrawable>(path);
  if (ScriptEngine::rawExists(this, "preWalking")) {
    ScriptEngine::rawCall(this, "preWalking");
  }
  m_pImpl->_walkingState.setDestination(path, facing);
  return path;
}

void Actor::setFps(int fps) {
  m_pImpl->_fps = fps;
}

int Actor::getFps() const {
  return m_pImpl->_fps;
}

void Actor::setInventoryOffset(int offset) { m_pImpl->_inventoryOffset = offset; }

int Actor::getInventoryOffset() const {
  if ((m_pImpl->_inventoryOffset * 4) > static_cast<int>(getObjects().size())) {
    m_pImpl->_inventoryOffset = 0;
  }
  return m_pImpl->_inventoryOffset;
}

} // namespace ng
