#pragma once
#include "../System/_Util.hpp"
#include "Engine/Engine.hpp"
#include "Parsers/Lip.hpp"
#include "squirrel.h"

namespace ng {
class _ActorPack : public Pack {
private:
  static Engine *g_pEngine;

private:
  void addTo(ScriptEngine &engine) const override {
    g_pEngine = &engine.getEngine();
    engine.registerGlobalFunction(actorAlpha, "actorAlpha");
    engine.registerGlobalFunction(actorAnimationNames, "actorAnimationNames");
    engine.registerGlobalFunction(actorAnimationFlags, "actorAnimationFlags");
    engine.registerGlobalFunction(actorAt, "actorAt");
    engine.registerGlobalFunction(actorBlinkRate, "actorBlinkRate");
    engine.registerGlobalFunction(actorColor, "actorColor");
    engine.registerGlobalFunction(actorCostume, "actorCostume");
    engine.registerGlobalFunction(actorDistanceTo, "actorDistanceTo");
    engine.registerGlobalFunction(actorDistanceWithin, "actorDistanceWithin");
    engine.registerGlobalFunction(actorFace, "actorFace");
    engine.registerGlobalFunction(actorHidden, "actorHidden");
    engine.registerGlobalFunction(actorHideLayer, "actorHideLayer");
    engine.registerGlobalFunction(actorInTrigger, "actorInTrigger");
    engine.registerGlobalFunction(actorInWalkbox, "actorInWalkbox");
    engine.registerGlobalFunction(actorLockFacing, "actorLockFacing");
    engine.registerGlobalFunction(actorPlayAnimation, "actorPlayAnimation");
    engine.registerGlobalFunction(actorPosX, "actorPosX");
    engine.registerGlobalFunction(actorPosY, "actorPosY");
    engine.registerGlobalFunction(actorRenderOffset, "actorRenderOffset");
    engine.registerGlobalFunction(actorRoom, "actorRoom");
    engine.registerGlobalFunction(actorShowLayer, "actorShowLayer");
    engine.registerGlobalFunction(actorSlotSelectable, "actorSlotSelectable");
    engine.registerGlobalFunction(actorStand, "actorStand");
    engine.registerGlobalFunction(actorStopWalking, "actorStopWalking");
    engine.registerGlobalFunction(actorTalkColors, "actorTalkColors");
    engine.registerGlobalFunction(actorTalkOffset, "actorTalkOffset");
    engine.registerGlobalFunction(actorTalking, "actorTalking");
    engine.registerGlobalFunction(actorTurnTo, "actorTurnTo");
    engine.registerGlobalFunction(actorUsePos, "actorUsePos");
    engine.registerGlobalFunction(actorUseWalkboxes, "actorUseWalkboxes");
    engine.registerGlobalFunction(actorVolume, "actorVolume");
    engine.registerGlobalFunction(actorWalkForward, "actorWalkForward");
    engine.registerGlobalFunction(actorWalkSpeed, "actorWalkSpeed");
    engine.registerGlobalFunction(actorWalkTo, "actorWalkTo");
    engine.registerGlobalFunction(actorWalking, "actorWalking");
    engine.registerGlobalFunction(addSelectableActor, "addSelectableActor");
    engine.registerGlobalFunction(createActor, "createActor");
    engine.registerGlobalFunction(flashSelectableActor, "flashSelectableActor");
    engine.registerGlobalFunction(isActor, "isActor");
    engine.registerGlobalFunction(isActorOnScreen, "isActorOnScreen");
    engine.registerGlobalFunction(isActor, "is_actor");
    engine.registerGlobalFunction(isActorSelectable, "isActorSelectable");
    engine.registerGlobalFunction(masterActorArray, "masterActorArray");
    engine.registerGlobalFunction(mumbleLine, "mumbleLine");
    engine.registerGlobalFunction(sayLine, "sayLine");
    engine.registerGlobalFunction(sayLineAt, "sayLineAt");
    engine.registerGlobalFunction(selectActor, "selectActor");
    engine.registerGlobalFunction(stopTalking, "stopTalking");
    engine.registerGlobalFunction(triggerActors, "triggerActors");
    engine.registerGlobalFunction(verbUIColors, "verbUIColors");
  }

  static SQInteger actorAlpha(HSQUIRRELVM v) {
    auto actor = ScriptEngine::getActor(v, 2);
    if (!actor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }
    SQFloat transparency;
    if (SQ_FAILED(sq_getfloat(v, 3, &transparency))) {
      return sq_throwerror(v, _SC("failed to get transparency"));
    }
    auto alpha = static_cast<sf::Uint8>(transparency * 255);
    auto color = actor->getColor();
    color.a = alpha;
    actor->setColor(color);
    return 0;
  }

