#include <engge/Room/Room.hpp>
#include <engge/Engine/EngineSettings.hpp>
#include <engge/Engine/Light.hpp>
#include <engge/System/Locator.hpp>
#include <engge/System/Logger.hpp>
#include <engge/Engine/EntityManager.hpp>
#include <engge/Room/RoomLayer.hpp>
#include <engge/Room/RoomScaling.hpp>
#include <engge/Graphics/SpriteSheet.hpp>
#include <engge/Entities/TextObject.hpp>
#include <engge/Scripting/ScriptEngine.hpp>
#include <engge/Entities/AnimationLoader.hpp>
#include "Util/Util.hpp"
#include <squirrel.h>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <memory>
#include <ngf/Math/PathFinding/PathFinder.h>
#include <ngf/Math/PathFinding/Walkbox.h>
#include <ngf/Graphics/RectangleShape.h>
#include <ngf/Graphics/FntFont.h>

namespace ng {

struct CmpLayer {
  bool operator()(int a, int b) const { return a > b; }
};

struct Room::Impl {
  ResourceManager &_textureManager;
  std::vector<std::unique_ptr<Object>> _objects;
  std::vector<Object *> _objectsToDelete;
  std::vector<ngf::Walkbox> _walkboxes;
  std::vector<ngf::Walkbox> _graphWalkboxes;
  std::map<int, std::unique_ptr<RoomLayer>, CmpLayer> _layers;
  std::vector<RoomScaling> _scalings;
  RoomScaling _scaling;
  glm::ivec2 _roomSize{0, 0};
  int32_t _screenHeight{0};
  std::string _sheet;
  std::string _name;
  int _fullscreen{0};
  HSQOBJECT _roomTable{};
  std::shared_ptr<ngf::PathFinder> _pf;
  ngf::Color _ambientColor{255, 255, 255, 255};
  SpriteSheet _spriteSheet;
  Room *_pRoom{nullptr};
  std::array<Light, LightingShader::MaxLights> _lights;
  int _numLights{0};
  float _rotation{0};
  LightingShader _lightingShader;
  int _selectedEffect{RoomEffectConstants::EFFECT_NONE};
  ngf::Color _overlayColor{ngf::Colors::Transparent};
  bool _pseudoRoom{false};

  explicit Impl(HSQOBJECT roomTable)
      : _textureManager(Locator<ResourceManager>::get()),
        _roomTable(roomTable) {
    _spriteSheet.setTextureManager(&_textureManager);
    for (int i = -3; i < 6; ++i) {
      _layers[i] = std::make_unique<RoomLayer>();
    }
  }

  void setEffect(int effect) {
    _selectedEffect = effect;
  }

  void setRoom(Room *pRoom) { _pRoom = pRoom; }

  void loadBackgrounds(ngf::GGPackValue &jWimpy) {
    int width = 0;
    if (!jWimpy["fullscreen"].isNull()) {
      _fullscreen = jWimpy["fullscreen"].getInt();
    }
    _layers[0]->setTexture(_spriteSheet.getTextureName());
    auto screenHeight = _pRoom->getScreenSize().y;
    auto offsetY = screenHeight - _pRoom->getRoomSize().y;
    if (jWimpy["background"].isArray()) {
      for (auto &bg : jWimpy["background"]) {
        auto frame = _spriteSheet.getRect(bg.getString());
        auto sourceSize = _spriteSheet.getSourceSize(bg.getString());
        auto spriteSourceSize = _spriteSheet.getSpriteSourceSize(bg.getString());
        _layers[0]->setRoomSizeY(_pRoom->getRoomSize().y);
        _layers[0]->setOffsetY(offsetY);
        _layers[0]->getBackgrounds().emplace_back(SpriteSheetItem{"background", frame, spriteSourceSize, sourceSize,
                                                                  false});
        width += frame.getWidth();
      }
    } else if (jWimpy["background"].isString()) {
      auto frame = _spriteSheet.getRect(jWimpy["background"].getString());
      auto sourceSize = _spriteSheet.getSourceSize(jWimpy["background"].getString());
      auto spriteSourceSize = _spriteSheet.getSpriteSourceSize(jWimpy["background"].getString());
      _layers[0]->setRoomSizeY(_pRoom->getRoomSize().y);
      _layers[0]->setOffsetY(offsetY);
      _layers[0]->getBackgrounds().emplace_back(SpriteSheetItem{"background", frame, spriteSourceSize, sourceSize,
                                                                false});
    }
    // room width seems to be not enough :S
    if (width > _roomSize.x) {
      _roomSize.x = width;
    }
  }

