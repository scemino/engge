#pragma once
#include <optional>
#include <squirrel.h>
#include "engge/Entities/Objects/Animation.hpp"
#include "engge/Engine/Engine.hpp"
#include "engge/Engine/EngineSettings.hpp"
#include "engge/Engine/Function.hpp"
#include "engge/Engine/Light.hpp"
#include "engge/System/Locator.hpp"
#include "../Room/_RoomTrigger.hpp"

namespace ng {
class _ChangeColor : public TimeFunction {
public:
  _ChangeColor(Room *pRoom,
               ngf::Color startColor,
               ngf::Color endColor,
               const ngf::TimeSpan &time,
               std::function<float(float)> anim = Interpolations::linear,
               bool isLooping = false)
      : TimeFunction(time),
        _pRoom(pRoom),
        _isLooping(isLooping),
        _anim(std::move(anim)),
        _a(endColor.a - startColor.a),
        _r(endColor.r - startColor.r),
        _g(endColor.g - startColor.g),
        _b(endColor.b - startColor.b),
        _startColor(startColor),
        _endColor(endColor),
        _current(startColor) {
  }

  void operator()(const ngf::TimeSpan &elapsed) override {
    TimeFunction::operator()(elapsed);
    _pRoom->setOverlayColor(_current);
    if (!isElapsed()) {
      auto t = _elapsed.getTotalSeconds() / _time.getTotalSeconds();
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
  [[nodiscard]] ngf::Color plusColor(const ngf::Color &color1, float f) const {
    auto a = color1.a + f * _a;
    auto r = color1.r + f * _r;
    auto g = color1.g + f * _g;
    auto b = color1.b + f * _b;
    return ngf::Color(r, g, b, a);
  }

private:
  Room *_pRoom{nullptr};
  bool _isLooping;
  std::function<float(float)> _anim;
  float _a, _r, _g, _b;
  ngf::Color _startColor;
  ngf::Color _endColor;
  ngf::Color _current;
};

class _RoomPack : public Pack {
private:
  static Engine *g_pEngine;

private:
  void registerPack() const override {
    g_pEngine = &ScriptEngine::getEngine();
    ScriptEngine::registerGlobalFunction(addTrigger, "addTrigger");
    ScriptEngine::registerGlobalFunction(clampInWalkbox, "clampInWalkbox");
    ScriptEngine::registerGlobalFunction(createLight, "createLight");
    ScriptEngine::registerGlobalFunction(defineRoom, "defineRoom");
    ScriptEngine::registerGlobalFunction(definePseudoRoom, "definePseudoRoom");
    ScriptEngine::registerGlobalFunction(enableTrigger, "enableTrigger");
    ScriptEngine::registerGlobalFunction(enterRoomFromDoor, "enterRoomFromDoor");
    ScriptEngine::registerGlobalFunction(findRoom, "findRoom");
    ScriptEngine::registerGlobalFunction(lightBrightness, "lightBrightness");
    ScriptEngine::registerGlobalFunction(lightConeAngle, "lightConeAngle");
    ScriptEngine::registerGlobalFunction(lightConeDirection, "lightConeDirection");
    ScriptEngine::registerGlobalFunction(lightConeFalloff, "lightConeFalloff");
    ScriptEngine::registerGlobalFunction(lightCutOffRadius, "lightCutOffRadius");
    ScriptEngine::registerGlobalFunction(lightHalfRadius, "lightHalfRadius");
    ScriptEngine::registerGlobalFunction(lightTurnOn, "lightTurnOn");
    ScriptEngine::registerGlobalFunction(lightZRange, "lightZRange");
    ScriptEngine::registerGlobalFunction(masterRoomArray, "masterRoomArray");
    ScriptEngine::registerGlobalFunction(removeTrigger, "removeTrigger");
    ScriptEngine::registerGlobalFunction(roomActors, "roomActors");
    ScriptEngine::registerGlobalFunction(roomEffect, "roomEffect");
    ScriptEngine::registerGlobalFunction(roomFade, "roomFade");
    ScriptEngine::registerGlobalFunction(roomLayer, "roomLayer");
    ScriptEngine::registerGlobalFunction(roomOverlayColor, "roomOverlayColor");
    ScriptEngine::registerGlobalFunction(roomRotateTo, "roomRotateTo");
    ScriptEngine::registerGlobalFunction(roomSize, "roomSize");
    ScriptEngine::registerGlobalFunction(walkboxHidden, "walkboxHidden");
  }

  static void _fadeTo(float a, const ngf::TimeSpan &time) {
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
    auto pLight = pRoom->createLight(_toColor(color), {x, y});

    ScriptEngine::pushObject(v, pLight);
    sq_getstackobj(v, -1, &pLight->table);
    sq_addref(v, &pLight->table);
    return 1;
  }

  static SQInteger lightBrightness(HSQUIRRELVM v) {
    Light *pLight;
    if (!EntityManager::tryGetLight(v, 2, pLight))
      return 0;

    if (!pLight)
      return sq_throwerror(v, _SC("failed to get light"));

    SQFloat brightness;
    if (SQ_FAILED(sq_getfloat(v, 3, &brightness)))
      return sq_throwerror(v, _SC("failed to get brightness"));

    pLight->brightness = brightness;
    return 0;
  }

  static SQInteger lightConeDirection(HSQUIRRELVM v) {
    Light *pLight;
    if (!EntityManager::tryGetLight(v, 2, pLight))
      return 0;

    if (!pLight)
      return sq_throwerror(v, _SC("failed to get light"));

    SQFloat direction;
    if (SQ_FAILED(sq_getfloat(v, 3, &direction))) {
      return sq_throwerror(v, _SC("failed to get direction"));
    }
    pLight->coneDirection = direction;
    return 0;
  }

  static SQInteger lightConeAngle(HSQUIRRELVM v) {
    Light *pLight;
    if (!EntityManager::tryGetLight(v, 2, pLight))
      return 0;

    if (!pLight)
      return sq_throwerror(v, _SC("failed to get light"));

    SQFloat angle;
    if (SQ_FAILED(sq_getfloat(v, 3, &angle))) {
      return sq_throwerror(v, _SC("failed to get angle"));
    }
    pLight->coneAngle = angle;
    return 0;
  }

  static SQInteger lightConeFalloff(HSQUIRRELVM v) {
    Light *pLight;
    if (!EntityManager::tryGetLight(v, 2, pLight))
      return 0;

    if (!pLight)
      return sq_throwerror(v, _SC("failed to get light"));

    SQFloat falloff;
    if (SQ_FAILED(sq_getfloat(v, 3, &falloff))) {
      return sq_throwerror(v, _SC("failed to get falloff"));
    }
    pLight->coneFalloff = falloff;
    return 0;
  }

  static SQInteger lightCutOffRadius(HSQUIRRELVM v) {
    Light *pLight;
    if (!EntityManager::tryGetLight(v, 2, pLight))
      return 0;

    if (!pLight)
      return sq_throwerror(v, _SC("failed to get light"));

    SQFloat cutOffRadius;
    if (SQ_FAILED(sq_getfloat(v, 3, &cutOffRadius))) {
      return sq_throwerror(v, _SC("failed to get cutOffRadius"));
    }
    pLight->cutOffRadius = cutOffRadius;
    return 0;
  }

  static SQInteger lightHalfRadius(HSQUIRRELVM v) {
    Light *pLight;
    if (!EntityManager::tryGetLight(v, 2, pLight))
      return 0;

    if (!pLight)
      return sq_throwerror(v, _SC("failed to get light"));

    SQFloat halfRadius;
    if (SQ_FAILED(sq_getfloat(v, 3, &halfRadius))) {
      return sq_throwerror(v, _SC("failed to get halfRadius"));
    }
    pLight->halfRadius = halfRadius;
    return 0;
  }

  static SQInteger lightTurnOn(HSQUIRRELVM v) {
    Light *pLight;
    if (!EntityManager::tryGetLight(v, 2, pLight))
      return 0;

    if (!pLight)
      return sq_throwerror(v, _SC("failed to get light"));

    SQInteger on;
    if (SQ_FAILED(sq_getinteger(v, 3, &on))) {
      return sq_throwerror(v, _SC("failed to get on"));
    }
    pLight->on = (on != 0);
    return 0;
  }

  static SQInteger lightZRange(HSQUIRRELVM v) {
    Light *pLight;
    if (!EntityManager::tryGetLight(v, 2, pLight))
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
    // TODO: ZRange ??
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
    auto get = [pRoom] { return pRoom->getRotation(); };
    auto set = [pRoom](float value) { pRoom->setRotation(value); };
    auto t = ngf::TimeSpan::seconds(0.200);
    auto rotateTo = std::make_unique<ChangeProperty<float>>(get, set, rotation, t);
    g_pEngine->addFunction(std::move(rotateTo));
    return 0;
  }

  static SQInteger roomSize(HSQUIRRELVM v) {
    auto pRoom = EntityManager::getRoom(v, 2);
    if (!pRoom) {
      return sq_throwerror(v, _SC("failed to get room"));
    }
    auto size = pRoom->getRoomSize();
    ScriptEngine::push(v, size);
    return 1;
  }

  static SQInteger addTrigger(HSQUIRRELVM v) {
    auto numArgs = sq_gettop(v);
    auto object = EntityManager::getObject(v, 2);
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
    SQInteger x = 0;
    if (SQ_FAILED(sq_getinteger(v, 2, &x))) {
      return sq_throwerror(v, _SC("failed to get x"));
    }
    SQInteger y = 0;
    if (SQ_FAILED(sq_getinteger(v, 3, &y))) {
      return sq_throwerror(v, _SC("failed to get y"));
    }
    ScriptEngine::push(v, glm::ivec2(x, y));
    return 1;
  }

  static SQInteger enableTrigger(HSQUIRRELVM v) {
    auto object = EntityManager::getObject(v, 2);
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
    auto obj = EntityManager::getObject(v, 2);
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
    info("findRoom({}) -> null", name);
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
    auto object = EntityManager::getObject(v, 2);
    if (!object) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    object->removeTrigger();
    return 0;
  }

  static SQInteger roomActors(HSQUIRRELVM v) {
    auto pRoom = EntityManager::getRoom(v, 2);
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
      _fadeTo(type == 0 ? 0.f : 1.f, ngf::TimeSpan::seconds(t));
    } else {
      error("roomFade not implemented");
    }
    return 0;
  }

  static SQInteger roomOverlayColor(HSQUIRRELVM v) {
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
                                                   ngf::TimeSpan::seconds(duration),
                                                   Interpolations::linear,
                                                   false);
      g_pEngine->addFunction(std::move(fadeTo));
    }
    return 0;
  }

