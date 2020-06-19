#include "Entities/Actor/Actor.hpp"
#include "Engine/Camera.hpp"
#include "Engine/Engine.hpp"
#include "Parsers/Lip.hpp"
#include "System/Locator.hpp"
#include "System/Logger.hpp"
#include "Math/PathFinding/PathFinder.hpp"
#include "Engine/Preferences.hpp"
#include "Engine/EntityManager.hpp"
#include "Entities/Objects/Object.hpp"
#include "Room/Room.hpp"
#include "Room/RoomScaling.hpp"
#include "Scripting/ScriptEngine.hpp"
#include "Audio/SoundId.hpp"
#include "Audio/SoundManager.hpp"
#include "Audio/SoundTrigger.hpp"
#include "../../Math/PathFinding/_Path.hpp"

namespace ng {
struct Actor::Impl {
  class WalkingState {
  public:
    WalkingState() = default;

    void setActor(Actor *pActor);
    void setDestination(const std::vector<sf::Vector2f> &path, std::optional<Facing> facing);
    void update(const sf::Time &elapsed);
    void stop();
    [[nodiscard]] bool isWalking() const { return _isWalking; }

  private:
    Facing getFacing();

  private:
    Actor *_pActor{nullptr};
    std::vector<sf::Vector2f> _path;
    std::optional<Facing> _facing{Facing::FACE_FRONT};
    bool _isWalking{false};
    sf::Vector2f _init;
    sf::Time _elapsed;
  };

  explicit Impl(Engine &engine)
      : _engine(engine), _costume(engine.getTextureManager()) {
  }

  void setActor(Actor *pActor) {
    _pActor = pActor;
    _walkingState.setActor(pActor);
    _costume.setActor(pActor);
  }

  void drawHotspot(sf::RenderTarget &target, sf::RenderStates states) const {
    if (!_hotspotVisible)
      return;

    auto rect = _pActor->getHotspot();

    sf::RectangleShape s(sf::Vector2f(rect.width, rect.height));
    s.setPosition(rect.left, rect.top);
    s.setOutlineThickness(1);
    s.setOutlineColor(sf::Color::Red);
    s.setFillColor(sf::Color::Transparent);
    target.draw(s, states);

    // draw actor position
    sf::RectangleShape rectangle;
    rectangle.setFillColor(sf::Color::Red);
    rectangle.setSize(sf::Vector2f(2, 2));
    rectangle.setOrigin(sf::Vector2f(1, 1));
    target.draw(rectangle, states);
  }