  void loadLayers(const ngf::GGPackValue &jWimpy) {
    if (jWimpy["layers"].isNull())
      return;

    auto offsetY = _pRoom->getScreenSize().y - _pRoom->getRoomSize().y;

    for (auto jLayer : jWimpy["layers"]) {
      auto zsort = jLayer["zsort"].getInt();
      auto &layer = _layers[zsort];
      layer->setRoomSizeY(_pRoom->getRoomSize().y);
      layer->setOffsetY(offsetY);
      layer->setTexture(_spriteSheet.getTextureName());
      layer->setZOrder(zsort);
      if (jLayer["name"].isArray()) {
        for (const auto &jName : jLayer["name"]) {
          layer->getBackgrounds().push_back(_spriteSheet.getItem(jName.getString()));
        }
      } else {
        layer->getBackgrounds().push_back(_spriteSheet.getItem(jLayer["name"].getString()));
      }
      if (jLayer["parallax"].isString()) {
        layer->setParallax(parsePos(jLayer["parallax"].getString()));
      } else {
        layer->setParallax({jLayer["parallax"].getDouble(), 1});
      }
    }
  }

  void loadObjects(const ngf::GGPackValue &jWimpy) {
    for (auto jObject : jWimpy["objects"]) {
      std::unique_ptr<Object> object;

      auto objectName = jObject["name"].getString();
      auto v = ScriptEngine::getVm();
      sq_pushobject(v, _pRoom->getTable());
      sq_pushstring(v, objectName.c_str(), -1);
      if (SQ_FAILED(sq_rawget(v, -2))) {
        object = std::make_unique<Object>();
        object->setTouchable(false);
        object->setName(objectName);
      } else {
        HSQOBJECT objTable;
        sq_resetobject(&objTable);
        if (_pRoom->isPseudoRoom()) {
          sq_clone(v, -1);
          sq_getstackobj(v, -1, &objTable);
          sq_remove(v, -2);
        } else {
          sq_getstackobj(v, -1, &objTable);
        }
        object = std::make_unique<Object>(objTable);

        bool initTouchable;
        if (ScriptEngine::get(object.get(), "initTouchable", initTouchable)) {
          object->setTouchable(initTouchable);
        }
      }
      if (!_pRoom->isPseudoRoom()) {
        ScriptEngine::set(objectName.data(), object->getTable());
      }
      ScriptEngine::set(_pRoom, objectName.data(), object->getTable());

      if (!ScriptEngine::rawExists(object.get(), "flags")) {
        ScriptEngine::set(object.get(), "flags", 0);
      }

      sq_pushobject(v, object->getTable());
      sq_pushobject(v, _pRoom->getTable());
      sq_setdelegate(v, -2);
      sq_pop(v, 1);

      // name
      object->setKey(objectName);
      // parent
      if (jObject["parent"].isString()) {
        auto parent = jObject["parent"].getString();
        auto it = std::find_if(_objects.begin(), _objects.end(), [&parent](const std::unique_ptr<Object> &o) {
          return o->getName() == parent;
        });
        if (it != _objects.end()) {
          object->setParent((*it).get());
        }
      }
      // zsort
      object->setZOrder(jObject["zsort"].getInt());
      // position
      auto pos = parsePos(jObject["pos"].getString());
      auto usePos = parsePos(jObject["usepos"].getString());
      auto useDir = toDirection(jObject["usedir"].getString());
      object->setUseDirection(useDir);
      // hotspot
      auto hotspot = parseRect(jObject["hotspot"].getString());
      object->setHotspot(ngf::irect::fromPositionSize(hotspot.getTopLeft(), hotspot.getSize()));
      // prop
      bool isProp = jObject["prop"].isInteger() && jObject["prop"].getInt() == 1;
      if (isProp)
        object->setType(ObjectType::Prop);
      // spot
      bool isSpot = jObject["spot"].isInteger() && jObject["spot"].getInt() == 1;
      if (isSpot)
        object->setType(ObjectType::Spot);
      // trigger
      bool isTrigger = jObject["trigger"].isInteger() && jObject["trigger"].getInt() == 1;
      if (isTrigger)
        object->setType(ObjectType::Trigger);

      object->setPosition(pos);
      object->setUsePosition(usePos);

      // animations
      if (jObject["animations"].isArray()) {
        auto anims = AnimationLoader::parseAnimations(*object, jObject["animations"], _spriteSheet);
        auto &objAnims = object->getAnims();
        std::copy(anims.begin(), anims.end(), std::back_inserter(objAnims));

        int initState = 0;
        ScriptEngine::get(object.get(), "initState", initState);
        object->setStateAnimIndex(initState);
      }
      object->setRoom(_pRoom);
      _layers[0]->addEntity(*object);
      _objects.push_back(std::move(object));
    }

    // update parent, it has to been done after objects initialization
    const auto &jObjects = jWimpy["objects"];
    for (auto &object : _objects) {
      auto name = object->getName();
      auto it = std::find_if(jObjects.cbegin(), jObjects.cend(), [&name](const auto &jObject) {
        return jObject["name"].getString() == name;
      });
      if (it == jObjects.cend())
        continue;
      auto jParent = (*it)["parent"];
      if (jParent.isNull())
        continue;
      auto parent = jParent.getString();
      auto itParent = std::find_if(_objects.cbegin(), _objects.cend(), [&parent](const auto &o) {
        return o->getName() == parent;
      });
      if (itParent == _objects.cend())
        continue;
      object->setParent(itParent->get());
    }

    // sort objects
    auto cmpObjects = [](std::unique_ptr<Object> &a, std::unique_ptr<Object> &b) {
      return a->getZOrder() > b->getZOrder();
    };
    std::sort(_objects.begin(), _objects.end(), cmpObjects);
  }