  static SQInteger _defineRoom(HSQUIRRELVM v, HSQOBJECT roomTable, const char *name = nullptr) {
    auto pRoom = Room::define(roomTable, name);
    sq_pushobject(v, pRoom->getTable());
    trace("Define room {}", pRoom->getName());
    g_pEngine->addRoom(std::move(pRoom));
    return 1;
  }

  static SQInteger defineRoom(HSQUIRRELVM v) {
    HSQOBJECT roomTable;
    sq_resetobject(&roomTable);
    sq_getstackobj(v, 2, &roomTable);

    return _defineRoom(v, roomTable);
  }

  static SQInteger definePseudoRoom(HSQUIRRELVM v) {
    const SQChar *name = nullptr;
    if (SQ_FAILED(sq_getstring(v, 2, &name))) {
      return sq_throwerror(v, _SC("failed to get name"));
    }

    // if this is a pseudo room, we have to clone the table
    // to have a different instance by room
    HSQOBJECT roomTable;
    sq_resetobject(&roomTable);
    sq_clone(v, 3);
    sq_getstackobj(v, -1, &roomTable);

    return _defineRoom(v, roomTable, name);
  }

  static SQInteger roomLayer(HSQUIRRELVM v) {
    auto pRoom = EntityManager::getRoom(v, 2);
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