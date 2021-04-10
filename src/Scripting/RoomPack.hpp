#pragma once
#include <optional>
#include <squirrel.h>
#include "engge/Engine/Engine.hpp"
#include "engge/Engine/EngineSettings.hpp"
#include "engge/Engine/Function.hpp"
#include "engge/Engine/Light.hpp"
#include "engge/System/Locator.hpp"
#include "../Room/RoomTrigger.hpp"

namespace ng {
class ChangeColor final : public TimeFunction {
public:
  ChangeColor(Room *pRoom,
              ngf::Color startColor,
              ngf::Color endColor,
              const ngf::TimeSpan &time,
              std::function<float(float)> anim = Interpolations::linear,
              bool isLooping = false)
      : TimeFunction(time),
        m_pRoom(pRoom),
        m_isLooping(isLooping),
        m_anim(std::move(anim)),
        m_a(endColor.a - startColor.a),
        m_r(endColor.r - startColor.r),
        m_g(endColor.g - startColor.g),
        m_b(endColor.b - startColor.b),
        m_startColor(startColor),
        m_endColor(endColor),
        m_current(startColor) {
  }

  void operator()(const ngf::TimeSpan &elapsed) override {
    TimeFunction::operator()(elapsed);
    m_pRoom->setOverlayColor(m_current);
    if (!isElapsed()) {
      auto t = m_elapsed.getTotalSeconds() / m_time.getTotalSeconds();
      auto f = m_anim(t);
      m_current = plusColor(m_startColor, f);
    }
  }

  bool isElapsed() override {
    if (!m_isLooping)
      return TimeFunction::isElapsed();
    return false;
  }

  void onElapsed() override {
    m_pRoom->setOverlayColor(m_endColor);
  }

private:
  [[nodiscard]] ngf::Color plusColor(const ngf::Color &color1, float f) const {
    auto a = color1.a + f * m_a;
    auto r = color1.r + f * m_r;
    auto g = color1.g + f * m_g;
    auto b = color1.b + f * m_b;
    return ngf::Color(r, g, b, a);
  }

private:
  Room *m_pRoom{nullptr};
  bool m_isLooping;
  std::function<float(float)> m_anim;
  float m_a, m_r, m_g, m_b;
  ngf::Color m_startColor;
  ngf::Color m_endColor;
  ngf::Color m_current;
};

class RoomPack final : public Pack {
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
    auto pLight = pRoom->createLight(fromRgba(color), {x, y});

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
    auto trigger = std::make_shared<RoomTrigger>(*g_pEngine, *object, inside, outside);
    object->addTrigger(trigger);

    return 0;
  }