  static SQInteger createObjectsFromTable(Room *pRoom, std::unordered_map<std::string, HSQOBJECT> &roomObjects) {
    auto v = ScriptEngine::getVm();
    auto isPseudoRoom = pRoom->isPseudoRoom();
    auto &roomTable = pRoom->getTable();

    // iterate each table from roomTable
    sq_pushobject(v, roomTable);
    sq_pushnull(v);
    while (SQ_SUCCEEDED(sq_next(v, -2))) {
//here -1 is the value and -2 is the key
      auto type = sq_gettype(v, -1);
      if (type == OT_TABLE) {
        const SQChar *key = nullptr;
        sq_getstring(v, -2, &key);
        HSQOBJECT object;
        sq_resetobject(&object);
        sq_getstackobj(v, -1, &object);
        if (roomObjects.find(key) == roomObjects.end()) {
          std::unique_ptr<Object> obj;
          if (isPseudoRoom) {
            obj = std::make_unique<Object>();
            sq_pushobject(v, object);
            sq_clone(v, -1);
            sq_getstackobj(v, -1, &obj->getTable());
            sq_addref(v, &obj->getTable());
            sq_pop(v, 2);
          } else {
            sq_addref(v, &object);
            obj = std::make_unique<Object>(object);
            ScriptEngine::set(key, obj->getTable());
          }

          auto initState = 0;
          ScriptEngine::get(obj.get(), "initState", initState);
          obj->setStateAnimIndex(initState);

          bool initTouchable;
          if (ScriptEngine::get(obj.get(), "initTouchable", initTouchable)) {
            obj->setTouchable(initTouchable);
          }

          if (!ScriptEngine::rawExists(obj.get(), "flags")) {
            ScriptEngine::set(obj.get(), "flags", 0);
          }

          sq_pushobject(v, obj->getTable());
          sq_pushobject(v, roomTable);
          sq_setdelegate(v, -2);
          sq_pop(v, 1);

          obj->setRoom(pRoom);
          obj->setKey(key);
          ScriptEngine::set(pRoom, key, obj->getTable());
          pRoom->getObjects().push_back(std::move(obj));
          roomObjects[key] = object;
        }
      }
      sq_pop(v, 2); //pops key and val before the nex iteration
    }
    sq_pop(v, 2); //pops the null iterator & roomTable
    return 0;
  }

