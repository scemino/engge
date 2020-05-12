#pragma once
#include <optional>
#include "squirrel.h"
#include "Entities/Objects/Animation.hpp"
#include "Engine/Engine.hpp"
#include "Engine/EngineSettings.hpp"
#include "Engine/Function.hpp"
#include "Engine/Light.hpp"
#include "System/Locator.hpp"
#include "../Room/_RoomTrigger.hpp"

namespace ng {
class _ChangeColor : public TimeFunction {
public:
  _ChangeColor(Room *pRoom,
               sf::Color startColor,
               sf::Color endColor,
               const sf::Time &time,
               std::function<float(float)> anim = Interpolations::linear,
               bool isLooping = false)
      : TimeFunction(time),
        _pRoom(pRoom),
        _isLooping(isLooping),
        _anim(std::move(anim)),
        _a(static_cast<sf::Int16>(endColor.a - startColor.a)),
        _r(static_cast<sf::Int16>(endColor.r - startColor.r)),
        _g(static_cast<sf::Int16>(endColor.g - startColor.g)),
        _b(static_cast<sf::Int16>(endColor.b - startColor.b)),
        _startColor(startColor),
        _endColor(endColor),
        _current(startColor) {
  }

  void operator()(const sf::Time &elapsed) override {
    TimeFunction::operator()(elapsed);
    _pRoom->setOverlayColor(_current);
    if (!isElapsed()) {
      auto t = _elapsed.asSeconds() / _time.asSeconds();
      auto f = _anim(t);
      _current = plusColor(_startColor, f);
    }
  }

  bool isElapsed() override {
    if (!_isLooping)
      return TimeFunction::isElapsed();
    return false;
  }

  void onElapsed() override {
    _pRoom->setOverlayColor(_endColor);
  }

private:
  sf::Color plusColor(const sf::Color &color1, float f) {
    auto a = static_cast<sf::Uint8>(color1.a + f * _a);
    auto r = static_cast<sf::Uint8>(color1.r + f * _r);
    auto g = static_cast<sf::Uint8>(color1.g + f * _g);
    auto b = static_cast<sf::Uint8>(color1.b + f * _b);
    return sf::Color(r, g, b, a);
  }

private:
  Room *_pRoom{nullptr};
  bool _isLooping;
  std::function<float(float)> _anim;
  sf::Int16 _a, _r, _g, _b;
  sf::Color _startColor;
  sf::Color _endColor;
  sf::Color _current;
};

class _RoomPack : public Pack {
private:
  static Engine *g_pEngine;

private:
  void addTo(ScriptEngine &engine) const override {
    g_pEngine = &engine.getEngine();
    engine.registerGlobalFunction(addTrigger, "addTrigger");
    engine.registerGlobalFunction(clampInWalkbox, "clampInWalkbox");
    engine.registerGlobalFunction(createLight, "createLight");
    engine.registerGlobalFunction(defineRoom, "defineRoom");
    engine.registerGlobalFunction(definePseudoRoom, "definePseudoRoom");
    engine.registerGlobalFunction(enableTrigger, "enableTrigger");
    engine.registerGlobalFunction(enterRoomFromDoor, "enterRoomFromDoor");
    engine.registerGlobalFunction(findRoom, "findRoom");
    engine.registerGlobalFunction(lightBrightness, "lightBrightness");
    engine.registerGlobalFunction(lightConeAngle, "lightConeAngle");
    engine.registerGlobalFunction(lightConeDirection, "lightConeDirection");
    engine.registerGlobalFunction(lightConeFalloff, "lightConeFalloff");
    engine.registerGlobalFunction(lightCutOffRadius, "lightCutOffRadius");
    engine.registerGlobalFunction(lightHalfRadius, "lightHalfRadius");
    engine.registerGlobalFunction(lightTurnOn, "lightTurnOn");
    engine.registerGlobalFunction(lightZRange, "lightZRange");
    engine.registerGlobalFunction(masterRoomArray, "masterRoomArray");
    engine.registerGlobalFunction(removeTrigger, "removeTrigger");
    engine.registerGlobalFunction(roomActors, "roomActors");
    engine.registerGlobalFunction(roomEffect, "roomEffect");
    engine.registerGlobalFunction(roomFade, "roomFade");
    engine.registerGlobalFunction(roomLayer, "roomLayer");
    engine.registerGlobalFunction(roomOverlayColor, "roomOverlayColor");
    engine.registerGlobalFunction(roomRotateTo, "roomRotateTo");
    engine.registerGlobalFunction(roomSize, "roomSize");
    engine.registerGlobalFunction(walkboxHidden, "walkboxHidden");
  }