  static SQInteger actorAnimationNames(HSQUIRRELVM v) {
    const SQChar *head = nullptr;
    const SQChar *stand = nullptr;
    const SQChar *walk = nullptr;
    const SQChar *reach = nullptr;
    auto *pActor = ScriptEngine::getActor(v, 2);
    if (!pActor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }
    HSQOBJECT obj;
    sq_resetobject(&obj);
    sq_getstackobj(v, 3, &obj);

    sq_pushobject(v, obj);
    sq_pushstring(v, _SC("head"), -1);
    if (SQ_SUCCEEDED(sq_get(v, -2))) {
      sq_getstring(v, -1, &head);
    }
    sq_pop(v, 1);

    sq_pushstring(v, _SC("stand"), -1);
    if (SQ_SUCCEEDED(sq_get(v, -2))) {
      sq_getstring(v, -1, &stand);
    }
    sq_pop(v, 1);

    sq_pushstring(v, _SC("walk"), -1);
    if (SQ_SUCCEEDED(sq_get(v, -2))) {
      sq_getstring(v, -1, &walk);
    }
    sq_pop(v, 1);

    sq_pushstring(v, _SC("reach"), -1);
    if (SQ_SUCCEEDED(sq_get(v, -2))) {
      sq_getstring(v, -1, &reach);
    }
    sq_pop(v, 1);

    pActor->getCostume().setAnimationNames(head ? head : "", stand ? stand : "", walk ? walk : "",
                                           reach ? reach : "");
    return 0;
  }

  static SQInteger actorAnimationFlags(HSQUIRRELVM v) {
    auto *pActor = ScriptEngine::getActor(v, 2);
    if (!pActor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }
    auto pAnim = pActor->getCostume().getAnimation();
    if (!pAnim) {
      sq_pushinteger(v, 0);
      return 1;
    }
    auto flags = pAnim->getFlags();
    sq_pushinteger(v, flags);
    return 1;
  }

  static Facing _getFacing(SQInteger dir, Facing currentFacing) {
    if (dir == 0x10) {
      switch (currentFacing) {
      case Facing::FACE_BACK:return Facing::FACE_FRONT;
      case Facing::FACE_FRONT:return Facing::FACE_BACK;
      case Facing::FACE_LEFT:return Facing::FACE_RIGHT;
      case Facing::FACE_RIGHT:return Facing::FACE_LEFT;
      default:throw std::invalid_argument("currentFacing is invalid");
      }
    } else {
      switch (currentFacing) {
      case Facing::FACE_BACK:return Facing::FACE_BACK;
      case Facing::FACE_FRONT:return Facing::FACE_FRONT;
      case Facing::FACE_LEFT:return Facing::FACE_LEFT;
      case Facing::FACE_RIGHT:return Facing::FACE_RIGHT;
      default:throw std::invalid_argument("currentFacing is invalid");
      }
    }
  }

  static SQInteger actorAt(HSQUIRRELVM v) {
    auto numArgs = sq_gettop(v);
    switch (numArgs) {
    case 3: {
      auto *pActor = ScriptEngine::getActor(v, 2);
      if (!pActor) {
        return sq_throwerror(v, _SC("failed to get actor"));
      }

      auto *pRoom = ScriptEngine::getRoom(v, 3);
      if (pRoom) {
        pActor->setRoom(pRoom);
        return 0;
      }

      auto *pObj = ScriptEngine::getObject(v, 3);
      if (!pObj) {
        return sq_throwerror(v, _SC("failed to get object or room"));
      }
      auto pos = pObj->getRealPosition() + pObj->getUsePosition().value_or(sf::Vector2f());
      pRoom = pObj->getRoom();
      pActor->setRoom(pRoom);
      pActor->setPosition(pos);
      pActor->getCostume().setFacing(_toFacing(pObj->getUseDirection()));
      return 0;
    }

    case 4: {
      auto *pActor = ScriptEngine::getActor(v, 2);
      if (!pActor) {
        return sq_throwerror(v, _SC("failed to get actor"));
      }
      SQInteger x, y;
      if (SQ_FAILED(sq_getinteger(v, 3, &x))) {
        return sq_throwerror(v, _SC("failed to get x"));
      }
      if (SQ_FAILED(sq_getinteger(v, 4, &y))) {
        return sq_throwerror(v, _SC("failed to get y"));
      }
      pActor->setPosition(sf::Vector2f(x, y));
      return 0;
    }
    case 5: [[fallthrough]];
    case 6: {
      auto *pActor = ScriptEngine::getActor(v, 2);
      if (!pActor) {
        return sq_throwerror(v, _SC("failed to get actor"));
      }
      auto *pRoom = ScriptEngine::getRoom(v, 3);
      if (!pRoom) {
        return sq_throwerror(v, _SC("failed to get room"));
      }
      SQInteger x, y, dir = 0;
      if (SQ_FAILED(sq_getinteger(v, 4, &x))) {
        return sq_throwerror(v, _SC("failed to get x"));
      }
      if (SQ_FAILED(sq_getinteger(v, 5, &y))) {
        return sq_throwerror(v, _SC("failed to get y"));
      }
      if (numArgs == 6 && SQ_FAILED(sq_getinteger(v, 6, &dir))) {
        return sq_throwerror(v, _SC("failed to get direction"));
      }
      auto facing = _getFacing(dir, pActor->getCostume().getFacing());
      pActor->setPosition(sf::Vector2f(x, y));
      pActor->getCostume().setFacing(facing);
      pActor->setRoom(pRoom);
      return 0;
    }
    }

    return sq_throwerror(v, _SC("invalid number of arguments"));
  }

  static SQInteger flashSelectableActor(HSQUIRRELVM v) {
    SQInteger on;
    if (SQ_FAILED(sq_getinteger(v, 2, &on))) {
      return sq_throwerror(v, _SC("failed to get on"));
    }
    g_pEngine->flashSelectableActor(on != 0);
    return 0;
  }