  static Scaling parseScaling(std::string_view value) {
    auto index = value.find('@');
    auto scale = std::strtof(value.substr(0, index).data(), nullptr);
    auto yPos = std::strtof(value.substr(index + 1).data(), nullptr);
    return {scale, yPos};
  }

  void loadScalings(const ngf::GGPackValue &jWimpy) {
    if (jWimpy["scaling"].isArray()) {
      if (jWimpy["scaling"][0].isString()) {
        RoomScaling scaling;
        for (const auto &jScaling : jWimpy["scaling"]) {
          scaling.getScalings().push_back(parseScaling(jScaling.getString()));
        }
        _scalings.push_back(scaling);
      } else if (jWimpy["scaling"][0].isHash()) {
        for (auto jScaling : jWimpy["scaling"]) {
          RoomScaling scaling;
          if (jScaling["trigger"].isString()) {
            scaling.setTrigger(jScaling["trigger"].getString());
          }
          for (const auto &jSubScaling : jScaling["scaling"]) {
            if (jSubScaling.isString()) {
              scaling.getScalings().push_back(parseScaling(jSubScaling.getString()));
            } else if (jSubScaling.isArray()) {
              for (const auto &jSubScalingScaling : jSubScaling) {
                scaling.getScalings().push_back(parseScaling(jSubScalingScaling.getString()));
              }
            }
          }
          _scalings.push_back(scaling);
        }
      }
    }

    if (_scalings.empty()) {
      RoomScaling scaling;
      _scalings.push_back(scaling);
    }
  }

  void loadWalkboxes(const ngf::GGPackValue &jWimpy) {
    for (auto jWalkbox : jWimpy["walkboxes"]) {
      std::vector<glm::ivec2> vertices;
      auto polygon = jWalkbox["polygon"].getString();
      parsePolygon(polygon, vertices);
      ngf::Walkbox walkbox(vertices);
      walkbox.setYAxisDirection(ngf::YAxisDirection::Up);
      if (jWalkbox["name"].isString()) {
        auto walkboxName = jWalkbox["name"].getString();
        walkbox.setName(walkboxName);
      }
      _walkboxes.push_back(walkbox);
    }
    _pf.reset();
  }

  bool updateGraph(const glm::vec2 &start) {
    _graphWalkboxes.clear();
    if (!_walkboxes.empty()) {
      _graphWalkboxes = ngf::Walkbox::merge(_walkboxes);
      return sortWalkboxes(start);
    }
    return false;
  }

