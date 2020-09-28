#include "engge/Room/Room.hpp"
#include "engge/Entities/Objects/Animation.hpp"
#include "engge/Engine/EngineSettings.hpp"
#include "engge/Math/PathFinding/Graph.hpp"
#include "engge/Parsers/JsonTokenReader.hpp"
#include "engge/Engine/Light.hpp"
#include "engge/System/Locator.hpp"
#include "engge/System/Logger.hpp"
#include "engge/Math/PathFinding/PathFinder.hpp"
#include "engge/Engine/EntityManager.hpp"
#include "engge/Room/RoomLayer.hpp"
#include "engge/Room/RoomScaling.hpp"
#include "engge/Graphics/SpriteSheet.hpp"
#include "engge/Entities/Objects/AnimationFrame.hpp"
#include "engge/Entities/Objects/TextObject.hpp"
#include "engge/Scripting/ScriptEngine.hpp"
#include "../System/_Util.hpp"
#include <squirrel.h>
#include <clipper.hpp>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <memory>

namespace ng {
struct CmpLayer {
  bool operator()(int a, int b) const { return a > b; }
};

struct Room::Impl {
  ResourceManager &_textureManager;
  std::vector<std::unique_ptr<Object>> _objects;
  std::vector<Walkbox> _walkboxes;
  std::vector<Walkbox> _graphWalkboxes;
  std::map<int, std::unique_ptr<RoomLayer>, CmpLayer> _layers;
  std::vector<RoomScaling> _scalings;
  RoomScaling _scaling;
  sf::Vector2i _roomSize;
  int32_t _screenHeight{0};
  std::string _sheet;
  std::string _name;
  int _fullscreen{0};
  HSQOBJECT _roomTable{};
  std::shared_ptr<PathFinder> _pf;
  sf::Color _ambientColor{255, 255, 255, 255};
  SpriteSheet _spriteSheet;
  Room *_pRoom{nullptr};
  std::vector<std::unique_ptr<Light>> _lights;
  float _rotation{0};
  sf::Shader _shader{};
  int _selectedEffect{RoomEffectConstants::EFFECT_NONE};
  sf::Color _overlayColor{sf::Color::Transparent};
  bool _pseudoRoom{false};

  explicit Impl(HSQOBJECT roomTable)
      : _textureManager(Locator<ResourceManager>::get()),
        _roomTable(roomTable) {
    _spriteSheet.setTextureManager(&_textureManager);
    for (int i = -3; i < 6; ++i) {
      _layers[i] = std::make_unique<RoomLayer>();
    }

    const std::string vertexShader = "void main()"
                                     "{"
                                     "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
                                     "    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;"
                                     "    gl_FrontColor = gl_Color;"
                                     "}";

    // load vertex shader
    if (!_shader.loadFromMemory(vertexShader, sf::Shader::Type::Vertex)) {
      std::cerr << "Error loading shaders" << std::endl;
      return;
    }
  }

  void setEffect(int effect) {
    if (effect == RoomEffectConstants::EFFECT_BLACKANDWHITE) {
      _selectedEffect = effect;
      const std::string fragmentShader = "uniform sampler2D texture;\n"
                                         "void main()\n"
                                         "{\n"
                                         "vec4 texColor = texture2D(texture, gl_TexCoord[0].xy);\n"
                                         "vec4 col = gl_Color * texColor;\n"
                                         "float gray = dot(col.xyz, vec3(0.299, 0.587, 0.114));\n"
                                         "gl_FragColor = vec4(gray, gray, gray, col.a);\n"
                                         "}";
      if (!_shader.loadFromMemory(fragmentShader, sf::Shader::Type::Fragment)) {
        std::cerr << "Error loading shaders" << std::endl;
      }
      _shader.setUniform("texture", sf::Shader::CurrentTexture);
      return;
    }
    _selectedEffect = RoomEffectConstants::EFFECT_NONE;
  }

  void setRoom(Room *pRoom) { _pRoom = pRoom; }