  static SQInteger actorColor(HSQUIRRELVM v) {
    auto actor = ScriptEngine::getActor(v, 2);
    if (!actor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }
    SQInteger c;
    if (SQ_FAILED(sq_getinteger(v, 3, &c))) {
      return sq_throwerror(v, _SC("failed to get color"));
    }
    auto alpha = actor->getColor().a;
    auto color = _fromRgb(c);
    color.a = alpha;
    actor->setColor(color);
    return 0;
  }

  static SQInteger actorCostume(HSQUIRRELVM v) {
    const SQChar *name;
    auto actor = ScriptEngine::getActor(v, 2);
    if (!actor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }
    if (SQ_FAILED(sq_getstring(v, 3, &name))) {
      return sq_throwerror(v, _SC("failed to get name"));
    }
    const SQChar *pSheet = nullptr;
    sq_getstring(v, 4, &pSheet);
    actor->setCostume(name, pSheet ? pSheet : "");
    return 0;
  }

  static SQInteger actorDistanceTo(HSQUIRRELVM v) {
    auto actor = ScriptEngine::getActor(v, 2);
    if (!actor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }
    auto object = ScriptEngine::getObject(v, 3);
    if (!object) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    auto pos = actor->getRealPosition() - object->getRealPosition();
    auto dist = sqrt(pos.x * pos.x + pos.y * pos.y);
    sq_pushinteger(v, dist);
    return 1;
  }

  static SQInteger actorDistanceWithin(HSQUIRRELVM v) {
    auto actor = ScriptEngine::getActor(v, 2);
    if (!actor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }
    SQInteger d;
    if (SQ_FAILED(sq_getinteger(v, 4, &d))) {
      return sq_throwerror(v, _SC("failed to get distance"));
    }
    auto pActor2 = ScriptEngine::getActor(v, 3);
    if (pActor2) {
      auto dist = _distance(actor->getRealPosition(), pActor2->getRealPosition());
      trace("actorDistanceWithin({},{},{})=>{} ({})",
            actor->getKey(),
            pActor2->getKey(),
            d,
            (dist < d) ? "YES" : "NO",
            dist);
      sq_pushbool(v, dist < d);
      return 1;
    }

    auto pObject = ScriptEngine::getObject(v, 3);
    if (!pObject) {
      return sq_throwerror(v, _SC("failed to get object or actor"));
    }

    auto posObj = pObject->getRealPosition() + pObject->getUsePosition().value_or(sf::Vector2f());
    auto dist = _distance(actor->getRealPosition(), posObj);
    trace("actorDistanceWithin({},{},{})=>{} ({})",
          actor->getKey(),
          pObject->getName(),
          d,
          (dist < d) ? "YES" : "NO",
          dist);
    sq_pushbool(v, dist < d);
    return 1;
  }

  static float _distance(sf::Vector2f p1, sf::Vector2f p2) {
    auto pos = p1 - p2;
    return sqrt(pos.x * pos.x + pos.y * pos.y);
  }

  static SQInteger actorFace(HSQUIRRELVM v) {
    auto actor = ScriptEngine::getActor(v, 2);
    if (!actor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }
    auto count = sq_gettop(v);
    SQInteger dir;
    if (count == 2) {
      dir = (SQInteger) actor->getCostume().getFacing();
      sq_pushinteger(v, dir);
      return 1;
    }

    if (sq_gettype(v, 3) == OT_INTEGER) {
      if (SQ_FAILED(sq_getinteger(v, 3, &dir))) {
        return sq_throwerror(v, _SC("failed to get direction"));
      }
      // FACE_FLIP ?
      if (dir == 0x10) {
        auto facing = _flip(actor->getCostume().getFacing());
        actor->getCostume().setFacing(facing);
        return 0;
      }
      actor->getCostume().setFacing((Facing) dir);
      return 0;
    }

    auto actor2 = ScriptEngine::getActor(v, 3);
    if (!actor2) {
      return sq_throwerror(v, _SC("failed to get actor to face to"));
    }
    auto facing = _getFacingToFaceTo(actor, actor2);
    actor->getCostume().setFacing(facing);
    return 0;
  }

  static SQInteger actorHidden(HSQUIRRELVM v) {
    auto *pActor = ScriptEngine::getActor(v, 2);
    if (!pActor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }
    SQInteger hidden = 0;
    if (SQ_FAILED(sq_getinteger(v, 3, &hidden))) {
      return sq_throwerror(v, _SC("failed to get hidden"));
    }
    pActor->setVisible(hidden == 0);
    return 0;
  }

  static SQInteger actorShowHideLayer(HSQUIRRELVM v, bool isVisible) {
    auto *pActor = ScriptEngine::getActor(v, 2);
    if (!pActor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }
    const SQChar *layerName;
    if (SQ_FAILED(sq_getstring(v, 3, &layerName))) {
      return sq_throwerror(v, _SC("failed to get layerName"));
    }
    pActor->getCostume().setLayerVisible(layerName, isVisible);
    return 0;
  }

  static SQInteger actorHideLayer(HSQUIRRELVM v) { return actorShowHideLayer(v, false); }

  static SQInteger actorInTrigger(HSQUIRRELVM v) {
    auto *actor = ScriptEngine::getActor(v, 2);
    if (!actor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }
    auto *object = ScriptEngine::getObject(v, 3);
    if (!object) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    bool isInside = object->getRealHotspot().contains((sf::Vector2i) (actor->getRealPosition()));
    sq_pushbool(v, isInside);
    return 1;
  }