  static void _fadeTo(float a, const sf::Time &time) {
    g_pEngine->fadeTo(a, time, InterpolationMethod::Linear);
  }

  static SQInteger createLight(HSQUIRRELVM v) {
    SQInteger color;
    if (SQ_FAILED(sq_getinteger(v, 2, &color))) {
      return sq_throwerror(v, _SC("failed to get color"));
    }
    SQInteger x;
    if (SQ_FAILED(sq_getinteger(v, 3, &x))) {
      return sq_throwerror(v, _SC("failed to get x"));
    }
    SQInteger y;
    if (SQ_FAILED(sq_getinteger(v, 4, &y))) {
      return sq_throwerror(v, _SC("failed to get y"));
    }
    auto pRoom = g_pEngine->getRoom();
    auto pLight = pRoom->createLight(_toColor(color), sf::Vector2i(x, y));

    ScriptEngine::pushObject(v, pLight);
    sq_getstackobj(v, -1, &pLight->getTable());
    sq_addref(v, &pLight->getTable());
    return 1;
  }

  static SQInteger lightBrightness(HSQUIRRELVM v) {
    Light *pLight;
    if (!ScriptEngine::tryGetLight(v, 2, pLight))
      return 0;

    if (!pLight)
      return sq_throwerror(v, _SC("failed to get light"));

    SQFloat brightness;
    if (SQ_FAILED(sq_getfloat(v, 3, &brightness)))
      return sq_throwerror(v, _SC("failed to get brightness"));

    pLight->setBrightness(brightness);
    return 0;
  }

  static SQInteger lightConeDirection(HSQUIRRELVM v) {
    Light *pLight;
    if (!ScriptEngine::tryGetLight(v, 2, pLight))
      return 0;

    if (!pLight)
      return sq_throwerror(v, _SC("failed to get light"));

    SQFloat direction;
    if (SQ_FAILED(sq_getfloat(v, 3, &direction))) {
      return sq_throwerror(v, _SC("failed to get direction"));
    }
    pLight->setConeDirection(direction);
    return 0;
  }

  static SQInteger lightConeAngle(HSQUIRRELVM v) {
    Light *pLight;
    if (!ScriptEngine::tryGetLight(v, 2, pLight))
      return 0;

    if (!pLight)
      return sq_throwerror(v, _SC("failed to get light"));

    SQFloat angle;
    if (SQ_FAILED(sq_getfloat(v, 3, &angle))) {
      return sq_throwerror(v, _SC("failed to get angle"));
    }
    pLight->setConeAngle(angle);
    return 0;
  }

  static SQInteger lightConeFalloff(HSQUIRRELVM v) {
    Light *pLight;
    if (!ScriptEngine::tryGetLight(v, 2, pLight))
      return 0;

    if (!pLight)
      return sq_throwerror(v, _SC("failed to get light"));

    SQFloat falloff;
    if (SQ_FAILED(sq_getfloat(v, 3, &falloff))) {
      return sq_throwerror(v, _SC("failed to get falloff"));
    }
    pLight->setConeFalloff(falloff);
    return 0;
  }

  static SQInteger lightCutOffRadius(HSQUIRRELVM v) {
    Light *pLight;
    if (!ScriptEngine::tryGetLight(v, 2, pLight))
      return 0;

    if (!pLight)
      return sq_throwerror(v, _SC("failed to get light"));

    SQFloat cutOffRadius;
    if (SQ_FAILED(sq_getfloat(v, 3, &cutOffRadius))) {
      return sq_throwerror(v, _SC("failed to get cutOffRadius"));
    }
    pLight->setCutOffRadius(cutOffRadius);
    return 0;
  }

  static SQInteger lightHalfRadius(HSQUIRRELVM v) {
    Light *pLight;
    if (!ScriptEngine::tryGetLight(v, 2, pLight))
      return 0;

    if (!pLight)
      return sq_throwerror(v, _SC("failed to get light"));

    SQFloat halfRadius;
    if (SQ_FAILED(sq_getfloat(v, 3, &halfRadius))) {
      return sq_throwerror(v, _SC("failed to get halfRadius"));
    }
    pLight->setHalfRadius(halfRadius);
    return 0;
  }