  Engine &_engine;
  Actor *_pActor{nullptr};
  Costume _costume;
  bool _useWalkboxes{true};
  Room *_pRoom{nullptr};
  sf::IntRect _hotspot;
  std::vector<Object *> _objects;
  WalkingState _walkingState;
  sf::Vector2i _speed{30, 15};
  std::optional<float> _volume;
  std::shared_ptr<_Path> _path;
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
  return icon;
}

void Actor::useWalkboxes(bool useWalkboxes) { pImpl->_useWalkboxes = useWalkboxes; }

Costume &Actor::getCostume() { return pImpl->_costume; }

Costume &Actor::getCostume() const { return pImpl->_costume; }

Room *Actor::getRoom() { return pImpl->_pRoom; }

void Actor::setHotspot(const sf::IntRect &hotspot) { pImpl->_hotspot = hotspot; }

sf::IntRect Actor::getHotspot() const { return pImpl->_hotspot; }

void Actor::showHotspot(bool show) { pImpl->_hotspotVisible = show; }

bool Actor::isHotspotVisible() const { return pImpl->_hotspotVisible; }

bool Actor::contains(const sf::Vector2f &pos) const {
  auto pAnim = pImpl->_costume.getAnimation();
  if (!pAnim)
    return false;

  auto scale = getScale();
  auto transformable = getTransform();
  transformable.scale(scale, scale);
  transformable.move(getRenderOffset().x * scale, getRenderOffset().y * scale);
  auto t = transformable.getInverseTransform();
  auto pos2 = t.transformPoint(pos);
  return pAnim->contains(pos2);
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

void Actor::setWalkSpeed(const sf::Vector2i &speed) { pImpl->_speed = speed; }

const sf::Vector2i &Actor::getWalkSpeed() const { return pImpl->_speed; }

void Actor::stopWalking() { pImpl->_walkingState.stop(); }

bool Actor::isWalking() const { return pImpl->_walkingState.isWalking(); }

void Actor::setVolume(float volume) { pImpl->_volume = volume; }
std::optional<float> Actor::getVolume() const { return pImpl->_volume; }

HSQOBJECT &Actor::getTable() { return pImpl->_table; }
HSQOBJECT &Actor::getTable() const { return pImpl->_table; }

bool Actor::isInventoryObject() const { return false; }

void Actor::Impl::WalkingState::setActor(Actor *pActor) { _pActor = pActor; }

void Actor::Impl::WalkingState::setDestination(const std::vector<sf::Vector2f> &path, std::optional<Facing> facing) {
  _path = path;
  _facing = facing;
  _path.erase(_path.begin());
  _pActor->getCostume().setFacing(getFacing());
  _pActor->getCostume().setWalkState();
  _isWalking = true;
  _init = _pActor->getRealPosition();
  _elapsed = sf::seconds(0);
  trace("{} go to : {},{}", _pActor->getName(), _path[0].x, _path[0].y);
}

void Actor::Impl::WalkingState::stop() {
  _isWalking = false;
  _pActor->getCostume().setStandState();
  if (ScriptEngine::rawExists(_pActor, "postWalking")) {
    ScriptEngine::objCall(_pActor, "postWalking");
  }
}

Facing Actor::Impl::WalkingState::getFacing() {
  auto pos = _pActor->getRealPosition();
  auto dx = _path[0].x - pos.x;
  auto dy = _path[0].y - pos.y;
  if (fabs(dx) > fabs(dy))
    return (dx > 0) ? Facing::FACE_RIGHT : Facing::FACE_LEFT;
  return (dy < 0) ? Facing::FACE_FRONT : Facing::FACE_BACK;
}

void Actor::Impl::WalkingState::update(const sf::Time &elapsed) {
  if (!_isWalking)
    return;

  _elapsed += elapsed;
  auto delta = (_path[0] - _init);
  auto vSpeed = _pActor->getWalkSpeed();
  sf::Vector2f vDuration;
  vDuration.x = std::abs(delta.x) / vSpeed.x;
  vDuration.y = std::abs(delta.y) / vSpeed.y;
  auto maxDuration = std::max(vDuration.x, vDuration.y);
  auto factor = (2.f * _elapsed.asSeconds()) / maxDuration;
  auto end = factor >= 1.f;
  auto newPos = end ? _path[0] : (_init + factor * delta);
  _pActor->setPosition(newPos);
  if (!end)
    return;

  _path.erase(_path.begin());
  if (!_path.empty()) {
    _pActor->getCostume().setFacing(getFacing());
    _pActor->getCostume().setWalkState();
    _init = newPos;
    _elapsed = sf::seconds(0);
    trace("{} go to : {},{}", _pActor->getName(), _path[0].x, _path[0].y);
    return;
  }

  stop();
  trace("Play anim stand");
  if (_facing.has_value()) {
    _pActor->getCostume().setFacing(_facing.value());
  }
  _pActor->getCostume().setStandState();
  if (ScriptEngine::rawExists(_pActor, "actorArrived")) {
    ScriptEngine::rawCall(_pActor, "actorArrived");
  }
}

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
  return pRoom->getRoomScaling().getScaling(getRealPosition().y);
}

void Actor::draw(sf::RenderTarget &target, sf::RenderStates states) const {
  auto statesHotSpot = states;

  if (isVisible()) {
    auto scale = getScale();
    auto transformable = getTransform();
    transformable.scale(scale, scale);
    transformable.move(getRenderOffset().x * scale, getRenderOffset().y * scale);
    transformable.setPosition(transformable.getPosition().x,
                              target.getView().getSize().y - transformable.getPosition().y);
    states.transform *= transformable.getTransform();
    target.draw(pImpl->_costume, states);
  }

  auto scale = getScale();
  auto transformable = getTransform();
  transformable.scale(scale, scale);
  transformable.setPosition(transformable.getPosition().x,
                            target.getView().getSize().y - transformable.getPosition().y);
  statesHotSpot.transform *= transformable.getTransform();
  pImpl->drawHotspot(target, statesHotSpot);
}

void Actor::drawForeground(sf::RenderTarget &target, sf::RenderStates states) const {
  Entity::drawForeground(target, states);
  if (pImpl->_path && pImpl->_pRoom && pImpl->_engine.getWalkboxesFlags()) {
    target.draw(*pImpl->_path, states);
  }
}

void Actor::update(const sf::Time &elapsed) {
  Entity::update(elapsed);

  pImpl->_costume.update(elapsed);
  pImpl->_walkingState.update(elapsed);
}

std::vector<sf::Vector2f> Actor::walkTo(const sf::Vector2f &destination, std::optional<Facing> facing) {
  if (pImpl->_pRoom == nullptr)
    return {getRealPosition()};

  std::vector<sf::Vector2f> path;
  if (pImpl->_useWalkboxes) {
    path = pImpl->_pRoom->calculatePath(getRealPosition(), destination);
    if (path.size() < 2) {
      pImpl->_path = nullptr;
      return path;
    }
  } else {
    path.push_back(getRealPosition());
    path.push_back(destination);
  }

  pImpl->_path = std::make_unique<_Path>(path);
  if (ScriptEngine::rawExists(this, "preWalking")) {
    ScriptEngine::rawCall(this, "preWalking");
  }
  pImpl->_walkingState.setDestination(path, facing);
  return path;
}

void Actor::trigSound(const std::string &name) {
  auto soundId = pImpl->_engine.getSoundDefinition(name);
  if (!soundId)
    return;
  pImpl->_engine.getSoundManager().playSound(soundId);
}

void Actor::setFps(int fps) {
  pImpl->_fps = fps;
}

int Actor::getFps() const {
  return pImpl->_fps;
}

void Actor::setInventoryOffset(int offset) { pImpl->_inventoryOffset = offset; }

int Actor::getInventoryOffset() const {
  if (static_cast<size_t>(pImpl->_inventoryOffset * 4) > getObjects().size()) {
    pImpl->_inventoryOffset = 0;
  }
  return pImpl->_inventoryOffset;
}

} // namespace ng
