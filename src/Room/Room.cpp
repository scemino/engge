#include <engge/Room/Room.hpp>
#include <engge/Engine/EngineSettings.hpp>
#include <engge/Engine/Light.hpp>
#include <engge/System/Locator.hpp>
#include <engge/System/Logger.hpp>
#include <engge/Engine/EntityManager.hpp>
#include <engge/Room/RoomLayer.hpp>
#include <engge/Room/RoomScaling.hpp>
#include <engge/Graphics/SpriteSheet.hpp>
#include <engge/Graphics/FntFont.h>
#include <engge/Entities/Objects/TextObject.hpp>
#include <engge/Scripting/ScriptEngine.hpp>
#include <engge/Entities/AnimationLoader.hpp>
#include "../Util/Util.hpp"
#include <squirrel.h>
#include <clipper.hpp>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <memory>
#include <ngf/Math/PathFinding/PathFinder.h>
#include <ngf/Math/PathFinding/Walkbox.h>
#include <ngf/Graphics/RectangleShape.h>

namespace ng {

struct CmpLayer {
  bool operator()(int a, int b) const { return a > b; }
};

struct Room::Impl {
  ResourceManager &_textureManager;
  std::vector<std::unique_ptr<Object>> _objects;
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
          auto layerName = jName.getString();
          auto frame = _spriteSheet.getRect(layerName);
          auto spriteSourceSize = _spriteSheet.getSpriteSourceSize(layerName);
          auto sourceSize = _spriteSheet.getSourceSize(layerName);
          layer->getBackgrounds().push_back(SpriteSheetItem{layerName, frame, spriteSourceSize, sourceSize, false});
        }
      } else {
        auto layerName = jLayer["name"].getString();
        auto frame = _spriteSheet.getRect(layerName);
        auto spriteSourceSize = _spriteSheet.getSpriteSourceSize(layerName);
        auto sourceSize = _spriteSheet.getSourceSize(layerName);
        layer->getBackgrounds().push_back(SpriteSheetItem{layerName, frame, spriteSourceSize, sourceSize, false});
      }
      if (jLayer["parallax"].isString()) {
        auto parallax = _parsePos(jLayer["parallax"].getString());
        layer->setParallax(parallax);
      } else {
        auto parallax = jLayer["parallax"].getDouble();
        layer->setParallax({parallax, 1});
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
      auto pos = _parsePos(jObject["pos"].getString());
      auto usePos = _parsePos(jObject["usepos"].getString());
      auto useDir = _toDirection(jObject["usedir"].getString());
      object->setUseDirection(useDir);
      // hotspot
      auto hotspot = _parseRect(jObject["hotspot"].getString());
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
      object->setTexture(_spriteSheet.getTextureName());
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
    const auto& jObjects = jWimpy["objects"];
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

  void loadScalings(const ngf::GGPackValue &jWimpy) {
    if (jWimpy["scaling"].isArray()) {
      if (jWimpy["scaling"][0].isString()) {
        RoomScaling scaling;
        for (const auto &jScaling : jWimpy["scaling"]) {
          auto value = jScaling.getString();
          auto index = value.find('@');
          auto scale = std::strtof(value.substr(0, index).c_str(), nullptr);
          auto yPos = std::strtof(value.substr(index + 1).c_str(), nullptr);
          Scaling s{};
          s.scale = scale;
          s.yPos = yPos;
          scaling.getScalings().push_back(s);
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
              auto value = jSubScaling.getString();
              auto index = value.find('@');
              auto scale = std::strtof(value.substr(0, index).c_str(), nullptr);
              auto yPos = std::strtof(value.substr(index + 1).c_str(), nullptr);
              Scaling s{};
              s.scale = scale;
              s.yPos = yPos;
              scaling.getScalings().push_back(s);
            } else if (jSubScaling.isArray()) {
              for (const auto &jSubScalingScaling : jSubScaling) {
                auto value = jSubScalingScaling.getString();
                auto index = value.find('@');
                auto scale = std::strtof(value.substr(0, index).c_str(), nullptr);
                auto yPos = std::strtof(value.substr(index + 1).c_str(), nullptr);
                Scaling s{};
                s.scale = scale;
                s.yPos = yPos;
                scaling.getScalings().push_back(s);
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
      _parsePolygon(polygon, vertices);
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

  static void toPath(const ngf::Walkbox &walkbox, ClipperLib::Path &path) {
    const auto &vertices = walkbox.getVertices();
    std::transform(vertices.begin(), vertices.end(), std::back_inserter(path),
                   [](const auto &p) { return ClipperLib::IntPoint{p.x, p.y}; });
  }

  bool updateGraph(const glm::vec2 &start) {
    _graphWalkboxes.clear();
    if (!_walkboxes.empty()) {
      mergeWalkboxes();
      return sortWalkboxes(start);
    }
    return false;
  }

  void mergeWalkboxes() {
    ClipperLib::Paths solutions;
    ClipperLib::Path path;
    toPath(_walkboxes[0], path);
    solutions.push_back(path);

    for (int i = 1; i < static_cast<int>(_walkboxes.size()); i++) {
      if (!_walkboxes[i].isEnabled())
        continue;
      path.clear();
      toPath(_walkboxes[i], path);
      ClipperLib::Clipper clipper;
      clipper.AddPaths(solutions, ClipperLib::ptSubject, true);
      clipper.AddPath(path, ClipperLib::ptClip, true);
      solutions.clear();
      clipper.Execute(ClipperLib::ctUnion, solutions, ClipperLib::pftEvenOdd);
    }

    for (auto &sol:solutions) {
      std::vector<glm::ivec2> sPoints;
      std::transform(sol.begin(), sol.end(), std::back_inserter(sPoints), [](auto &p) -> glm::ivec2 {
        return glm::ivec2(p.X, p.Y);
      });
      bool isEnabled = ClipperLib::Orientation(sol);
      if (!isEnabled) {
        std::reverse(sPoints.begin(), sPoints.end());
      }
      ngf::Walkbox walkbox(sPoints);
      walkbox.setYAxisDirection(ngf::YAxisDirection::Down);
      walkbox.setEnabled(isEnabled);
      _graphWalkboxes.push_back(walkbox);
    }
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
    : pImpl(std::make_unique<Impl>(roomTable)) {
  _id = Locator<EntityManager>::get().getRoomId();
  pImpl->setRoom(this);
  ScriptEngine::set(this, "_id", getId());
}

Room::~Room() = default;

void Room::setName(const std::string &name) { pImpl->_name = name; }
std::string Room::getName() const { return pImpl->_name; }

std::vector<std::unique_ptr<Object>> &Room::getObjects() { return pImpl->_objects; }
const std::vector<std::unique_ptr<Object>> &Room::getObjects() const { return pImpl->_objects; }

std::array<Light, LightingShader::MaxLights> &Room::getLights() { return pImpl->_lights; }

std::vector<ngf::Walkbox> &Room::getWalkboxes() { return pImpl->_walkboxes; }

const ngf::Walkbox *Room::getWalkbox(const std::string &name) const {
  auto it = std::find_if(pImpl->_walkboxes.begin(), pImpl->_walkboxes.end(), [&name](const auto &w) {
    return w.getName() == name;
  });
  if (it != pImpl->_walkboxes.end()) {
    return &(*it);
  }
  return nullptr;
}

std::vector<ngf::Walkbox> &Room::getGraphWalkboxes() { return pImpl->_graphWalkboxes; }

glm::ivec2 Room::getRoomSize() const { return pImpl->_roomSize; }

int32_t Room::getScreenHeight() const { return pImpl->_screenHeight; }

int32_t Room::getFullscreen() const { return pImpl->_fullscreen; }

HSQOBJECT &Room::getTable() { return pImpl->_roomTable; }

void Room::setAmbientLight(ngf::Color color) { pImpl->_ambientColor = color; }

ngf::Color Room::getAmbientLight() const { return pImpl->_ambientColor; }

void Room::setAsParallaxLayer(Entity *pEntity, int layerNum) {
  for (auto &layer : pImpl->_layers) {
    layer.second->removeEntity(*pEntity);
  }
  pImpl->_layers[layerNum]->addEntity(*pEntity);
}

void Room::roomLayer(int layerNum, bool enabled) { pImpl->_layers[layerNum]->setEnabled(enabled); }

void Room::removeEntity(Entity *pEntity) {
  for (auto &layer : pImpl->_layers) {
    layer.second->removeEntity(*pEntity);
  }
  pImpl->_objects.erase(std::remove_if(pImpl->_objects.begin(), pImpl->_objects.end(),
                                       [pEntity](std::unique_ptr<Object> &pObj) { return pObj.get() == pEntity; }),
                        pImpl->_objects.end());
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

  pImpl->_sheet = hash["sheet"].getString();
  pImpl->_screenHeight = hash["height"].getInt();
  pImpl->_roomSize = (glm::ivec2) _parsePos(hash["roomsize"].getString());

  // load json file
  pImpl->_spriteSheet.load(pImpl->_sheet);

  pImpl->loadBackgrounds(hash);
  pImpl->loadLayers(hash);
  pImpl->loadObjects(hash);
  pImpl->loadScalings(hash);
  pImpl->loadWalkboxes(hash);
}

TextObject &Room::createTextObject(const std::string &fontName) {
  auto object = std::make_unique<TextObject>();
  std::string path;
  path.append(fontName).append(".fnt");
  if (!Locator<EngineSettings>::get().hasEntry(path)) {
    path.clear();
    path.append(fontName).append("Font.fnt");
  }

  const auto &font = pImpl->_textureManager.getFntFont(path);
  object->setFont(&font);
  object->setTexture(getSpriteSheet().getTextureName());
  auto &obj = *object;
  obj.setVisible(true);
  obj.setRoom(this);
  std::ostringstream s;
  s << "TextObject #" << pImpl->_objects.size();
  pImpl->_objects.push_back(std::move(object));
  pImpl->_layers[0]->addEntity(obj);
  return obj;
}

void Room::deleteObject(Object &object) { pImpl->_layers[0]->removeEntity(object); }

Object &Room::createObject(const std::vector<std::string> &anims) { return createObject(pImpl->_sheet, anims); }

Object &Room::createObject(const std::string &sheet, const std::vector<std::string> &frames) {
  auto object = std::make_unique<Object>();
  auto spriteSheet = pImpl->_textureManager.getSpriteSheet(sheet);
  object->setTexture(spriteSheet.getTextureName());

  ObjectAnimation anim;
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
  pImpl->_layers[0]->addEntity(obj);
  pImpl->_objects.push_back(std::move(object));
  return obj;
}

Object &Room::createObject(const std::string &image) {
  auto name = image;
  checkLanguage(name);

  const std::vector<std::string> anims{name};
  auto object = std::make_unique<Object>();
  auto texture = Locator<ResourceManager>::get().getTexture(name);
  object->setTexture(name);

  ObjectAnimation anim;
  auto size = texture->getSize();
  ngf::irect rect = ngf::irect::fromPositionSize({0, 0}, size);
  anim.name = "state0";
  anim.texture = name;
  anim.frames.push_back(SpriteSheetItem{"state0", rect, rect, size, false});
  object->getAnims().push_back(anim);

  object->setAnimation("state0");
  auto &obj = *object;
  obj.setTemporary(true);
  obj.setZOrder(1);
  obj.setRoom(this);
  pImpl->_layers[0]->addEntity(obj);
  pImpl->_objects.push_back(std::move(object));
  return obj;
}

Object &Room::createObject() {
  auto object = std::make_unique<Object>();

  auto &obj = *object;
  obj.setTemporary(true);
  obj.setZOrder(1);
  obj.setRoom(this);
  pImpl->_layers[0]->addEntity(obj);
  pImpl->_objects.push_back(std::move(object));
  return obj;
}

const ngf::Graph *Room::getGraph() const {
  if (pImpl->_pf) {
    return pImpl->_pf->getGraph().get();
  }
  return nullptr;
}

void Room::update(const ngf::TimeSpan &elapsed) {
  for (auto &&layer : pImpl->_layers) {
    layer.second->update(elapsed);
  }
}

void Room::draw(ngf::RenderTarget &target, const glm::vec2 &cameraPos) const {

  // update lighting
  auto nLights = pImpl->_numLights;
  pImpl->_lightingShader.setAmbientColor(pImpl->_ambientColor);
  pImpl->_lightingShader.setNumberLights(nLights);
  pImpl->_lightingShader.setLights(pImpl->_lights);

  for (const auto &layer : pImpl->_layers) {
    auto parallax = layer.second->getParallax();
    ngf::Transform t;
    t.move({-cameraPos.x * parallax.x, cameraPos.y * parallax.y});

    ngf::RenderStates states;
    states.shader = &pImpl->_lightingShader;
    states.transform = t.getTransform();
    layer.second->draw(target, states);
  }
}

void Room::drawForeground(ngf::RenderTarget &target, const glm::vec2 &cameraPos) const {
  for (const auto &layer : pImpl->_layers) {
    auto parallax = layer.second->getParallax();
    ngf::Transform t;
    t.setPosition({-cameraPos.x * parallax.x, cameraPos.y * parallax.y});

    ngf::RenderStates states;
    states.transform = t.getTransform();
    layer.second->drawForeground(target, states);
  }
}

const RoomScaling &Room::getRoomScaling() const { return pImpl->_scaling; }

void Room::setRoomScaling(const RoomScaling &scaling) { pImpl->_scaling = scaling; }

void Room::setWalkboxEnabled(const std::string &name, bool isEnabled) {
  auto it = std::find_if(pImpl->_walkboxes.begin(), pImpl->_walkboxes.end(),
                         [&name](const auto &walkbox) { return walkbox.getName() == name; });
  if (it == pImpl->_walkboxes.end()) {
    error("walkbox {} has not been found", name);
    return;
  }
  it->setEnabled(isEnabled);
  pImpl->_pf.reset();
}

std::vector<RoomScaling> &Room::getScalings() { return pImpl->_scalings; }

std::vector<glm::vec2> Room::calculatePath(glm::vec2 start, glm::vec2 end) const {
  if (!pImpl->_pf) {
    if (!pImpl->updateGraph(start)) {
      return std::vector<glm::vec2>();
    }
  } else if (!pImpl->_graphWalkboxes.empty() && !pImpl->_graphWalkboxes[0].inside(start)) {
    if (!pImpl->sortWalkboxes(start)) {
      return std::vector<glm::vec2>();
    }
  }
  return pImpl->_pf->calculatePath(start, end);
}

float Room::getRotation() const { return pImpl->_rotation; }

void Room::setRotation(float angle) { pImpl->_rotation = angle; }

Light *Room::createLight(ngf::Color color, glm::ivec2 pos) {
  auto &light = pImpl->_lights[pImpl->_numLights++];
  light.color = color;
  light.pos = pos;
  return &light;
}

int Room::getNumberLights() const { return pImpl->_numLights; }

void Room::exit() {
  pImpl->_numLights = 0;
  for (auto &obj : pImpl->_objects) {
    if (!obj->isTemporary())
      continue;
    for (auto &layer : pImpl->_layers) {
      layer.second->removeEntity(*obj);
    }
  }
  pImpl->_objects.erase(std::remove_if(pImpl->_objects.begin(), pImpl->_objects.end(),
                                       [](auto &pObj) { return pObj->isTemporary(); }),
                        pImpl->_objects.end());
}

void Room::setEffect(int effect) { pImpl->setEffect(effect); }

int Room::getEffect() const { return pImpl->_selectedEffect; }

void Room::setOverlayColor(ngf::Color color) { pImpl->_overlayColor = color; }

ngf::Color Room::getOverlayColor() const { return pImpl->_overlayColor; }

const SpriteSheet &Room::getSpriteSheet() const { return pImpl->_spriteSheet; }

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
  pImpl->_pseudoRoom = pseudoRoom;
}

bool Room::isPseudoRoom() const {
  return pImpl->_pseudoRoom;
}

} // namespace ng