  static SQInteger lightTurnOn(HSQUIRRELVM v) {
    Light *pLight;
    if (!ScriptEngine::tryGetLight(v, 2, pLight))
      return 0;

    if (!pLight)
      return sq_throwerror(v, _SC("failed to get light"));

    SQInteger on;
    if (SQ_FAILED(sq_getinteger(v, 3, &on))) {
      return sq_throwerror(v, _SC("failed to get on"));
    }
    pLight->setOn(on != 0);
    return 0;
  }

  static SQInteger lightZRange(HSQUIRRELVM v) {
    Light *pLight;
    if (!ScriptEngine::tryGetLight(v, 2, pLight))
      return 0;

    if (!pLight)
      return sq_throwerror(v, _SC("failed to get light"));

    SQInteger nearY, farY;
    if (SQ_FAILED(sq_getinteger(v, 3, &nearY))) {
      return sq_throwerror(v, _SC("failed to get nearY"));
    }
    if (SQ_FAILED(sq_getinteger(v, 4, &farY))) {
      return sq_throwerror(v, _SC("failed to get farY"));
    }
    pLight->setZRange(nearY, farY);
    return 0;
  }

  static SQInteger masterRoomArray(HSQUIRRELVM v) {
    sq_newarray(v, 0);
    for (auto &&pRoom : g_pEngine->getRooms()) {
      sq_pushobject(v, pRoom->getTable());
      sq_arrayappend(v, -2);
    }
    return 1;
  }

  static SQInteger roomRotateTo(HSQUIRRELVM v) {
    auto pRoom = g_pEngine->getRoom();
    SQFloat rotation = 0;
    if (SQ_FAILED(sq_getfloat(v, 2, &rotation))) {
      return sq_throwerror(v, _SC("failed to get rotation"));
    }
    auto get = std::bind(&Room::getRotation, pRoom);
    auto set = std::bind(&Room::setRotation, pRoom, std::placeholders::_1);
    auto t = sf::seconds(0.200);
    auto rotateTo = std::make_unique<ChangeProperty<float>>(get, set, rotation, t);
    g_pEngine->addFunction(std::move(rotateTo));
    return 0;
  }

  static SQInteger roomSize(HSQUIRRELVM v) {
    auto pRoom = ScriptEngine::getRoom(v, 2);
    if (!pRoom) {
      return sq_throwerror(v, _SC("failed to get room"));
    }
    auto size = pRoom->getRoomSize();
    ScriptEngine::push(v, size);
    return 1;
  }

  static SQInteger addTrigger(HSQUIRRELVM v) {
    auto numArgs = sq_gettop(v);
    auto object = ScriptEngine::getObject(v, 2);
    if (!object) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    HSQOBJECT inside;
    sq_resetobject(&inside);
    if (SQ_FAILED(sq_getstackobj(v, 3, &inside))) {
      return sq_throwerror(v, _SC("failed to get insideTriggerFunction"));
    }

    HSQOBJECT outside;
    sq_resetobject(&outside);
    if (numArgs == 4) {
      if (SQ_FAILED(sq_getstackobj(v, 4, &outside))) {
        return sq_throwerror(v, _SC("failed to get outsideTriggerFunction"));
      }
    }
    auto trigger = std::make_shared<_RoomTrigger>(*g_pEngine, *object, inside, outside);
    object->addTrigger(trigger);

    return 0;
  }

  static SQInteger clampInWalkbox(HSQUIRRELVM v) {
    error("TODO: clampInWalkbox: not implemented");
    ScriptEngine::push(v, sf::Vector2i(10, 10));
    return 1;
  }

  static SQInteger enableTrigger(HSQUIRRELVM v) {
    auto object = ScriptEngine::getObject(v, 2);
    if (!object) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    SQInteger enabled = 0;
    if (SQ_FAILED(sq_getinteger(v, 3, &enabled))) {
      return sq_throwerror(v, _SC("failed to get enabled"));
    }
    object->enableTrigger(enabled != 0);
    return 0;
  }

  static SQInteger enterRoomFromDoor(HSQUIRRELVM v) {
    auto obj = ScriptEngine::getObject(v, 2);
    return g_pEngine->enterRoomFromDoor(obj);
  }