  static SQInteger actorInWalkbox(HSQUIRRELVM v) {
    auto *actor = ScriptEngine::getActor(v, 2);
    if (!actor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }

    const SQChar *walkboxName;
    if (SQ_FAILED(sq_getstring(v, 3, &walkboxName))) {
      return sq_throwerror(v, _SC("failed to get walkbox"));
    }
    auto pWalkbox = g_pEngine->getRoom()->getWalkbox(walkboxName);
    auto inWalkbox = pWalkbox && pWalkbox->inside(actor->getRealPosition());
    sq_pushbool(v, inWalkbox ? SQTrue : SQFalse);
    return 1;
  }

  static SQInteger actorLockFacing(HSQUIRRELVM v) {
    SQInteger facing;
    Actor *actor = ScriptEngine::getActor(v, 2);
    if (!actor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }
    HSQOBJECT obj;
    sq_getstackobj(v, 3, &obj);
    if (sq_istable(obj)) {
      auto back = static_cast<SQInteger>(Facing::FACE_BACK);
      auto front = static_cast<SQInteger>(Facing::FACE_FRONT);
      auto left = static_cast<SQInteger>(Facing::FACE_LEFT);
      auto right = static_cast<SQInteger>(Facing::FACE_RIGHT);
      SQInteger reset = 0;
      readFieldInt(v, _SC("back"), back);
      readFieldInt(v, _SC("front"), front);
      readFieldInt(v, _SC("left"), left);
      readFieldInt(v, _SC("right"), right);
      readFieldInt(v, _SC("reset"), reset);
      if (reset) {
        actor->getCostume().resetLockFacing();
        return 0;
      }
      actor->getCostume().lockFacing(static_cast<Facing>(left), static_cast<Facing>(right),
                                     static_cast<Facing>(front), static_cast<Facing>(back));
      return 0;
    }
    if (SQ_FAILED(sq_getinteger(v, 3, &facing))) {
      return sq_throwerror(v, _SC("failed to get facing"));
    }
    if (facing == 0) {
      actor->getCostume().unlockFacing();
      return 0;
    }
    auto allFacing = static_cast<Facing>(facing);
    actor->getCostume().lockFacing(allFacing, allFacing, allFacing, allFacing);
    return 0;
  }

  static SQInteger actorBlinkRate(HSQUIRRELVM v) {
    Actor *pActor = ScriptEngine::getActor(v, 2);
    if (!pActor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }
    SQFloat min = 0;
    if (SQ_FAILED(sq_getfloat(v, 3, &min))) {
      return sq_throwerror(v, _SC("failed to get min"));
    }
    SQFloat max = 0;
    if (SQ_FAILED(sq_getfloat(v, 4, &max))) {
      return sq_throwerror(v, _SC("failed to get max"));
    }
    pActor->getCostume().setBlinkRate(min, max);
    return 0;
  }

  static SQInteger actorPlayAnimation(HSQUIRRELVM v) {
    Actor *pActor = ScriptEngine::getActor(v, 2);
    if (!pActor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }
    const SQChar *animation = nullptr;
    if (SQ_FAILED(sq_getstring(v, 3, &animation))) {
      return sq_throwerror(v, _SC("failed to get animation"));
    }
    SQInteger loop = 0;
    sq_getinteger(v, 4, &loop);
    trace("Play anim {} {}{}", pActor->getName(), animation, (loop != 0 ? " (loop)" : ""));
    pActor->stopWalking();
    pActor->getCostume().setState(animation, loop != 0);
    return 0;
  }

  static SQInteger actorPosX(HSQUIRRELVM v) {
    auto *pActor = ScriptEngine::getActor(v, 2);
    if (!pActor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }
    sq_pushinteger(v, pActor->getRealPosition().x);
    return 1;
  }

  static SQInteger actorPosY(HSQUIRRELVM v) {
    auto *pActor = ScriptEngine::getActor(v, 2);
    if (!pActor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }
    sq_pushinteger(v, pActor->getRealPosition().y);
    return 1;
  }

  static SQInteger actorRenderOffset(HSQUIRRELVM v) {
    auto *pActor = ScriptEngine::getActor(v, 2);
    if (!pActor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }
    SQInteger x, y;
    if (SQ_FAILED(sq_getinteger(v, 3, &x))) {
      return sq_throwerror(v, _SC("failed to get x"));
    }
    if (SQ_FAILED(sq_getinteger(v, 4, &y))) {
      return sq_throwerror(v, _SC("failed to get y"));
    }
    pActor->setRenderOffset(sf::Vector2i(x, y));
    return 0;
  }

  static SQInteger actorRoom(HSQUIRRELVM v) {
    auto *pActor = ScriptEngine::getActor(v, 2);
    if (!pActor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }
    auto pRoom = pActor->getRoom();
    if (pRoom) {
      sq_pushobject(v, pRoom->getTable());
      return 1;
    }
    sq_pushnull(v);
    return 1;
  }

  static SQInteger actorStand(HSQUIRRELVM v) {
    auto *pActor = ScriptEngine::getActor(v, 2);
    if (!pActor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }
    pActor->getCostume().setStandState();
    return 0;
  }

  static SQInteger actorShowLayer(HSQUIRRELVM v) { return actorShowHideLayer(v, true); }