  void loadBackgrounds(GGPackValue &jWimpy) {
    int width = 0;
    if (!jWimpy["fullscreen"].isNull()) {
      _fullscreen = jWimpy["fullscreen"].int_value;
    }
    auto screenHeight = _pRoom->getScreenSize().y;
    if (jWimpy["background"].isArray()) {
      for (auto &bg : jWimpy["background"].array_value) {
        auto frame = _spriteSheet.getRect(bg.string_value);
        auto sourceSize = _spriteSheet.getSourceSize(bg.string_value);
        auto sourceRect = _spriteSheet.getSpriteSourceSize(bg.string_value);
        auto sprite = sf::Sprite();
        auto offset = sourceSize.y - screenHeight;
        sprite.move(sourceRect.left + width, sourceRect.top - offset);
        sprite.setTexture(*_textureManager.getTexture(_sheet));
        sprite.setTextureRect(frame);
        width += sprite.getTextureRect().width;
        _layers[0]->getSprites().push_back(sprite);
      }
    } else if (jWimpy["background"].isString()) {
      auto frame = _spriteSheet.getRect(jWimpy["background"].string_value);
      auto sourceSize = _spriteSheet.getSourceSize(jWimpy["background"].string_value);
      auto sourceRect = _spriteSheet.getSpriteSourceSize(jWimpy["background"].string_value);
      auto offset = sourceSize.y - screenHeight;
      auto sprite = sf::Sprite();
      sprite.move(sourceRect.left, sourceRect.top - offset);
      sprite.setTexture(*_textureManager.getTexture(_sheet));
      sprite.setTextureRect(frame);
      _layers[0]->getSprites().push_back(sprite);
    }
    // room width seems to be not enough :S
    if (width > _roomSize.x) {
      _roomSize.x = width;
    }
  }

  void loadLayers(GGPackValue &jWimpy) {
    if (jWimpy["layers"].isNull())
      return;

    auto offsetY = _pRoom->getScreenSize().y - _pRoom->getRoomSize().y;

    for (auto jLayer : jWimpy["layers"].array_value) {
      auto zsort = jLayer["zsort"].int_value;
      auto &layer = _layers[zsort];
      layer->setZOrder(zsort);
      if (jLayer["name"].isArray()) {
        float offsetX = 0;
        for (const auto &jName : jLayer["name"].array_value) {
          auto layerName = jName.string_value;
          auto rect = _spriteSheet.getRect(layerName);
          auto spriteSourceSize = _spriteSheet.getSpriteSourceSize(layerName);
          auto sourceSize = _spriteSheet.getSourceSize(layerName);
          sf::Vector2f origin(-spriteSourceSize.left, sourceSize.y - _roomSize.y - spriteSourceSize.top);
          sf::Sprite s;
          s.setTexture(*_textureManager.getTexture(_sheet));
          s.setTextureRect(rect);
          s.setOrigin(origin);
          s.move(sf::Vector2f(offsetX, offsetY));
          offsetX += rect.width;
          layer->getSprites().push_back(s);
        }
      } else {
        auto layerName = jLayer["name"].string_value;
        auto rect = _spriteSheet.getRect(layerName);
        auto spriteSourceSize = _spriteSheet.getSpriteSourceSize(layerName);
        auto sourceSize = _spriteSheet.getSourceSize(layerName);
        sf::Vector2f origin(-spriteSourceSize.left, sourceSize.y - _roomSize.y - spriteSourceSize.top);
        sf::Sprite s;
        s.setTexture(*_textureManager.getTexture(_sheet));
        s.setTextureRect(rect);
        s.setOrigin(origin);
        s.move(sf::Vector2f(0, offsetY));
        layer->getSprites().push_back(s);
      }
      if (jLayer["parallax"].isString()) {
        auto parallax = _parsePos(jLayer["parallax"].string_value);
        layer->setParallax(parallax);
      } else {
        auto parallax = jLayer["parallax"].double_value;
        layer->setParallax(sf::Vector2f(parallax, 1));
      }
    }
  }