  static SQInteger roomEffect(HSQUIRRELVM v) {
    SQInteger effect = 0;
    if (SQ_FAILED(sq_getinteger(v, 2, &effect))) {
      return sq_throwerror(v, _SC("failed to get effect"));
    }
    auto pRoom = g_pEngine->getRoom();
    if (pRoom) {
      pRoom->setEffect(static_cast<int>(effect));
    }
    return 0;
  }

  static SQInteger findRoom(HSQUIRRELVM v) {
    const SQChar *name = nullptr;
    if (SQ_FAILED(sq_getstring(v, 2, &name))) {
      return sq_throwerror(v, _SC("failed to get room name"));
    }
    for (auto &&pRoom : g_pEngine->getRooms()) {
      if (pRoom->getName() == name) {
        sq_pushobject(v, pRoom->getTable());
        return 1;
      }
    }
    sq_pushnull(v);
    return 1;
  }

  static SQInteger walkboxHidden(HSQUIRRELVM v) {
    const SQChar *name = nullptr;
    if (SQ_FAILED(sq_getstring(v, 2, &name))) {
      return sq_throwerror(v, _SC("failed to get walkbox name"));
    }
    SQBool hidden;
    sq_tobool(v, 3, &hidden);
    g_pEngine->getRoom()->setWalkboxEnabled(name, hidden == SQFalse);
    return 0;
  }

  static SQInteger removeTrigger(HSQUIRRELVM v) {
    trace("removeTrigger");
    if (sq_gettype(v, 2) == OT_CLOSURE) {
      HSQOBJECT closure;
      sq_getstackobj(v, 2, &closure);
      for (auto &obj : g_pEngine->getRoom()->getObjects()) {
        auto pTrigger = obj->getTrigger();
        if (!pTrigger)
          continue;
        auto pRoomTrigger = dynamic_cast<_RoomTrigger *>(pTrigger);
        if (!pRoomTrigger)
          continue;
        if (&pRoomTrigger->getInside() == &closure) {
          obj->removeTrigger();
          return 0;
        }
        if (&pRoomTrigger->getOutside() == &closure) {
          obj->removeTrigger();
          return 0;
        }
      }
      return 0;
    }
    auto object = ScriptEngine::getObject(v, 2);
    if (!object) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    object->removeTrigger();
    return 0;
  }

  static SQInteger roomActors(HSQUIRRELVM v) {
    auto pRoom = ScriptEngine::getRoom(v, 2);
    if (!pRoom) {
      return sq_throwerror(v, _SC("failed to get room"));
    }
    sq_newarray(v, 0);
    for (auto &&pActor : g_pEngine->getActors()) {
      if (pActor->getRoom() != pRoom)
        continue;
      sq_pushobject(v, pActor->getTable());
      sq_arrayappend(v, -2);
    }
    return 1;
  }

  static SQInteger roomFade(HSQUIRRELVM v) {
    SQInteger type;
    SQFloat t;
    if (SQ_FAILED(sq_getinteger(v, 2, &type))) {
      return sq_throwerror(v, _SC("failed to get type"));
    }
    if (SQ_FAILED(sq_getfloat(v, 3, &t))) {
      return sq_throwerror(v, _SC("failed to get time"));
    }
    if (type < 2) {
      _fadeTo(type == 0 ? 0.f : 1.f, sf::seconds(t));
    } else {
      error("roomFade not implemented");
    }
    return 0;
  }

  static SQInteger roomOverlayColor(HSQUIRRELVM v) {
    trace("roomOverlayColor");
    SQInteger startColor;
    auto numArgs = sq_gettop(v);
    if (SQ_FAILED(sq_getinteger(v, 2, &startColor))) {
      return sq_throwerror(v, _SC("failed to get startColor"));
    }
    auto pRoom = g_pEngine->getRoom();
    pRoom->setOverlayColor(_toColor(startColor));
    if (numArgs == 4) {
      SQInteger endColor;
      if (SQ_FAILED(sq_getinteger(v, 3, &endColor))) {
        return sq_throwerror(v, _SC("failed to get endColor"));
      }
      SQFloat duration;
      if (SQ_FAILED(sq_getfloat(v, 4, &duration))) {
        return sq_throwerror(v, _SC("failed to get duration"));
      }
      auto fadeTo = std::make_unique<_ChangeColor>(pRoom,
                                                   _toColor(startColor),
                                                   _toColor(endColor),
                                                   sf::seconds(duration),
                                                   Interpolations::linear,
                                                   false);
      g_pEngine->addFunction(std::move(fadeTo));
    }
    return 0;
  }