  bool sortWalkboxes(const glm::vec2 &start) {
    auto it = std::find_if(_graphWalkboxes.begin(), _graphWalkboxes.end(), [start](auto &w) {
      return w.inside(start);
    });
    if (it != _graphWalkboxes.end()) {
      std::iter_swap(_graphWalkboxes.begin(), it);
    }
    _pf = std::make_shared<ngf::PathFinder>(_graphWalkboxes);
    return true;
  }
};

std::unique_ptr<Room> Room::define(HSQOBJECT roomTable, const char *name) {
  auto isPseudoRoom = name != nullptr;
  auto pRoom = std::make_unique<Room>(roomTable);

  // loadRoom
  const char *background = nullptr;
  if (!ScriptEngine::get(pRoom.get(), "background", background)) {
    error("Can't find background entry");
    return nullptr;
  }

  pRoom->setName(isPseudoRoom ? name : background);
  ScriptEngine::set(pRoom.get(), "_key", pRoom->getName());
  pRoom->setPseudoRoom(isPseudoRoom);
  pRoom->load(background);

  std::unordered_map<std::string, HSQOBJECT> roomObjects;
  for (auto &obj: pRoom->getObjects()) {
    roomObjects[obj->getKey()] = obj->getTable();
  }
  auto result = Impl::createObjectsFromTable(pRoom.get(), roomObjects);
  if (SQ_FAILED(result)) {
    error("Error when reading room objects");
    return nullptr;
  }

  // declare room in root table
  ScriptEngine::set(pRoom->getName().data(), pRoom.get());
  return pRoom;
}

Room::Room(HSQOBJECT roomTable)
    : m_pImpl(std::make_unique<Impl>(roomTable)) {
  m_id = Locator<EntityManager>::get().getRoomId();
  m_pImpl->setRoom(this);
  ScriptEngine::set(this, "_id", getId());
}

Room::~Room() = default;

void Room::setName(const std::string &name) { m_pImpl->_name = name; }
std::string Room::getName() const { return m_pImpl->_name; }

std::vector<std::unique_ptr<Object>> &Room::getObjects() { return m_pImpl->_objects; }
const std::vector<std::unique_ptr<Object>> &Room::getObjects() const { return m_pImpl->_objects; }

std::array<Light, LightingShader::MaxLights> &Room::getLights() { return m_pImpl->_lights; }

std::vector<ngf::Walkbox> &Room::getWalkboxes() { return m_pImpl->_walkboxes; }

const ngf::Walkbox *Room::getWalkbox(const std::string &name) const {
  auto it = std::find_if(m_pImpl->_walkboxes.begin(), m_pImpl->_walkboxes.end(), [&name](const auto &w) {
    return w.getName() == name;
  });
  if (it != m_pImpl->_walkboxes.end()) {
    return &(*it);
  }
  return nullptr;
}

std::vector<ngf::Walkbox> &Room::getGraphWalkboxes() { return m_pImpl->_graphWalkboxes; }

glm::ivec2 Room::getRoomSize() const { return m_pImpl->_roomSize; }

int32_t Room::getScreenHeight() const { return m_pImpl->_screenHeight; }

int32_t Room::getFullscreen() const { return m_pImpl->_fullscreen; }

HSQOBJECT &Room::getTable() { return m_pImpl->_roomTable; }

void Room::setAmbientLight(ngf::Color color) { m_pImpl->_ambientColor = color; }

ngf::Color Room::getAmbientLight() const { return m_pImpl->_ambientColor; }

void Room::setAsParallaxLayer(Entity *pEntity, int layerNum) {
  for (auto &layer : m_pImpl->_layers) {
    layer.second->removeEntity(*pEntity);
  }
  m_pImpl->_layers[layerNum]->addEntity(*pEntity);
}

void Room::roomLayer(int layerNum, bool enabled) { m_pImpl->_layers[layerNum]->setEnabled(enabled); }

void Room::removeEntity(Entity *pEntity) {
  for (auto &layer : m_pImpl->_layers) {
    layer.second->removeEntity(*pEntity);
  }
  m_pImpl->_objects.erase(std::remove_if(m_pImpl->_objects.begin(), m_pImpl->_objects.end(),
                                         [pEntity](auto &pObj) { return pObj.get() == pEntity; }),
                          m_pImpl->_objects.end());
}

void Room::load(const char *name) {
  // load wimpy file
  std::string wimpyFilename;
  wimpyFilename.append(name).append(".wimpy");
  trace("Load room {}", wimpyFilename);

  if (!Locator<EngineSettings>::get().hasEntry(wimpyFilename))
    return;

  auto hash = Locator<EngineSettings>::get().readEntry(wimpyFilename);

#if 0
  std::ofstream out;
  out.open(wimpyFilename, std::ios::out);
  out << hash;
  out.close();
#endif

  m_pImpl->_sheet = hash["sheet"].getString();
  m_pImpl->_screenHeight = hash["height"].getInt();
  m_pImpl->_roomSize = (glm::ivec2) parsePos(hash["roomsize"].getString());

  // load json file
  m_pImpl->_spriteSheet.load(m_pImpl->_sheet);

  m_pImpl->loadBackgrounds(hash);
  m_pImpl->loadLayers(hash);
  m_pImpl->loadObjects(hash);
  m_pImpl->loadScalings(hash);
  m_pImpl->loadWalkboxes(hash);
}

TextObject &Room::createTextObject(const std::string &fontName) {
  auto object = std::make_unique<TextObject>();
  std::string path;
  path.append(fontName).append(".fnt");
  if (!Locator<EngineSettings>::get().hasEntry(path)) {
    path.clear();
    path.append(fontName).append("Font.fnt");
  }

  const auto &font = m_pImpl->_textureManager.getFntFont(path);
  object->setFont(&font);
  auto &obj = *object;
  obj.setRoom(this);
  std::ostringstream s;
  s << "TextObject #" << m_pImpl->_objects.size();
  m_pImpl->_objects.push_back(std::move(object));
  m_pImpl->_layers[0]->addEntity(obj);
  return obj;
}

void Room::deleteObject(Object &object) {
  m_pImpl->_objectsToDelete.push_back(&object);
}

Object &Room::createObject(const std::vector<std::string> &anims) { return createObject(m_pImpl->_sheet, anims); }

Object &Room::createObject(const std::string &sheet, const std::vector<std::string> &frames) {
  auto object = std::make_unique<Object>();
  auto spriteSheet = m_pImpl->_textureManager.getSpriteSheet(sheet);

  Animation anim;
  anim.name = "state0";
  anim.texture = spriteSheet.getTextureName();

  for (auto frame :frames) {
    checkLanguage(frame);
    anim.frames.push_back(spriteSheet.getItem(frame));
  }
  object->getAnims().push_back(anim);
  object->setStateAnimIndex(0);
  object->setTemporary(true);
  object->setRoom(this);
  object->setZOrder(1);
  auto &obj = *object;
  m_pImpl->_layers[0]->addEntity(obj);
  m_pImpl->_objects.push_back(std::move(object));
  return obj;
}

Object &Room::createObject(const std::string &image) {
  auto name = image;
  checkLanguage(name);

  const std::vector<std::string> anims{name};
  auto object = std::make_unique<Object>();
  auto texture = Locator<ResourceManager>::get().getTexture(name + ".png");

  Animation anim;
  auto size = texture->getSize();
  ngf::irect rect = ngf::irect::fromPositionSize({0, 0}, size);
  anim.name = "state0";
  anim.texture = name + ".png";
  anim.frames.push_back(SpriteSheetItem{"state0", rect, rect, size, false});
  object->getAnims().push_back(anim);

  object->setAnimation("state0");
  auto &obj = *object;
  obj.setTemporary(true);
  obj.setZOrder(1);
  obj.setRoom(this);
  m_pImpl->_layers[0]->addEntity(obj);
  m_pImpl->_objects.push_back(std::move(object));
  return obj;
}

Object &Room::createObject() {
  auto object = std::make_unique<Object>();

  auto &obj = *object;
  obj.setTemporary(true);
  obj.setZOrder(1);
  obj.setRoom(this);
  m_pImpl->_layers[0]->addEntity(obj);
  m_pImpl->_objects.push_back(std::move(object));
  return obj;
}

const ngf::Graph *Room::getGraph() const {
  if (m_pImpl->_pf) {
    return m_pImpl->_pf->getGraph().get();
  }
  return nullptr;
}

void Room::update(const ngf::TimeSpan &elapsed) {
  if (!m_pImpl->_objectsToDelete.empty()) {
    for (auto &obj : m_pImpl->_objectsToDelete) {
      for (auto &&layer : m_pImpl->_layers) {
        layer.second->removeEntity(*obj);
      }
      m_pImpl->_objects.erase(std::remove_if(
          m_pImpl->_objects.begin(), m_pImpl->_objects.end(),
          [&obj](auto &pObj) { return pObj.get() == obj; }), m_pImpl->_objects.end());
    }
    m_pImpl->_objectsToDelete.clear();
  }

  for (auto &&layer : m_pImpl->_layers) {
    layer.second->update(elapsed);
  }
}

void Room::draw(ngf::RenderTarget &target, const glm::vec2 &cameraPos) const {
  // update lighting
  auto nLights = m_pImpl->_numLights;
  m_pImpl->_lightingShader.setAmbientColor(m_pImpl->_ambientColor);
  m_pImpl->_lightingShader.setNumberLights(nLights);
  m_pImpl->_lightingShader.setLights(m_pImpl->_lights);

  for (const auto &layer : m_pImpl->_layers) {
    auto parallax = layer.second->getParallax();
    ngf::Transform t;
    t.move({-cameraPos.x * parallax.x, cameraPos.y * parallax.y});

    ngf::RenderStates states;
    states.shader = &m_pImpl->_lightingShader;
    states.transform = t.getTransform();
    layer.second->draw(target, states);
  }
}

void Room::drawForeground(ngf::RenderTarget &target, const glm::vec2 &cameraPos) const {
  ngf::RenderStates states;
  states.shader = &m_pImpl->_lightingShader;
  m_pImpl->_lightingShader.setNumberLights(0);
  m_pImpl->_lightingShader.setAmbientColor(ngf::Colors::White);

  for (const auto &layer : m_pImpl->_layers) {
    auto parallax = layer.second->getParallax();
    ngf::Transform t;
    t.setPosition({-cameraPos.x * parallax.x, cameraPos.y * parallax.y});

    states.transform = t.getTransform();
    layer.second->drawForeground(target, states);
  }
}

const RoomScaling &Room::getRoomScaling() const { return m_pImpl->_scaling; }

void Room::setRoomScaling(const RoomScaling &scaling) { m_pImpl->_scaling = scaling; }

void Room::setWalkboxEnabled(const std::string &name, bool isEnabled) {
  auto it = std::find_if(m_pImpl->_walkboxes.begin(), m_pImpl->_walkboxes.end(),
                         [&name](const auto &walkbox) { return walkbox.getName() == name; });
  if (it == m_pImpl->_walkboxes.end()) {
    error("walkbox {} has not been found", name);
    return;
  }
  it->setEnabled(isEnabled);
  m_pImpl->_pf.reset();
}

std::vector<RoomScaling> &Room::getScalings() { return m_pImpl->_scalings; }

std::vector<glm::vec2> Room::calculatePath(glm::vec2 start, glm::vec2 end) const {
  if (!m_pImpl->_pf) {
    if (!m_pImpl->updateGraph(start)) {
      return std::vector<glm::vec2>();
    }
  } else if (!m_pImpl->_graphWalkboxes.empty() && !m_pImpl->_graphWalkboxes[0].inside(start)) {
    if (!m_pImpl->sortWalkboxes(start)) {
      return std::vector<glm::vec2>();
    }
  }
  return m_pImpl->_pf->calculatePath(start, end);
}

float Room::getRotation() const { return m_pImpl->_rotation; }

void Room::setRotation(float angle) { m_pImpl->_rotation = angle; }

Light *Room::createLight(ngf::Color color, glm::ivec2 pos) {
  auto &light = m_pImpl->_lights[m_pImpl->_numLights++];
  light.color = color;
  light.pos = pos;
  return &light;
}

int Room::getNumberLights() const { return m_pImpl->_numLights; }

LightingShader& Room::getLightingShader() { return m_pImpl->_lightingShader; }

void Room::exit() {
  m_pImpl->_numLights = 0;
  for (auto &obj : m_pImpl->_objects) {
    if (!obj->isTemporary())
      continue;
    for (auto &layer : m_pImpl->_layers) {
      layer.second->removeEntity(*obj);
    }
  }
  m_pImpl->_objects.erase(std::remove_if(m_pImpl->_objects.begin(), m_pImpl->_objects.end(),
                                         [](auto &pObj) { return pObj->isTemporary(); }), m_pImpl->_objects.end());
}

void Room::setEffect(int effect) { m_pImpl->setEffect(effect); }

int Room::getEffect() const { return m_pImpl->_selectedEffect; }

void Room::setOverlayColor(ngf::Color color) { m_pImpl->_overlayColor = color; }

ngf::Color Room::getOverlayColor() const { return m_pImpl->_overlayColor; }

const SpriteSheet &Room::getSpriteSheet() const { return m_pImpl->_spriteSheet; }

glm::ivec2 Room::getScreenSize() const {
  glm::ivec2 screen;
  if (getFullscreen() == 1) {
    screen = getRoomSize();
    if (getScreenHeight() != 0) {
      screen.y = getScreenHeight();
    }
  } else {
    auto height = getScreenHeight();
    switch (height) {
    case 0:return getRoomSize();
    case 128:return glm::ivec2(320, 180);
    case 172:return glm::ivec2(428, 240);
    case 256:return glm::ivec2(640, 360);
    default: {
      height = 180.f * height / 128.f;
      auto ratio = 320.f / 180.f;
      return glm::ivec2(ratio * height, height);
    }
    }
  }
  return screen;
}

void Room::setPseudoRoom(bool pseudoRoom) {
  m_pImpl->_pseudoRoom = pseudoRoom;
}

bool Room::isPseudoRoom() const {
  return m_pImpl->_pseudoRoom;
}

} // namespace ng