  static SQInteger actorSlotSelectable(HSQUIRRELVM v) {
    auto numArgs = sq_gettop(v);
    if (numArgs == 2) {
      SQInteger selectable;
      if (SQ_FAILED(sq_getinteger(v, 2, &selectable))) {
        return sq_throwerror(v, _SC("failed to get selectable"));
      }
      auto mode = g_pEngine->getActorSlotSelectable();
      switch (selectable) {
      case 0:g_pEngine->setActorSlotSelectable(mode & ~ActorSlotSelectableMode::On);
        return 0;
      case 1:g_pEngine->setActorSlotSelectable(mode | ActorSlotSelectableMode::On);
        return 0;
      case 2:g_pEngine->setActorSlotSelectable(mode | ActorSlotSelectableMode::TemporaryUnselectable);
        return 0;
      case 3:g_pEngine->setActorSlotSelectable(mode & ~ActorSlotSelectableMode::TemporaryUnselectable);
        return 0;
      default:return sq_throwerror(v, _SC("invalid selectable value"));
      }
    }

    if (numArgs == 3) {
      SQInteger selectable;
      if (SQ_FAILED(sq_getinteger(v, 3, &selectable))) {
        return sq_throwerror(v, _SC("failed to get selectable"));
      }
      if (sq_gettype(v, 2) == OT_INTEGER) {
        SQInteger actorIndex = 0;
        if (SQ_FAILED(sq_getinteger(v, 2, &actorIndex))) {
          return sq_throwerror(v, _SC("failed to get actor index"));
        }
        g_pEngine->actorSlotSelectable(actorIndex, selectable == SQTrue);
        return 0;
      }
      auto actor = ScriptEngine::getActor(v, 2);
      if (!actor) {
        return sq_throwerror(v, _SC("failed to get actor"));
      }
      g_pEngine->actorSlotSelectable(actor, selectable != 0);
      return 0;
    }
    return sq_throwerror(v, _SC("invalid number of arguments"));
  }

  static SQInteger actorTalking(HSQUIRRELVM v) {
    Actor *pActor;
    if (sq_gettop(v) == 2) {
      pActor = ScriptEngine::getActor(v, 2);
      if (!pActor) {
        return sq_throwerror(v, _SC("failed to get actor"));
      }
    } else {
      pActor = g_pEngine->getCurrentActor();
    }
    sq_pushbool(v, pActor && pActor->isTalking());
    return 1;
  }

  static SQInteger actorStopWalking(HSQUIRRELVM v) {
    auto actor = ScriptEngine::getActor(v, 2);
    if (!actor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }
    actor->stopWalking();
    return 0;
  }

  static SQInteger actorTalkColors(HSQUIRRELVM v) {
    auto actor = ScriptEngine::getActor(v, 2);
    if (!actor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }
    SQInteger color;
    if (SQ_FAILED(sq_getinteger(v, 3, &color))) {
      return sq_throwerror(v, _SC("failed to get fps"));
    }
    actor->setTalkColor(_fromRgb(color));
    return 0;
  }

  static SQInteger actorTalkOffset(HSQUIRRELVM v) {
    Actor *actor = ScriptEngine::getActor(v, 2);
    if (!actor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }
    SQInteger x;
    if (SQ_FAILED(sq_getinteger(v, 3, &x))) {
      return sq_throwerror(v, _SC("failed to get x"));
    }
    SQInteger y;
    if (SQ_FAILED(sq_getinteger(v, 4, &y))) {
      return sq_throwerror(v, _SC("failed to get y"));
    }
    actor->setTalkOffset(sf::Vector2i(x, y));
    return 0;
  }

  static SQInteger actorTurnTo(HSQUIRRELVM v) {
    Actor *actor = ScriptEngine::getActor(v, 2);
    if (!actor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }

    if (sq_gettype(v, 3) == OT_INTEGER) {
      SQInteger facing = 0;
      if (SQ_FAILED(sq_getinteger(v, 3, &facing))) {
        return sq_throwerror(v, _SC("failed to get facing"));
      }
      actor->getCostume().setFacing((Facing) facing);
      return 0;
    }

    Entity *entity = ScriptEngine::getEntity(v, 3);
    if (!entity) {
      return sq_throwerror(v, _SC("failed to get entity to face to"));
    }

    auto facing = _getFacingToFaceTo(actor, entity);
    actor->getCostume().setFacing(facing);
    return 0;
  }

  static Facing _getFacingToFaceTo(Actor *pActor, Entity *pEntity) {
    Facing facing;
    auto d = pEntity->getRealPosition() - pActor->getRealPosition();
    if (d.x == 0) {
      facing = d.y > 0 ? Facing::FACE_FRONT : Facing::FACE_BACK;
    } else {
      facing = d.x > 0 ? Facing::FACE_RIGHT : Facing::FACE_LEFT;
    }
    return facing;
  }

  static Facing _flip(Facing facing) {
    switch (facing) {
    case Facing::FACE_BACK:return Facing::FACE_FRONT;
    case Facing::FACE_FRONT:return Facing::FACE_BACK;
    case Facing::FACE_LEFT:return Facing::FACE_RIGHT;
    case Facing::FACE_RIGHT:return Facing::FACE_LEFT;
    }
    throw std::out_of_range("Invalid facing");
  }