  static void setObjectSlot(HSQUIRRELVM v, const SQChar *name, Object &object) {
    sq_pushstring(v, name, -1);
    ScriptEngine::pushObject(v, &object);
    sq_pushstring(v, _SC("name"), -1);
    sq_pushstring(v, object.getName().c_str(), -1);
    sq_newslot(v, -3, SQFalse);
    sq_newslot(v, -3, SQFalse);
  }

  static SQInteger _defineRoom(HSQUIRRELVM v, SQInteger index, Room *pRoom, bool isPseudoRoom) {
    // oh :( this code is really ugly, I need to refactor this
    // don't be suprised if it's full of bugs :S
    auto &roomTable = pRoom->getTable();
    if (isPseudoRoom) {
      // if this is a pseudo room, we have to clone the table
      // to have a different instance by room
      sq_clone(v, index);
      sq_getstackobj(v, -1, &roomTable);
    } else {
      sq_getstackobj(v, index, &roomTable);
    }

    // loadRoom
    const char *background = nullptr;
    if (!ScriptEngine::get(pRoom, "background", background)) {
      return sq_throwerror(v, _SC("can't find background entry"));
    }

    if (!isPseudoRoom) {
      pRoom->setName(background);
    }
    pRoom->load(background);
    pRoom->setPseudoRoom(isPseudoRoom);

    // define instance
    ScriptEngine::set(pRoom, "_id", pRoom->getId());

    std::unordered_map<std::string, HSQOBJECT> roomObjects;

    // define room objects
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
        if (isPseudoRoom) {
          // if this is a pseudo room object, we have to clone the table
          // to have a different instance by room object
          sq_clone(v, -1);
        }
        if (SQ_SUCCEEDED(sq_getstackobj(v, -1, &object))) {
          sq_addref(v, &object);
          roomObjects[key] = object;
        }
        if (isPseudoRoom) {
          sq_pop(v, 1);
        }
      }
      sq_pop(v, 2); //pops key and val before the nex iteration
    }
    sq_pop(v, 1); //pops the null iterator

    for (auto &obj: pRoom->getObjects()) {
      sq_pushobject(v, roomTable);
      sq_pushstring(v, obj->getKey().c_str(), -1);
      if (SQ_FAILED(sq_rawget(v, -2))) {
        setObjectSlot(v, obj->getKey().c_str(), *obj);

        sq_pushobject(v, roomTable);
        sq_pushstring(v, obj->getKey().c_str(), -1);
        sq_rawget(v, -2);
        sq_resetobject(&obj->getTable());
        sq_getstackobj(v, -1, &obj->getTable());
        sq_addref(ScriptEngine::getVm(), &obj->getTable());
        if (!sq_istable(obj->getTable())) {
          return sq_throwerror(v, _SC("object should be a table entry"));
        }

        continue;
      }

      obj->setTouchable(true);
      sq_resetobject(&obj->getTable());
      sq_getstackobj(v, -1, &obj->getTable());
      sq_addref(ScriptEngine::getVm(), &obj->getTable());
      if (!sq_istable(obj->getTable())) {
        return sq_throwerror(v, _SC("object should be a table entry"));
      }

      int initState;
      if (ScriptEngine::get(v, obj.get(), "initState", initState)) {
        obj->setStateAnimIndex(initState);
      }
      bool initTouchable;
      if (ScriptEngine::get(v, obj.get(), "initTouchable", initTouchable)) {
        obj->setTouchable(initTouchable);
      }
      const char *objName;
      if (ScriptEngine::get(v, obj.get(), "name", objName)) {
        obj->setName(objName);
      }

//      trace("Room {}: Set object id {} to {}", pRoom->getName(), obj->getId(), obj->getKey());
      ScriptEngine::set(obj.get(), "_id", obj->getId());

      sq_pushobject(v, obj->getTable());
      sq_pushstring(v, _SC("flags"), -1);
      if (SQ_FAILED(sq_rawget(v, -2))) {
        sq_pushstring(v, _SC("flags"), -1);
        sq_pushinteger(v, 0);
        sq_newslot(v, -3, SQFalse);
      }

      sq_pushobject(v, obj->getTable());
      sq_pushobject(v, roomTable);
      sq_setdelegate(v, -2);
    }