  void loadObjects(const GGPackValue &jWimpy) {
    auto &texture = *_textureManager.getTexture(_sheet);

    for (auto jObject : jWimpy["objects"].array_value) {
      std::unique_ptr<Object> object;

      auto objectName = jObject["name"].string_value;
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
        auto parent = jObject["parent"].string_value;
        auto it = std::find_if(_objects.begin(), _objects.end(), [&parent](const std::unique_ptr<Object> &o) {
          return o->getName() == parent;
        });
        if (it != _objects.end()) {
          object->setParent((*it).get());
        }
      }
      // zsort
      object->setZOrder(jObject["zsort"].int_value);
      // position
      auto pos = _parsePos(jObject["pos"].string_value);
      auto usePos = _parsePos(jObject["usepos"].string_value);
      auto useDir = _toDirection(jObject["usedir"].string_value);
      object->setUseDirection(useDir);
      // hotspot
      auto hotspot = _parseRect(jObject["hotspot"].string_value);
      object->setHotspot(sf::IntRect(hotspot.left, hotspot.top, hotspot.width, hotspot.height));
      // prop
      bool isProp = jObject["prop"].isInteger() && jObject["prop"].int_value == 1;
      if (isProp)
        object->setType(ObjectType::Prop);
      // spot
      bool isSpot = jObject["spot"].isInteger() && jObject["spot"].int_value == 1;
      if (isSpot)
        object->setType(ObjectType::Spot);
      // trigger
      bool isTrigger = jObject["trigger"].isInteger() && jObject["trigger"].int_value == 1;
      if (isTrigger)
        object->setType(ObjectType::Trigger);

      object->setPosition(pos);
      object->setUsePosition(usePos);

      // animations
      if (jObject["animations"].isArray()) {
        for (auto jAnimation : jObject["animations"].array_value) {
          auto animName = jAnimation["name"].string_value;
          auto anim = std::make_unique<Animation>(texture, animName);
          if (!jAnimation["fps"].isNull()) {
            anim->setFps(jAnimation["fps"].int_value);
          }
          std::vector<std::optional<int>> triggers;
          if (!jAnimation["triggers"].isNull()) {
            for (const auto &jtrigger : jAnimation["triggers"].array_value) {
              if (!jtrigger.isNull()) {
                auto name = jtrigger.string_value;
                auto trigger = std::strtol(name.data() + 1, nullptr, 10);
                triggers.emplace_back(trigger);
              } else {
                triggers.emplace_back(std::nullopt);
              }
            }
          }
          for (size_t i = 0; i < jAnimation["frames"].array_value.size(); i++) {
            const auto &jFrame = jAnimation["frames"].array_value.at(i);
            auto name = jFrame.string_value;
            if (!_spriteSheet.hasRect(name))
              continue;
            auto rect = _spriteSheet.getRect(name);
            auto size = _spriteSheet.getSourceSize(name);
            auto sourceRect = _spriteSheet.getSpriteSourceSize(name);
            std::optional<int> trigger = !triggers.empty() ? triggers.at(i) : std::nullopt;
            std::function<void()> callback = nullptr;
            if (trigger.has_value()) {
              auto pObj = object.get();
              callback = [trigger, pObj]() { pObj->trig(trigger.value()); };
            }
            AnimationFrame frame(rect, callback);
            frame.setName(name);
            frame.setSourceRect(sourceRect);
            frame.setSize(size);
            anim->addFrame(std::move(frame));
          }
          anim->reset();
          object->getAnims().push_back(std::move(anim));
        }

        int initState = 0;
        ScriptEngine::get(object.get(), "initState", initState);
        object->setStateAnimIndex(initState);
      }
      object->setRoom(_pRoom);
      _layers[0]->addEntity(*object);
      _objects.push_back(std::move(object));
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

  void loadScalings(GGPackValue &jWimpy) {
    if (jWimpy["scaling"].isArray()) {
      if (jWimpy["scaling"][0].isString()) {
        RoomScaling scaling;
        for (const auto &jScaling : jWimpy["scaling"].array_value) {
          auto value = jScaling.string_value;
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
        for (auto jScaling : jWimpy["scaling"].array_value) {
          RoomScaling scaling;
          if (jScaling["trigger"].isString()) {
            scaling.setTrigger(jScaling["trigger"].string_value);
          }
          for (const auto &jSubScaling : jScaling["scaling"].array_value) {
            if (jSubScaling.isString()) {
              auto value = jSubScaling.string_value;
              auto index = value.find('@');
              auto scale = std::strtof(value.substr(0, index).c_str(), nullptr);
              auto yPos = std::strtof(value.substr(index + 1).c_str(), nullptr);
              Scaling s{};
              s.scale = scale;
              s.yPos = yPos;
              scaling.getScalings().push_back(s);
            } else if (jSubScaling.isArray()) {
              for (const auto &jSubScalingScaling : jSubScaling.array_value) {
                auto value = jSubScalingScaling.string_value;
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

  void loadWalkboxes(const GGPackValue &jWimpy) {
    for (auto jWalkbox : jWimpy["walkboxes"].array_value) {
      std::vector<sf::Vector2i> vertices;
      auto polygon = jWalkbox["polygon"].string_value;
      _parsePolygon(polygon, vertices);
      Walkbox walkbox(vertices);
      if (jWalkbox["name"].isString()) {
        auto walkboxName = jWalkbox["name"].string_value;
        walkbox.setName(walkboxName);
      }
      _walkboxes.push_back(walkbox);
    }
    _pf.reset();
  }

  static void toPath(const Walkbox &walkbox, ClipperLib::Path &path) {
    const auto &vertices = walkbox.getVertices();
    std::transform(vertices.rbegin(),
                   vertices.rend(),
                   std::back_inserter(path),
                   [](const auto &p) -> ClipperLib::IntPoint {
                     return {p.x, p.y};
                   });
  }

  bool updateGraph(const sf::Vector2f &start) {
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
      std::vector<sf::Vector2i> sPoints;
      std::transform(sol.begin(), sol.end(), std::back_inserter(sPoints), [](auto &p) -> sf::Vector2i {
        return sf::Vector2i(p.X, p.Y);
      });
      bool isEnabled = ClipperLib::Orientation(sol);
      if (!isEnabled) {
        std::reverse(sPoints.begin(), sPoints.end());
      }
      Walkbox walkbox(sPoints);
      walkbox.setEnabled(isEnabled);
      _graphWalkboxes.push_back(walkbox);
    }
  }

  bool sortWalkboxes(const sf::Vector2f &start) {
    auto it = std::find_if(_graphWalkboxes.begin(), _graphWalkboxes.end(), [start](auto &w) {
      return w.inside(start);
    });
    if (it != _graphWalkboxes.end()) {
      std::iter_swap(_graphWalkboxes.begin(), it);
    }
    _pf = std::make_shared<PathFinder>(_graphWalkboxes);
    return true;
  }

  void drawFade(sf::RenderTarget &target) const {
    sf::RectangleShape fadeShape;
    auto screen = target.getView().getSize();
    fadeShape.setSize(sf::Vector2f(screen.x, screen.y));
    fadeShape.setFillColor(_overlayColor);
    target.draw(fadeShape);
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

std::vector<std::unique_ptr<Light>> &Room::getLights() { return pImpl->_lights; }

std::vector<Walkbox> &Room::getWalkboxes() { return pImpl->_walkboxes; }

const Walkbox *Room::getWalkbox(const std::string &name) const {
  auto it = std::find_if(pImpl->_walkboxes.begin(), pImpl->_walkboxes.end(), [&name](const Walkbox &w) {
    return w.getName() == name;
  });
  if (it != pImpl->_walkboxes.end()) {
    return &(*it);
  }
  return nullptr;
}

std::vector<Walkbox> &Room::getGraphWalkboxes() { return pImpl->_graphWalkboxes; }

sf::Vector2i Room::getRoomSize() const { return pImpl->_roomSize; }

int32_t Room::getScreenHeight() const { return pImpl->_screenHeight; }

int32_t Room::getFullscreen() const { return pImpl->_fullscreen; }

HSQOBJECT &Room::getTable() { return pImpl->_roomTable; }

void Room::setAmbientLight(sf::Color color) { pImpl->_ambientColor = color; }

sf::Color Room::getAmbientLight() const { return pImpl->_ambientColor; }

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

  GGPackValue hash;
  Locator<EngineSettings>::get().readEntry(wimpyFilename, hash);

#if 0
  std::ofstream out;
  out.open(wimpyFilename, std::ios::out);
  out << hash;
  out.close();
#endif

  pImpl->_sheet = hash["sheet"].string_value;
  pImpl->_screenHeight = hash["height"].int_value;
  pImpl->_roomSize = (sf::Vector2i) _parsePos(hash["roomsize"].string_value);

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

  object->getFont().loadFromFile(path);
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

Object &Room::createObject(const std::string &sheet, const std::vector<std::string> &anims) {
  auto texture = pImpl->_textureManager.getTexture(sheet);

  // load json file
  std::string jsonFilename;
  jsonFilename.append(sheet).append(".json");
  std::vector<char> buffer;
  Locator<EngineSettings>::get().readEntry(jsonFilename, buffer);
  auto json = ng::Json::Parser::parse(buffer);

  auto object = std::make_unique<Object>();
  auto animation = std::make_unique<Animation>(*texture, "state0");
  for (auto n : anims) {
    if (json["frames"][n].isNull())
      continue;

    checkLanguage(n);

    auto frame = json["frames"][n]["frame"];
    auto rect = _toRect(frame);
    auto size = _toSize(json["frames"][n]["sourceSize"]);
    auto sourceRect = _toRect(json["frames"][n]["spriteSourceSize"]);
    AnimationFrame animFrame(rect);
    animFrame.setSourceRect(sourceRect);
    animFrame.setSize(size);
    animation->addFrame(std::move(animFrame));
  }
  animation->reset();
  object->getAnims().push_back(std::move(animation));

  for (auto &anim : object->getAnims()) {
    if (!anim->empty()) {
      object->setAnimation(anim->getName());
      break;
    }
  }
  auto &obj = *object;
  obj.setTemporary(true);
  obj.setRoom(this);
  obj.setZOrder(1);
  pImpl->_layers[0]->addEntity(obj);
  pImpl->_objects.push_back(std::move(object));
  return obj;
}

Object &Room::createObject(const std::string &image) {
  auto name = image;
  checkLanguage(name);

  const std::vector<std::string> anims{name};
  auto texture = pImpl->_textureManager.getTexture(name);

  auto object = std::make_unique<Object>();
  auto animation = std::make_unique<Animation>(*texture, "state0");
  auto size = texture->getSize();
  sf::IntRect rect(0, 0, size.x, size.y);
  AnimationFrame animFrame(rect);
  animFrame.setSourceRect(rect);
  animFrame.setSize(sf::Vector2i(size.x, size.y));
  animation->addFrame(std::move(animFrame));
  animation->reset();
  object->getAnims().push_back(std::move(animation));

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

const Graph *Room::getGraph() const {
  if (pImpl->_pf) {
    return pImpl->_pf->getGraph().get();
  }
  return nullptr;
}

void Room::update(const sf::Time &elapsed) {
  for (auto &&layer : pImpl->_layers) {
    layer.second->update(elapsed);
  }
}

void Room::draw(sf::RenderTarget &target, const sf::Vector2f &cameraPos) const {
  sf::RenderStates states;
  if (pImpl->_selectedEffect != RoomEffectConstants::EFFECT_NONE) {
    states.shader = &pImpl->_shader;
  }

  auto screen = target.getView().getSize();
  auto halfScreen = sf::Vector2f(screen.x / 2.f, screen.y / 2.f);

  for (const auto &layer : pImpl->_layers) {
    auto parallax = layer.second->getParallax();

    sf::Transform t;
    t.rotate(pImpl->_rotation, halfScreen.x, halfScreen.y);
    t.translate(-cameraPos.x * parallax.x, cameraPos.y * parallax.y);
    states.transform = t;
    layer.second->draw(target, states);
  }
}

void Room::drawForeground(sf::RenderTarget &target, const sf::Vector2f &cameraPos) const {
  sf::RenderStates states;
  if (pImpl->_selectedEffect != RoomEffectConstants::EFFECT_NONE) {
    states.shader = &pImpl->_shader;
  }

  auto screen = target.getView().getSize();
  auto halfScreen = sf::Vector2f(screen.x / 2.f, screen.y / 2.f);

  pImpl->drawFade(target);

  for (const auto &layer : pImpl->_layers) {
    auto parallax = layer.second->getParallax();

    sf::Transform t;
    t.rotate(pImpl->_rotation, halfScreen.x, halfScreen.y);
    t.translate(-cameraPos.x * parallax.x, cameraPos.y * parallax.y);
    states.transform = t;
    layer.second->drawForeground(target, states);
  }
}

const RoomScaling &Room::getRoomScaling() const { return pImpl->_scaling; }

void Room::setRoomScaling(const RoomScaling &scaling) { pImpl->_scaling = scaling; }

void Room::setWalkboxEnabled(const std::string &name, bool isEnabled) {
  auto it = std::find_if(pImpl->_walkboxes.begin(), pImpl->_walkboxes.end(),
                         [&name](const Walkbox &walkbox) { return walkbox.getName() == name; });
  if (it == pImpl->_walkboxes.end()) {
    error("walkbox {} has not been found", name);
    return;
  }
  it->setEnabled(isEnabled);
  pImpl->_pf.reset();
}

std::vector<RoomScaling> &Room::getScalings() { return pImpl->_scalings; }

std::vector<sf::Vector2f> Room::calculatePath(sf::Vector2f start, sf::Vector2f end) const {
  if (!pImpl->_pf) {
    if (!pImpl->updateGraph(start)) {
      return std::vector<sf::Vector2f>();
    }
  } else if (!pImpl->_graphWalkboxes.empty() && !pImpl->_graphWalkboxes[0].inside(start)) {
    if (!pImpl->sortWalkboxes(start)) {
      return std::vector<sf::Vector2f>();
    }
  }
  return pImpl->_pf->calculatePath(start, end);
}

float Room::getRotation() const { return pImpl->_rotation; }

void Room::setRotation(float angle) { pImpl->_rotation = angle; }

Light *Room::createLight(sf::Color color, sf::Vector2i pos) {
  auto light = std::make_unique<Light>(color, pos);
  Light *pLight = light.get();
  pImpl->_lights.emplace_back(std::move(light));
  return pLight;
}

void Room::exit() {
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

void Room::setOverlayColor(sf::Color color) { pImpl->_overlayColor = color; }

sf::Color Room::getOverlayColor() const { return pImpl->_overlayColor; }

const SpriteSheet &Room::getSpriteSheet() const { return pImpl->_spriteSheet; }

sf::Vector2i Room::getScreenSize() const {
  auto height = getScreenHeight();
  switch (height) {
  case 0:return getRoomSize();
  case 128:return sf::Vector2i(320, 180);
  case 172:return sf::Vector2i(428, 240);
  case 256:return sf::Vector2i(640, 360);
  default: {
    height = 180.f * height / 128.f;
    auto ratio = 320.f / 180.f;
    return sf::Vector2i(ratio * height, height);
  }
  }
}

void Room::setPseudoRoom(bool pseudoRoom) {
  pImpl->_pseudoRoom = pseudoRoom;
}

bool Room::isPseudoRoom() const {
  return pImpl->_pseudoRoom;
}

} // namespace ng