  static SQInteger actorUsePos(HSQUIRRELVM v) {
    auto numArgs = sq_gettop(v);
    auto *actor = ScriptEngine::getActor(v, 2);
    if (!actor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }
    auto *obj = ScriptEngine::getObject(v, 3);
    if (!obj) {
      actor->setUsePosition(sf::Vector2f());
      return 0;
    }
    auto usePos = obj->getUsePosition();
    actor->setUsePosition(usePos);
    if (numArgs == 4) {
      SQInteger dir;
      if (SQ_FAILED(sq_getinteger(v, 5, &dir))) {
        return sq_throwerror(v, _SC("failed to get direction"));
      }
      actor->setUseDirection(static_cast<UseDirection>(dir));
    }
    return 0;
  }

  static SQInteger actorUseWalkboxes(HSQUIRRELVM v) {
    Actor *actor = ScriptEngine::getActor(v, 2);
    if (!actor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }
    SQBool use;
    sq_tobool(v, 3, &use);

    actor->useWalkboxes(use);
    return 0;
  }

  static SQInteger actorVolume(HSQUIRRELVM v) {
    Actor *actor = ScriptEngine::getActor(v, 2);
    if (!actor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }
    SQFloat volume;
    if (SQ_FAILED(sq_getfloat(v, 3, &volume))) {
      return sq_throwerror(v, _SC("failed to get volume"));
    }
    actor->setVolume(volume);
    return 0;
  }

  static SQInteger actorWalkForward(HSQUIRRELVM v) {
    Actor *actor = ScriptEngine::getActor(v, 2);
    if (!actor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }
    SQInteger dist;
    if (SQ_FAILED(sq_getinteger(v, 3, &dist))) {
      return sq_throwerror(v, _SC("failed to get distance"));
    }
    sf::Vector2f direction;
    switch (actor->getCostume().getFacing()) {
    case Facing::FACE_FRONT:direction = sf::Vector2f(0, -dist);
      break;
    case Facing::FACE_BACK:direction = sf::Vector2f(0, dist);
      break;
    case Facing::FACE_LEFT:direction = sf::Vector2f(-dist, 0);
      break;
    case Facing::FACE_RIGHT:direction = sf::Vector2f(dist, 0);
      break;
    }
    actor->walkTo(actor->getRealPosition() + direction);
    return 0;
  }

  static SQInteger actorWalking(HSQUIRRELVM v) {
    auto numArgs = sq_gettop(v) - 1;
    Actor *pActor = nullptr;
    if (numArgs == 0) {
      pActor = g_pEngine->getCurrentActor();
    } else if (numArgs == 1) {
      pActor = ScriptEngine::getActor(v, 2);
    }
    if (!pActor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }
    sq_pushbool(v, pActor->isWalking() ? SQTrue : SQFalse);
    return 1;
  }

  static SQInteger actorWalkSpeed(HSQUIRRELVM v) {
    auto pActor = ScriptEngine::getActor(v, 2);
    if (!pActor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }
    SQInteger x, y;
    if (SQ_FAILED(sq_getinteger(v, 3, &x))) {
      return sq_throwerror(v, _SC("failed to get x"));
    }
    if (SQ_FAILED(sq_getinteger(v, 4, &y))) {
      return sq_throwerror(v, _SC("failed to get y"));
    }
    pActor->setWalkSpeed(sf::Vector2i(x, y));
    return 0;
  }

  static SQInteger actorWalkTo(HSQUIRRELVM v) {
    auto numArgs = sq_gettop(v);
    auto *pActor = ScriptEngine::getActor(v, 2);
    if (!pActor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }
    if (numArgs == 3) {
      auto *pObject = ScriptEngine::getObject(v, 3);
      if (pObject) {
        auto pos = pObject->getRealPosition();
        auto usePos = pObject->getUsePosition().value_or(sf::Vector2f());
        pos.x += usePos.x;
        pos.y += usePos.y;
        pActor->walkTo(pos, _toFacing(pObject->getUseDirection()));
        return 0;
      }

      pActor = ScriptEngine::getActor(v, 3);
      if (!pActor) {
        return sq_throwerror(v, _SC("failed to get object or actor"));
      }

      auto pos = pActor->getRealPosition();
      pActor->walkTo(sf::Vector2f(pos), getOppositeFacing(pActor->getCostume().getFacing()));
      return 0;
    }

    SQInteger x, y;
    if (SQ_FAILED(sq_getinteger(v, 3, &x))) {
      return sq_throwerror(v, _SC("failed to get x"));
    }
    if (SQ_FAILED(sq_getinteger(v, 4, &y))) {
      return sq_throwerror(v, _SC("failed to get y"));
    }
    pActor->walkTo(sf::Vector2f(x, y));
    return 0;
  }

  static SQInteger addSelectableActor(HSQUIRRELVM v) {
    SQInteger slot;
    if (SQ_FAILED(sq_getinteger(v, 2, &slot))) {
      return sq_throwerror(v, _SC("failed to get slot"));
    }
    auto *pActor = ScriptEngine::getActor(v, 3);
    g_pEngine->addSelectableActor(slot, pActor);
    return 0;
  }

  static SQInteger createActor(HSQUIRRELVM v) {
    auto pActor = std::make_unique<Actor>(*g_pEngine);
    auto &table = pActor->getTable();
    sq_resetobject(&table);
    sq_getstackobj(v, 2, &table);
    sq_addref(v, &table);

    const char *key = nullptr;
    if (ScriptEngine::rawGet(pActor.get(), "_key", key)) {
      pActor->setKey(key);
    }

    // define instance
    ScriptEngine::set(pActor.get(), "_id", pActor->getId());

    trace("Create actor {}", pActor->getName());
    g_pEngine->addActor(std::move(pActor));

    sq_pushobject(v, table);
    return 1;
  }