    // don't know if this is the best way to do this
    // but it seems that room objects and inventory objects are accessible
    // from the roottable
    for (auto &roomObject: roomObjects) {
      if (!isPseudoRoom) {
        sq_pushroottable(v);
        sq_pushstring(v, roomObject.first.data(), -1);
        sq_pushobject(v, roomObject.second);
        sq_newslot(v, -3, SQFalse);
      }

      std::unique_ptr<Object> object;
      if (!ScriptEngine::rawExists(roomObject.second, "_id")) {
        object = std::make_unique<Object>(roomObject.second);
        object->setKey(roomObject.first);
//          trace("Room {}: Set object id {} to {}", pRoom->getName(), object->getId(), object->getKey());
        ScriptEngine::set(object.get(), "_id", object->getId());

        if (!ScriptEngine::rawExists(object.get(), "icon")) {
          sq_pushobject(v, object->getTable());
          sq_pushobject(v, roomTable);
          sq_setdelegate(v, -2);

          pRoom->getObjects().push_back(std::move(object));
          continue;
        }
      }

      sq_pushobject(v, roomObject.second);
      sq_pushstring(v, _SC("icon"), -1);
      if (SQ_SUCCEEDED(sq_rawget(v, -2))) {
        object->setTouchable(true);

        if (sq_gettype(v, -1) == OT_STRING) {
          const SQChar *icon = nullptr;
          sq_getstring(v, -1, &icon);
          object->setIcon(icon);
        } else if (sq_gettype(v, -1) == OT_ARRAY) {
          SQInteger fps = 0;
          const SQChar *icon = nullptr;
          std::vector<std::string> icons;
          sq_pushnull(v); // null iterator
          if (SQ_SUCCEEDED(sq_next(v, -2))) {
            sq_getinteger(v, -1, &fps);
            sq_pop(v, 2);
          }
          while (SQ_SUCCEEDED(sq_next(v, -2))) {
            sq_getstring(v, -1, &icon);
            icons.emplace_back(icon);
            sq_pop(v, 2);
          }
          sq_pop(v, 1); // pops the null iterator

          object->setIcon(fps, icons);
        } else {
          error("TODO: objectIcon with type {} not implemented", sq_gettype(v, -1));
        }

        const char *objName;
        if (ScriptEngine::get(v, object.get(), "name", objName)) {
          object->setName(objName);
          trace("inventory object {} {} {}", roomObject.first, objName, object->getId());
        }

        sq_pushobject(v, object->getTable());
        sq_pushobject(v, roomTable);
        sq_setdelegate(v, -2);

        pRoom->getObjects().push_back(std::move(object));
      }
    }
    return 0;
  }

  static SQInteger definePseudoRoom(HSQUIRRELVM v) {
    const SQChar *name = nullptr;
    if (SQ_FAILED(sq_getstring(v, 2, &name))) {
      return sq_throwerror(v, _SC("failed to get name"));
    }
    auto pRoom = std::make_unique<Room>(g_pEngine->getTextureManager());
    pRoom->setName(name);

    auto result = _defineRoom(v, 3, pRoom.get(), true);
    if (SQ_FAILED(result))
      return result;

    // declare pseudo room in root table
    sq_pushroottable(v);
    sq_pushstring(v, pRoom->getName().data(), -1);
    sq_pushobject(v, pRoom->getTable());
    sq_newslot(v, -3, SQFalse);

    sq_pushobject(v, pRoom->getTable());
    g_pEngine->addRoom(std::move(pRoom));
    return 1;
  }

  static SQInteger defineRoom(HSQUIRRELVM v) {
    auto pRoom = std::make_unique<Room>(g_pEngine->getTextureManager());
    auto result = _defineRoom(v, 2, pRoom.get(), false);
    if (SQ_SUCCEEDED(result)) {
      g_pEngine->addRoom(std::move(pRoom));
    }
    return result;
  }

  static SQInteger roomLayer(HSQUIRRELVM v) {
    auto pRoom = ScriptEngine::getRoom(v, 2);
    SQInteger layer;
    if (SQ_FAILED(sq_getinteger(v, 3, &layer))) {
      return sq_throwerror(v, _SC("failed to get layer"));
    }
    SQInteger enabled;
    if (SQ_FAILED(sq_getinteger(v, 4, &enabled))) {
      return sq_throwerror(v, _SC("failed to get enabled"));
    }
    pRoom->roomLayer(static_cast<int>(layer), enabled != 0);
    return 0;
  }
};

Engine *_RoomPack::g_pEngine = nullptr;

} // namespace ng