  static SQInteger clampInWalkbox(HSQUIRRELVM v) {
    auto numArgs = sq_gettop(v);
    glm::vec2 pos1, pos2;
    if (numArgs == 3) {
      SQInteger x = 0;
      if (SQ_FAILED(sq_getinteger(v, 2, &x))) {
        return sq_throwerror(v, _SC("failed to get x"));
      }
      SQInteger y = 0;
      if (SQ_FAILED(sq_getinteger(v, 3, &y))) {
        return sq_throwerror(v, _SC("failed to get y"));
      }
      pos1 = {x, y};
      auto pActor = g_pEngine->getCurrentActor();
      pos2 = pActor->getPosition();
    } else if (numArgs == 5) {
      SQInteger x1 = 0;
      if (SQ_FAILED(sq_getinteger(v, 2, &x1))) {
        return sq_throwerror(v, _SC("failed to get x1"));
      }
      SQInteger y1 = 0;
      if (SQ_FAILED(sq_getinteger(v, 3, &y1))) {
        return sq_throwerror(v, _SC("failed to get y1"));
      }
      pos1 = {x1, y1};
      SQInteger x2 = 0;
      if (SQ_FAILED(sq_getinteger(v, 2, &x2))) {
        return sq_throwerror(v, _SC("failed to get x2"));
      }
      SQInteger y2 = 0;
      if (SQ_FAILED(sq_getinteger(v, 3, &y1))) {
        return sq_throwerror(v, _SC("failed to get y2"));
      }
      pos2 = {x2, y2};
    } else {
      return sq_throwerror(v, _SC("Invalid argument number in clampInWalkbox"));
    }
    auto walkboxes = g_pEngine->getRoom()->getWalkboxes();
    auto it = std::find_if(walkboxes.cbegin(), walkboxes.cend(), [pos2](const auto &walkbox) {
      return walkbox.inside(pos2);
    });
    if (it == walkboxes.cend()) {
      error("Actor's walkbox has not been found.");
      ScriptEngine::push(v, glm::ivec2(pos1));
    } else {
      auto pos = it->getClosestPointOnEdge(pos1);
      ScriptEngine::push(v, pos);
    }
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
    auto count = sq_gettop(v);
    if (SQ_FAILED(sq_getinteger(v, 2, &effect))) {
      return sq_throwerror(v, _SC("failed to get effect"));
    }
    auto pRoom = g_pEngine->getRoom();
    if (!pRoom)
      return 0;
    if (count == 14) {
      SQInteger unused;
      sq_getfloat(v, 3, &g_pEngine->roomEffect.iFade);
      sq_getfloat(v, 4, &g_pEngine->roomEffect.wobbleIntensity);
      sq_getinteger(v, 5, &unused);
      sq_getfloat(v, 6, &g_pEngine->roomEffect.shadows.r);
      sq_getfloat(v, 7, &g_pEngine->roomEffect.shadows.g);
      sq_getfloat(v, 8, &g_pEngine->roomEffect.shadows.b);
      sq_getfloat(v, 9, &g_pEngine->roomEffect.midtones.r);
      sq_getfloat(v, 10, &g_pEngine->roomEffect.midtones.g);
      sq_getfloat(v, 11, &g_pEngine->roomEffect.midtones.b);
      sq_getfloat(v, 12, &g_pEngine->roomEffect.highlights.r);
      sq_getfloat(v, 13, &g_pEngine->roomEffect.highlights.g);
      sq_getfloat(v, 14, &g_pEngine->roomEffect.highlights.b);
    } else {
      g_pEngine->roomEffect.reset();
    }
    pRoom->setEffect(static_cast<int>(effect));
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
        auto pRoomTrigger = dynamic_cast<RoomTrigger *>(pTrigger);
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
    FadeEffect effect;
    switch (type) {
    case 0:
      effect = FadeEffect::In;
      break;
    case 1:
      effect = FadeEffect::Out;
      break;
    case 2:
      effect = FadeEffect::Wobble;
      break;
    default: {
      error("roomFade not implemented");
      return 0;
    }
    }
    g_pEngine->fadeTo(effect, ngf::TimeSpan::seconds(t));
    return 0;
  }

  static SQInteger roomOverlayColor(HSQUIRRELVM v) {
    SQInteger startColor;
    auto numArgs = sq_gettop(v);
    if (SQ_FAILED(sq_getinteger(v, 2, &startColor))) {
      return sq_throwerror(v, _SC("failed to get startColor"));
    }
    auto pRoom = g_pEngine->getRoom();
    pRoom->setOverlayColor(fromRgba(startColor));
    if (numArgs == 4) {
      SQInteger endColor;
      if (SQ_FAILED(sq_getinteger(v, 3, &endColor))) {
        return sq_throwerror(v, _SC("failed to get endColor"));
      }
      SQFloat duration;
      if (SQ_FAILED(sq_getfloat(v, 4, &duration))) {
        return sq_throwerror(v, _SC("failed to get duration"));
      }
      auto fadeTo = std::make_unique<ChangeColor>(pRoom,
                                                  fromRgba(startColor),
                                                  fromRgba(endColor),
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

Engine *RoomPack::g_pEngine = nullptr;

} // namespace ng