  static SQInteger isActor(HSQUIRRELVM v) {
    auto actor = ScriptEngine::getActor(v, 2);
    sq_pushbool(v, actor ? SQTrue : SQFalse);
    return 1;
  }

  static SQInteger isActorSelectable(HSQUIRRELVM v) {
    auto actor = ScriptEngine::getActor(v, 2);
    bool isSelectable = g_pEngine->isActorSelectable(actor);
    sq_pushbool(v, isSelectable ? SQTrue : SQFalse);
    return 1;
  }

  static SQInteger isActorOnScreen(HSQUIRRELVM v) {
    auto entity = ScriptEngine::getEntity(v, 2);
    if (!entity) {
      return sq_throwerror(v, _SC("failed to get entity"));
    }

    const Room *pEntityRoom = entity->getRoom();
    const Room *pRoom = g_pEngine->getRoom();
    if (pEntityRoom != pRoom) {
      sq_pushbool(v, SQFalse);
      return 1;
    }

    auto screen = g_pEngine->getWindow().getView().getSize();
    auto pos = (sf::Vector2i) entity->getRealPosition();
    auto camera = g_pEngine->getCamera().getAt();
    sf::IntRect rect(camera.x, camera.y, screen.x, screen.y);
    auto isOnScreen = rect.contains(pos);
    sq_pushbool(v, isOnScreen ? SQTrue : SQFalse);
    return 1;
  }

  static SQInteger masterActorArray(HSQUIRRELVM v) {
    auto &actors = g_pEngine->getActors();
    sq_newarray(v, 0);
    for (auto &actor : actors) {
      sq_pushobject(v, actor->getTable());
      sq_arrayappend(v, -2);
    }
    return 1;
  }

  static SQInteger _sayLine(HSQUIRRELVM v, bool mumble = false) {
    Actor *actor;
    SQInteger index;
    if (sq_gettype(v, 2) == OT_TABLE) {
      actor = ScriptEngine::getActor(v, 2);
      index = 3;
    } else {
      actor = g_pEngine->getCurrentActor();
      index = 2;
    }

    if (!actor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }

    if (sq_gettype(v, index) == OT_ARRAY) {
      sq_push(v, index);
      sq_pushnull(v);  //null iterator
      while (SQ_SUCCEEDED(sq_next(v, -2))) {
        //here -1 is the value and -2 is the key
        const SQChar *idText = nullptr;
        sq_getstring(v, -1, &idText);
        actor->say(idText, mumble);
        sq_pop(v, 2); //pops key and val before the nex iteration
      }
      sq_pop(v, 2); //pops the null iterator + array
    } else {
      auto numIds = sq_gettop(v) - index + 1;
      for (int i = 0; i < numIds; i++) {
        const SQChar *idText = nullptr;
        if (SQ_FAILED(sq_getstring(v, index + i, &idText))) {
          return sq_throwerror(v, _SC("failed to get text"));
        }
        actor->say(idText, mumble);
      }
    }
    return 0;
  }

  static SQInteger mumbleLine(HSQUIRRELVM v) { return _sayLine(v, true); }

  static SQInteger sayLine(HSQUIRRELVM v) {
    auto pCurrentActor = ScriptEngine::getActor(v, 2);
    if (!pCurrentActor) {
      pCurrentActor = g_pEngine->getCurrentActor();
    }

    // actors should stop talking except current actor
    auto &actors = g_pEngine->getActors();
    for (auto &a : actors) {
      if (a.get() == pCurrentActor)
        continue;
      a->stopTalking();
    }

    return _sayLine(v);
  }

  static SQInteger sayLineAt(HSQUIRRELVM v) {
    SQInteger x;
    if (SQ_FAILED(sq_getinteger(v, 2, &x))) {
      return sq_throwerror(v, _SC("failed to get x"));
    }
    SQInteger y;
    if (SQ_FAILED(sq_getinteger(v, 3, &y))) {
      return sq_throwerror(v, _SC("failed to get y"));
    }
    if (sq_gettype(v, 4) == OT_INTEGER) {
      SQInteger c;
      if (SQ_FAILED(sq_getinteger(v, 4, &c))) {
        return sq_throwerror(v, _SC("failed to get color"));
      }
      auto color = _fromRgb(c);
      SQFloat t;
      if (SQ_FAILED(sq_getfloat(v, 5, &t))) {
        return sq_throwerror(v, _SC("failed to get time"));
      }
      const SQChar *text;
      if (SQ_FAILED(sq_getstring(v, 6, &text))) {
        return sq_throwerror(v, _SC("failed to get text"));
      }
      g_pEngine->sayLineAt(sf::Vector2i(x, y), color, sf::seconds(t), text);
      return 0;
    }
    auto *actor = ScriptEngine::getActor(v, 4);
    if (!actor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }
    const SQChar *text;
    if (SQ_FAILED(sq_getstring(v, 6, &text))) {
      return sq_throwerror(v, _SC("failed to get text"));
    }
    g_pEngine->sayLineAt(sf::Vector2i(x, y), *actor, text);
    return 0;
  }

  static SQInteger selectActor(HSQUIRRELVM v) {
    auto *actor = ScriptEngine::getActor(v, 2);
    if (!actor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }
    g_pEngine->setCurrentActor(actor, false);
    return 0;
  }

  static SQInteger stopTalking(HSQUIRRELVM v) {
    auto numArgs = sq_gettop(v) - 1;
    if (numArgs == 1) {
      if (sq_gettype(v, 2) == OT_INTEGER) {
        for (auto &&a : g_pEngine->getActors()) {
          a->stopTalking();
        }
        return 0;
      }
      auto actor = ScriptEngine::getActor(v, 2);
      if (!actor) {
        return sq_throwerror(v, _SC("failed to get actor"));
      }
      actor->stopTalking();
      return 0;
    }

    g_pEngine->getCurrentActor()->stopTalking();
    return 0;
  }

  static SQInteger triggerActors(HSQUIRRELVM v) {
    auto *object = ScriptEngine::getObject(v, 2);
    if (!object) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    sq_newarray(v, 0);
    for (const auto &actor : g_pEngine->getActors()) {
      if (object->getRealHotspot().contains((sf::Vector2i) actor->getRealPosition())) {
        sq_pushobject(v, actor->getTable());
        sq_arrayappend(v, -2);
      }
    }
    return 1;
  }

  static SQInteger readFieldInt(HSQUIRRELVM v, const SQChar *name, SQInteger &field) {
    sq_pushstring(v, name, -1);
    if (SQ_FAILED(sq_get(v, -2))) {
      return SQ_ERROR;
    }

    field = 0;
    if (SQ_FAILED(sq_getinteger(v, -1, &field))) {
      return SQ_ERROR;
    }
    sq_pop(v, 1);
    return SQ_OK;
  }

  static SQInteger verbUIColors(HSQUIRRELVM v) {
    SQInteger actorSlot;
    if (SQ_FAILED(sq_getinteger(v, 2, &actorSlot))) {
      return sq_throwerror(v, _SC("failed to get actor slot"));
    }

    HSQOBJECT table;
    if (SQ_FAILED(sq_getstackobj(v, 3, &table))) {
      return sq_throwerror(v, _SC("failed to get verb definitionTable"));
    }
    if (!sq_istable(table)) {
      const SQChar *tmp;
      sq_getstring(v, 3, &tmp);
      return sq_throwerror(v, _SC("failed to get verb definitionTable"));
    }

    sq_pushobject(v, table);

    // sentence
    SQInteger sentence = 0;
    if (SQ_FAILED(readFieldInt(v, _SC("sentence"), sentence))) {
      return sq_throwerror(v, _SC("failed to get sentence"));
    }

    SQInteger verbNormal = 0;
    if (SQ_FAILED(readFieldInt(v, _SC("verbNormal"), verbNormal))) {
      return sq_throwerror(v, _SC("failed to get verbNormal"));
    }

    SQInteger verbNormalTint = 0;
    if (SQ_FAILED(readFieldInt(v, _SC("verbNormalTint"), verbNormalTint))) {
      return sq_throwerror(v, _SC("failed to get verbNormal"));
    }

    SQInteger verbHighlight = 0;
    if (SQ_FAILED(readFieldInt(v, _SC("verbHighlight"), verbHighlight))) {
      return sq_throwerror(v, _SC("failed to get verbHighlight"));
    }

    SQInteger verbHighlightTint = 0;
    if (SQ_FAILED(readFieldInt(v, _SC("verbHighlightTint"), verbHighlightTint))) {
      return sq_throwerror(v, _SC("failed to get verbHighlightTint"));
    }

    SQInteger inventoryFrame = 0;
    if (SQ_FAILED(readFieldInt(v, _SC("inventoryFrame"), inventoryFrame))) {
      return sq_throwerror(v, _SC("failed to get inventoryFrame"));
    }

    SQInteger inventoryBackground = 0;
    if (SQ_FAILED(readFieldInt(v, _SC("inventoryBackground"), inventoryBackground))) {
      return sq_throwerror(v, _SC("failed to get inventoryBackground"));
    }

    SQInteger retroNormal = verbNormal;
    readFieldInt(v, _SC("retroNormal"), retroNormal);

    SQInteger retroHighlight = verbNormalTint;
    readFieldInt(v, _SC("retroHighlight"), retroHighlight);

    SQInteger dialogNormal = verbNormal;
    readFieldInt(v, _SC("dialogNormal"), dialogNormal);

    SQInteger dialogHighlight = verbHighlight;
    readFieldInt(v, _SC("dialogHighlight"), dialogHighlight);

    VerbUiColors colors;
    colors.sentence = _fromRgb(sentence);
    colors.verbNormal = _fromRgb(verbNormal);
    colors.verbNormalTint = _fromRgb(verbNormalTint);
    colors.verbHighlight = _fromRgb(verbHighlight);
    colors.verbHighlightTint = _fromRgb(verbHighlightTint);
    colors.dialogNormal = _fromRgb(dialogNormal);
    colors.dialogHighlight = _fromRgb(dialogHighlight);
    colors.inventoryFrame = _fromRgb(inventoryFrame);
    colors.inventoryBackground = _fromRgb(inventoryBackground);
    colors.retroNormal = _fromRgb(retroNormal);
    colors.retroHighlight = _fromRgb(retroHighlight);
    g_pEngine->getHud().setVerbUiColors(static_cast<int>(actorSlot) - 1, colors);
    return 0;
  }
};

Engine *_ActorPack::g_pEngine = nullptr;

} // namespace ng
