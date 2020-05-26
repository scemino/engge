#pragma once
#include <utility>

#include "Entities/Objects/TextObject.hpp"
#include "squirrel.h"

namespace ng {
class _ObjectPack : public Pack {
private:
  static Engine *g_pEngine;

private:
  void addTo(ScriptEngine &engine) const override {
    g_pEngine = &engine.getEngine();
    engine.registerGlobalFunction(createObject, "createObject");
    engine.registerGlobalFunction(createTextObject, "createTextObject");
    engine.registerGlobalFunction(deleteObject, "deleteObject");
    engine.registerGlobalFunction(findObjectAt, "findObjectAt");
    engine.registerGlobalFunction(isObject, "is_object");
    engine.registerGlobalFunction(isInventoryOnScreen, "isInventoryOnScreen");
    engine.registerGlobalFunction(isObject, "isObject");
    engine.registerGlobalFunction(jiggleInventory, "jiggleInventory");
    engine.registerGlobalFunction(jiggleObject, "jiggleObject");
    engine.registerGlobalFunction(loopObjectState, "loopObjectState");
    engine.registerGlobalFunction(objectAlpha, "objectAlpha");
    engine.registerGlobalFunction(objectAlphaTo, "objectAlphaTo");
    engine.registerGlobalFunction(objectAt, "objectAt");
    engine.registerGlobalFunction(objectBumperCycle, "objectBumperCycle");
    engine.registerGlobalFunction(objectCenter, "objectCenter");
    engine.registerGlobalFunction(objectColor, "objectColor");
    engine.registerGlobalFunction(objectDependentOn, "objectDependentOn");
    engine.registerGlobalFunction(objectFPS, "objectFPS");
    engine.registerGlobalFunction(objectHidden, "objectHidden");
    engine.registerGlobalFunction(objectHotspot, "objectHotspot");
    engine.registerGlobalFunction(objectIcon, "objectIcon");
    engine.registerGlobalFunction(objectLit, "objectLit");
    engine.registerGlobalFunction(objectMoveTo, "objectMoveTo");
    engine.registerGlobalFunction(objectOffset, "objectOffset");
    engine.registerGlobalFunction(objectOffsetTo, "objectOffsetTo");
    engine.registerGlobalFunction(objectOwner, "objectOwner");
    engine.registerGlobalFunction(objectParallaxLayer, "objectParallaxLayer");
    engine.registerGlobalFunction(objectParent, "objectParent");
    engine.registerGlobalFunction(objectPosX, "objectPosX");
    engine.registerGlobalFunction(objectPosY, "objectPosY");
    engine.registerGlobalFunction(objectRenderOffset, "objectRenderOffset");
    engine.registerGlobalFunction(objectRoom, "objectRoom");
    engine.registerGlobalFunction(objectRotate, "objectRotate");
    engine.registerGlobalFunction(objectRotateTo, "objectRotateTo");
    engine.registerGlobalFunction(objectScale, "objectScale");
    engine.registerGlobalFunction(objectScaleTo, "objectScaleTo");
    engine.registerGlobalFunction(objectScreenSpace, "objectScreenSpace");
    engine.registerGlobalFunction(objectShader, "objectShader");
    engine.registerGlobalFunction(objectSort, "objectSort");
    engine.registerGlobalFunction(objectState, "objectState");
    engine.registerGlobalFunction(objectTouchable, "objectTouchable");
    engine.registerGlobalFunction(objectUsePos, "objectUsePos");
    engine.registerGlobalFunction(objectUsePosX, "objectUsePosX");
    engine.registerGlobalFunction(objectUsePosY, "objectUsePosY");
    engine.registerGlobalFunction(objectValidUsePos, "objectValidUsePos");
    engine.registerGlobalFunction(objectValidVerb, "objectValidVerb");
    engine.registerGlobalFunction(pickupObject, "pickupObject");
    engine.registerGlobalFunction(pickupReplacementObject, "pickupReplacementObject");
    engine.registerGlobalFunction(playObjectState, "playObjectState");
    engine.registerGlobalFunction(popInventory, "popInventory");
    engine.registerGlobalFunction(removeInventory, "removeInventory");
    engine.registerGlobalFunction(setDefaultObject, "setDefaultObject");
    engine.registerGlobalFunction(scale, "scale");
    engine.registerGlobalFunction(shakeObject, "shakeObject");
    engine.registerGlobalFunction(stopObjectMotors, "stopObjectMotors");
  }

  static SQInteger findObjectAt(HSQUIRRELVM v) {
    SQInteger x = 0;
    if (SQ_FAILED(sq_getinteger(v, 2, &x))) {
      return sq_throwerror(v, _SC("failed to get x"));
    }
    SQInteger y = 0;
    if (SQ_FAILED(sq_getinteger(v, 3, &y))) {
      return sq_throwerror(v, _SC("failed to get y"));
    }
    auto *room = g_pEngine->getRoom();
    auto &objects = room->getObjects();

    for (auto &obj : objects) {
      if (obj->getRealHotspot().contains(sf::Vector2i(x, y))) {
        sq_pushobject(v, obj->getTable());
        return 1;
      }
    }

    sq_pushnull(v);
    return 1;
  }

  static SQInteger isInventoryOnScreen(HSQUIRRELVM v) {
    auto object = ScriptEngine::getObject(v, 2);
    auto owner = object->getOwner();
    if (!owner) {
      sq_pushbool(v, SQFalse);
      return 1;
    }
    if (g_pEngine->getCurrentActor() != owner) {
      sq_pushbool(v, SQFalse);
      return 1;
    }
    auto offset = owner->getInventoryOffset();
    auto &objects = owner->getObjects();
    auto it = std::find_if(objects.begin(), objects.end(), [&object](auto &pObj) {
      return pObj == object;
    });
    auto index = std::distance(objects.begin(), it);
    if (index >= offset * 4 && index < (offset * 4 + 8)) {
      sq_pushbool(v, SQTrue);
      return 1;
    }
    sq_pushbool(v, SQFalse);
    return 1;
  }

  static SQInteger isObject(HSQUIRRELVM v) {
    auto object = ScriptEngine::getObject(v, 2);
    sq_pushbool(v, object ? SQTrue : SQFalse);
    return 1;
  }

  static SQInteger jiggleInventory(HSQUIRRELVM v) {
    Object *obj = ScriptEngine::getObject(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    SQInteger enabled = 0;
    if (SQ_FAILED(sq_getinteger(v, 3, &enabled))) {
      return sq_throwerror(v, _SC("failed to get enabled"));
    }
    obj->setJiggle(enabled != 0);
    return 0;
  }

  static SQInteger jiggleObject(HSQUIRRELVM v) {
    error("TODO: jiggleObject: not implemented");
    Object *obj = ScriptEngine::getObject(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    SQFloat amount = 0;
    if (SQ_FAILED(sq_getfloat(v, 3, &amount))) {
      return sq_throwerror(v, _SC("failed to get amount"));
    }
    // obj->jiggle(amount);
    return 0;
  }

  static SQInteger scale(HSQUIRRELVM v) {
    SQFloat s = 0;
    if (SQ_FAILED(sq_getfloat(v, 3, &s))) {
      return sq_throwerror(v, _SC("failed to get scale"));
    }
    Object *self = ScriptEngine::getObject(v, 2);
    if (!self) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    self->setScale(s);
    return 0;
  }

  static SQInteger objectAlpha(HSQUIRRELVM v) {
    if (sq_gettype(v, 2) == OT_NULL)
      return 0;

    Object *obj = ScriptEngine::getObject(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    SQFloat alpha = 0;
    if (SQ_FAILED(sq_getfloat(v, 3, &alpha))) {
      return sq_throwerror(v, _SC("failed to get alpha"));
    }
    alpha = alpha > 1.f ? 1.f : alpha;
    alpha = alpha < 0.f ? 0.f : alpha;
    auto a = (sf::Uint8) (alpha * 255);
    auto color = obj->getColor();
    obj->setColor(sf::Color(color.r, color.g, color.b, a));
    return 0;
  }

  static SQInteger objectAlphaTo(HSQUIRRELVM v) {
    if (sq_gettype(v, 2) == OT_NULL)
      return 0;

    Object *obj = ScriptEngine::getObject(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    SQFloat alpha = 0;
    if (SQ_FAILED(sq_getfloat(v, 3, &alpha))) {
      return sq_throwerror(v, _SC("failed to get alpha"));
    }
    alpha = alpha > 1.f ? 1.f : alpha;
    alpha = alpha < 0.f ? 0.f : alpha;
    SQFloat time = 0;
    if (SQ_FAILED(sq_getfloat(v, 4, &time))) {
      return sq_throwerror(v, _SC("failed to get time"));
    }
    SQInteger interpolation;
    if (SQ_FAILED(sq_getinteger(v, 5, &interpolation))) {
      interpolation = 0;
    }
    obj->alphaTo(alpha, sf::seconds(time), toInterpolationMethod(interpolation));
    return 0;
  }

  static SQInteger objectBumperCycle(HSQUIRRELVM v) {
    auto pObj = ScriptEngine::getEntity(v, 2);
    if (!pObj) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    SQInteger enabled = 0;
    if (SQ_FAILED(sq_getinteger(v, 3, &enabled))) {
      return sq_throwerror(v, _SC("failed to get enabled"));
    }
    pObj->objectBumperCycle(enabled != 0);
    return 0;
  }

  static SQInteger objectHotspot(HSQUIRRELVM v) {
    SQInteger left = 0;
    SQInteger top = 0;
    SQInteger right = 0;
    SQInteger bottom = 0;

    auto numArgs = sq_gettop(v);

    Object *obj = ScriptEngine::getObject(v, 2);
    Actor *actor = nullptr;
    if (!obj) {
      actor = ScriptEngine::getActor(v, 2);
      if (!actor) {
        return sq_throwerror(v, _SC("failed to get object or actor"));
      }
    }
    if (numArgs == 2) {
      // this really fucked up, when seeting hotspot it's a relative rect,
      // when getting hotspot the position is absolute
      const auto pos = obj->getPosition();
      const auto hotspot = obj->getHotspot();
      sf::IntRect r = {hotspot.left + static_cast<int>(pos.x), hotspot.top + static_cast<int>(pos.y), hotspot.width,
                       hotspot.height};
      ScriptEngine::push(v, r);
      return 1;
    }
    if (SQ_FAILED(sq_getinteger(v, 3, &left))) {
      return sq_throwerror(v, _SC("failed to get left"));
    }
    if (SQ_FAILED(sq_getinteger(v, 4, &top))) {
      return sq_throwerror(v, _SC("failed to get top"));
    }
    if (SQ_FAILED(sq_getinteger(v, 5, &right))) {
      return sq_throwerror(v, _SC("failed to get right"));
    }
    if (SQ_FAILED(sq_getinteger(v, 6, &bottom))) {
      return sq_throwerror(v, _SC("failed to get bottom"));
    }

    if (obj) {
      obj->setHotspot(sf::IntRect(static_cast<int>(left), static_cast<int>(top), static_cast<int>(right - left),
                                  static_cast<int>(bottom - top)));
    } else {
      actor->setHotspot(sf::IntRect(static_cast<int>(left), static_cast<int>(top), static_cast<int>(right - left),
                                    static_cast<int>(bottom - top)));
    }
    return 0;
  }

  static SQInteger objectOffset(HSQUIRRELVM v) {
    SQInteger x = 0;
    SQInteger y = 0;
    auto *obj = ScriptEngine::getEntity(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object or actor"));
    }
    if (SQ_FAILED(sq_getinteger(v, 3, &x))) {
      return sq_throwerror(v, _SC("failed to get x"));
    }
    if (SQ_FAILED(sq_getinteger(v, 4, &y))) {
      return sq_throwerror(v, _SC("failed to get y"));
    }
    obj->setOffset(sf::Vector2f(x, y));
    return 0;
  }

  static SQInteger objectScreenSpace(HSQUIRRELVM v) {
    auto obj = ScriptEngine::getObject(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    obj->setScreenSpace(ScreenSpace::Object);
    return 0;
  }

  static SQInteger objectState(HSQUIRRELVM v) {
    auto obj = ScriptEngine::getObject(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    auto numArgs = sq_gettop(v) - 2;
    if (numArgs == 0) {
      sq_pushinteger(v, obj->getState());
      return 1;
    }

    SQInteger state;
    if (SQ_FAILED(sq_getinteger(v, 3, &state))) {
      return sq_throwerror(v, _SC("failed to get state"));
    }
    obj->setStateAnimIndex(state);

    return 0;
  }

  static SQInteger objectOffsetTo(HSQUIRRELVM v) {
    SQInteger x = 0;
    SQInteger y = 0;
    SQFloat t = 0;
    auto *obj = ScriptEngine::getEntity(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get entity"));
    }
    if (SQ_FAILED(sq_getinteger(v, 3, &x))) {
      return sq_throwerror(v, _SC("failed to get x"));
    }
    if (SQ_FAILED(sq_getinteger(v, 4, &y))) {
      return sq_throwerror(v, _SC("failed to get y"));
    }
    if (SQ_FAILED(sq_getfloat(v, 5, &t))) {
      return sq_throwerror(v, _SC("failed to get t"));
    }
    SQInteger interpolation;
    if (SQ_FAILED(sq_getinteger(v, 6, &interpolation))) {
      interpolation = 0;
    }
    obj->offsetTo(sf::Vector2f(x, y), sf::seconds(t), toInterpolationMethod(interpolation));
    return 0;
  }

  static SQInteger objectMoveTo(HSQUIRRELVM v) {
    SQInteger x = 0;
    SQInteger y = 0;
    SQFloat t = 0;
    Entity *obj = ScriptEngine::getEntity(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    if (SQ_FAILED(sq_getinteger(v, 3, &x))) {
      return sq_throwerror(v, _SC("failed to get x"));
    }
    if (SQ_FAILED(sq_getinteger(v, 4, &y))) {
      return sq_throwerror(v, _SC("failed to get y"));
    }
    if (SQ_FAILED(sq_getfloat(v, 5, &t))) {
      return sq_throwerror(v, _SC("failed to get t"));
    }
    SQInteger interpolation;
    if (SQ_FAILED(sq_getinteger(v, 6, &interpolation))) {
      interpolation = 0;
    }
    obj->moveTo(sf::Vector2f(x, y), sf::seconds(t), toInterpolationMethod(interpolation));
    return 0;
  }

  static SQInteger loopObjectState(HSQUIRRELVM v) {
    SQInteger index;
    Object *obj = ScriptEngine::getObject(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    if (SQ_FAILED(sq_getinteger(v, 3, &index))) {
      return sq_throwerror(v, _SC("failed to get state"));
    }
    obj->playAnim(static_cast<int>(index), true);
    return 0;
  }

  static SQInteger playObjectState(HSQUIRRELVM v) {
    Object *obj = ScriptEngine::getObject(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    if (sq_gettype(v, 3) == OT_INTEGER) {
      SQInteger index;
      if (SQ_FAILED(sq_getinteger(v, 3, &index))) {
        return sq_throwerror(v, _SC("failed to get state"));
      }
      obj->playAnim(static_cast<int>(index), false);
      return 0;
    }
    if (sq_gettype(v, 3) == OT_STRING) {
      const SQChar *state;
      if (SQ_FAILED(sq_getstring(v, 3, &state))) {
        return sq_throwerror(v, _SC("failed to get state"));
      }
      obj->playAnim(state, false);
      return 0;
    }
    return sq_throwerror(v, _SC("failed to get state"));
  }

  static SQInteger popInventory(HSQUIRRELVM v) {
    Object *obj = ScriptEngine::getObject(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    error("TODO: popInventory not implemented");
    return 0;
  }

  static SQInteger removeInventory(HSQUIRRELVM v) {
    Object *obj = ScriptEngine::getObject(v, 2);
    if (obj) {
      auto owner = obj->getOwner();
      if (owner) {
        owner->removeInventory(obj);
      }
      return 0;
    }

    Actor *actor = ScriptEngine::getActor(v, 2);
    if (!actor) {
      return sq_throwerror(v, _SC("failed to get object or actor"));
    }
    actor->clearInventory();
    return 0;
  }

  static SQInteger objectAt(HSQUIRRELVM v) {
    SQInteger x, y;
    auto numArgs = sq_gettop(v);
    Object *obj = ScriptEngine::getObject(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    if (numArgs == 3) {
      Object *spot = ScriptEngine::getObject(v, 3);
      if (!spot) {
        return sq_throwerror(v, _SC("failed to get spot"));
      }
      auto pos = spot->getRealPosition();
      auto usePos = spot->getUsePosition().value_or(sf::Vector2f());
      auto hotspot = spot->getHotspot();
      pos.x += usePos.x + hotspot.left + hotspot.width / 2;
      pos.y += usePos.y + hotspot.top + hotspot.height / 2;
      x = pos.x;
      y = pos.y;
    } else {
      if (SQ_FAILED(sq_getinteger(v, 3, &x))) {
        return sq_throwerror(v, _SC("failed to get x"));
      }
      if (SQ_FAILED(sq_getinteger(v, 4, &y))) {
        return sq_throwerror(v, _SC("failed to get y"));
      }
    }
    obj->setPosition(sf::Vector2f(x, y));
    return 0;
  }

  static SQInteger objectScale(HSQUIRRELVM v) {
    SQFloat value;
    Object *obj = ScriptEngine::getObject(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    if (SQ_FAILED(sq_getfloat(v, 3, &value))) {
      return sq_throwerror(v, _SC("failed to get scale"));
    }
    obj->setScale(value);
    return 0;
  }

  static SQInteger objectScaleTo(HSQUIRRELVM v) {
    Object *obj = ScriptEngine::getObject(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    SQFloat value;
    if (SQ_FAILED(sq_getfloat(v, 3, &value))) {
      return sq_throwerror(v, _SC("failed to get scale"));
    }
    SQFloat t;
    if (SQ_FAILED(sq_getfloat(v, 4, &t))) {
      return sq_throwerror(v, _SC("failed to get time"));
    }
    SQInteger interpolation;
    if (SQ_FAILED(sq_getinteger(v, 5, &interpolation))) {
      interpolation = 0;
    }
    obj->scaleTo(value, sf::seconds(t), toInterpolationMethod(interpolation));
    return 0;
  }

  static SQInteger objectPosX(HSQUIRRELVM v) {
    Object *obj = ScriptEngine::getObject(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    auto pos = obj->getRealPosition();
    auto hotspot = obj->getHotspot();
    auto usePos = obj->getUsePosition().value_or(sf::Vector2f());
    sq_pushinteger(v, static_cast<SQInteger>(pos.x + usePos.x + hotspot.left + hotspot.width / 2));
    return 1;
  }

  static SQInteger objectPosY(HSQUIRRELVM v) {
    Object *obj = ScriptEngine::getObject(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    auto pos = obj->getRealPosition();
    auto hotspot = obj->getHotspot();
    auto usePos = obj->getUsePosition().value_or(sf::Vector2f());
    pos.y += usePos.y + hotspot.top + hotspot.height / 2;
    sq_pushinteger(v, static_cast<SQInteger>(pos.y));
    return 1;
  }

  static SQInteger objectRenderOffset(HSQUIRRELVM v) {
    Object *obj = ScriptEngine::getObject(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    SQInteger x;
    if (SQ_FAILED(sq_getinteger(v, 3, &x))) {
      return sq_throwerror(v, _SC("failed to get x"));
    }
    SQInteger y;
    if (SQ_FAILED(sq_getinteger(v, 4, &y))) {
      return sq_throwerror(v, _SC("failed to get y"));
    }
    obj->setRenderOffset({static_cast<int>(x), static_cast<int>(y)});
    return 0;
  }

  static SQInteger objectRoom(HSQUIRRELVM v) {
    Object *obj = ScriptEngine::getObject(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    auto pRoom = obj->getRoom();
    if (!pRoom) {
      sq_pushnull(v);
      return 1;
    }
    sq_pushobject(v, pRoom->getTable());
    return 1;
  }

  static SQInteger objectSort(HSQUIRRELVM v) {
    SQInteger zOrder;
    Object *obj = ScriptEngine::getObject(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    if (SQ_FAILED(sq_getinteger(v, 3, &zOrder))) {
      return sq_throwerror(v, _SC("failed to get zOrder"));
    }
    obj->setZOrder(static_cast<int>(zOrder));
    return 0;
  }

  static SQInteger objectRotate(HSQUIRRELVM v) {
    SQInteger angle;
    Object *obj = ScriptEngine::getObject(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    if (SQ_FAILED(sq_getinteger(v, 3, &angle))) {
      return sq_throwerror(v, _SC("failed to get angle"));
    }
    obj->setRotation(static_cast<float>(angle));
    return 0;
  }

  static SQInteger objectRotateTo(HSQUIRRELVM v) {
    SQFloat value;
    SQFloat t;
    Object *obj = ScriptEngine::getObject(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    if (SQ_FAILED(sq_getfloat(v, 3, &value))) {
      return sq_throwerror(v, _SC("failed to get value"));
    }
    if (SQ_FAILED(sq_getfloat(v, 4, &t))) {
      return sq_throwerror(v, _SC("failed to get time"));
    }
    SQInteger interpolation;
    if (SQ_FAILED(sq_getinteger(v, 5, &interpolation))) {
      interpolation = 0;
    }
    obj->rotateTo(value, sf::seconds(t), toInterpolationMethod(interpolation));
    return 0;
  }

  static SQInteger objectParallaxLayer(HSQUIRRELVM v) {
    SQInteger layer;
    Object *obj = ScriptEngine::getObject(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    if (SQ_FAILED(sq_getinteger(v, 3, &layer))) {
      return sq_throwerror(v, _SC("failed to get layer number"));
    }
    g_pEngine->getRoom()->setAsParallaxLayer(obj, static_cast<int>(layer));
    return 0;
  }

  static SQInteger objectParent(HSQUIRRELVM v) {
    auto pChild = ScriptEngine::getObject(v, 2);
    if (!pChild) {
      return sq_throwerror(v, _SC("failed to get child object"));
    }
    auto pParent = ScriptEngine::getObject(v, 3);
    if (!pParent) {
      return sq_throwerror(v, _SC("failed to get parent object"));
    }
    pChild->setParent(pParent);
    return 0;
  }

  static SQInteger objectTouchable(HSQUIRRELVM v) {
    SQInteger isTouchable;
    auto *obj = ScriptEngine::getEntity(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object or actor"));
    }
    auto numArgs = sq_gettop(v);
    if (numArgs == 2) {
      isTouchable = obj->isTouchable() ? 1 : 0;
      sq_pushinteger(v, isTouchable);
      return 1;
    }
    if (SQ_FAILED(sq_getinteger(v, 3, &isTouchable))) {
      return sq_throwerror(v, _SC("failed to get isTouchable"));
    }
    obj->setTouchable(isTouchable != 0);
    return 0;
  }

  static SQInteger objectLit(HSQUIRRELVM v) {
    SQInteger isLit;
    auto *obj = ScriptEngine::getEntity(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get actor or object"));
    }
    if (SQ_FAILED(sq_getinteger(v, 3, &isLit))) {
      return sq_throwerror(v, _SC("failed to get isLit parameter"));
    }
    obj->setLit(isLit != 0);
    return 0;
  }

  static SQInteger objectOwner(HSQUIRRELVM v) {
    auto *obj = ScriptEngine::getObject(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object"));
    }

    Actor *pActor = obj->getOwner();
    if (!pActor) {
      sq_pushnull(v);
      return 1;
    }

    sq_pushobject(v, pActor->getTable());
    return 1;
  }

  static SQInteger objectUsePos(HSQUIRRELVM v) {
    auto *obj = ScriptEngine::getObject(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    SQInteger x, y, dir;
    if (SQ_FAILED(sq_getinteger(v, 3, &x))) {
      return sq_throwerror(v, _SC("failed to get x"));
    }
    if (SQ_FAILED(sq_getinteger(v, 4, &y))) {
      return sq_throwerror(v, _SC("failed to get y"));
    }
    if (SQ_FAILED(sq_getinteger(v, 5, &dir))) {
      return sq_throwerror(v, _SC("failed to get direction"));
    }
    obj->setUsePosition(sf::Vector2f(x, y));
    obj->setUseDirection(static_cast<UseDirection>(dir));
    return 0;
  }

  static SQInteger objectUsePosX(HSQUIRRELVM v) {
    auto *obj = ScriptEngine::getObject(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    auto usePos = obj->getUsePosition().value_or(sf::Vector2f());
    sq_pushinteger(v, (SQInteger) usePos.x);
    return 1;
  }

  static SQInteger objectUsePosY(HSQUIRRELVM v) {
    auto *obj = ScriptEngine::getObject(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    auto height = obj->getRoom()->getRoomSize().y;
    sq_pushinteger(v, (SQInteger) height - obj->getUsePosition().value_or(sf::Vector2f()).y);
    return 1;
  }

  static SQInteger objectCenter(HSQUIRRELVM v) {
    sf::Vector2f pos;
    Object *obj = ScriptEngine::getObject(v, 2);
    if (obj) {
      pos = obj->getRealPosition();
      auto usePos = obj->getUsePosition().value_or(sf::Vector2f());
      pos += usePos;
    } else {
      auto *actor = ScriptEngine::getActor(v, 2);
      if (!actor) {
        return sq_throwerror(v, _SC("failed to get object or actor"));
      }
      pos = actor->getPosition();
    }

    ScriptEngine::push(v, pos);
    return 1;
  }

  static SQInteger objectColor(HSQUIRRELVM v) {
    Object *obj = ScriptEngine::getObject(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    SQInteger color;
    if (SQ_FAILED(sq_getinteger(v, 3, &color))) {
      return sq_throwerror(v, _SC("failed to get color"));
    }
    sf::Uint8 r, g, b;
    r = (color & 0x00FF0000) >> 16;
    g = (color & 0x0000FF00) >> 8;
    b = (color & 0x000000FF);
    obj->setColor(sf::Color(r, g, b));
    return 0;
  }

  static SQInteger objectDependentOn(HSQUIRRELVM v) {
    Object *childObject = ScriptEngine::getObject(v, 2);
    if (!childObject) {
      return sq_throwerror(v, _SC("failed to get childObject"));
    }
    Object *parentObject = ScriptEngine::getObject(v, 3);
    if (!parentObject) {
      return sq_throwerror(v, _SC("failed to get parentObject"));
    }
    SQInteger state;
    if (SQ_FAILED(sq_getinteger(v, 4, &state))) {
      return sq_throwerror(v, _SC("failed to get state"));
    }
    childObject->dependentOn(parentObject, state);
    return 0;
  }

  static SQInteger objectIcon(HSQUIRRELVM v) {
    Object *obj = ScriptEngine::getObject(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    if (sq_gettype(v, 3) == OT_STRING) {
      const SQChar *icon;
      if (SQ_FAILED(sq_getstring(v, 3, &icon))) {
        return sq_throwerror(v, _SC("failed to get icon"));
      }
      obj->setIcon(icon);
      return 0;
    }
    if (sq_gettype(v, 3) == OT_ARRAY) {
      SQInteger fps = 0;
      const SQChar *icon = nullptr;
      std::vector<std::string> icons;
      sq_push(v, 3);
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
      sq_pop(v, 2); // pops the null iterator and object
      obj->setIcon(fps, icons);
      return 0;
    }
    error("TODO: objectIcon with type {} not implemented", sq_gettype(v, 3));
    return 0;
  }

  static SQInteger objectFPS(HSQUIRRELVM v) {
    auto obj = ScriptEngine::getEntity(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object or actor"));
    }
    SQInteger fps;
    if (SQ_FAILED(sq_getinteger(v, 3, &fps))) {
      return sq_throwerror(v, _SC("failed to get fps"));
    }
    obj->setFps(fps);
    return 0;
  }

  static SQInteger objectValidUsePos(HSQUIRRELVM v) {
    Object *obj = ScriptEngine::getObject(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    sq_pushbool(v, obj->getUsePosition() != sf::Vector2f() ? SQTrue : SQFalse);
    return 1;
  }

  static SQInteger objectValidVerb(HSQUIRRELVM v) {
    auto *obj = ScriptEngine::getEntity(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object or actor"));
    }
    SQInteger verb;
    if (SQ_FAILED(sq_getinteger(v, 3, &verb))) {
      return sq_throwerror(v, _SC("failed to get verb"));
    }
    std::string name = Verb::getName(verb);
    sq_pushobject(v, obj->getTable());
    sq_pushstring(v, name.data(), -1);
    SQInteger isValid = 0;
    if (SQ_SUCCEEDED(sq_get(v, -2))) {
      isValid = 1;
    }
    sq_pop(v, 2);
    sq_pushinteger(v, isValid);
    return 1;
  }

  static SQInteger objectShader(HSQUIRRELVM) {
    error("TODO: objectShader: not implemented");
    return 0;
  }

  static SQInteger objectHidden(HSQUIRRELVM v) {
    SQInteger hidden;
    Object *obj = ScriptEngine::getObject(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object"));
    }
    if (SQ_FAILED(sq_getinteger(v, 3, &hidden))) {
      return sq_throwerror(v, _SC("failed to get hidden"));
    }
    obj->setVisible(hidden == 0);
    return 0;
  }

  static SQInteger pickupObject(HSQUIRRELVM v) {
    auto object = ScriptEngine::getObject(v, 2);
    if (!object) {
      return sq_throwerror(v, _SC("failed to get object to pickup"));
    }

    auto actor = ScriptEngine::getActor(v, 3);
    if (!actor) {
      actor = g_pEngine->getCurrentActor();
    }

    if (!actor) {
      error("There is no actor to pickup object");
      return 0;
    }

    if (!object->getRoom()) {
      // if the object is not in a room, than no need to animate the actor
      actor->pickupObject(object);
      return 0;
    }

    sq_pushobject(v, object->getTable());
    sq_pushstring(v, _SC("flags"), -1);
    if (SQ_FAILED(sq_rawget(v, -2))) {
      sq_pushstring(v, _SC("flags"), -1);
      sq_pushinteger(v, 0);
      sq_newslot(v, -3, SQFalse);
    }

    actor->pickupObject(object);

    return 0;
  }

  static SQInteger pickupReplacementObject(HSQUIRRELVM v) {
    auto *obj1 = ScriptEngine::getObject(v, 2);
    if (!obj1) {
      return sq_throwerror(v, _SC("failed to get object 1"));
    }
    auto *obj2 = ScriptEngine::getObject(v, 3);
    if (!obj2) {
      return sq_throwerror(v, _SC("failed to get object 2"));
    }
    auto actor = obj2->getOwner();
    assert(actor);
    actor->pickupReplacementObject(obj2, obj1);
    return 0;
  }

  static SQInteger createTextObject(HSQUIRRELVM v) {
    auto numArgs = sq_gettop(v);

    const SQChar *fontName;
    if (SQ_FAILED(sq_getstring(v, 2, &fontName))) {
      return sq_throwerror(v, _SC("failed to get fontName"));
    }
    auto &obj = g_pEngine->getRoom()->createTextObject(fontName);

    const SQChar *text;
    if (SQ_FAILED(sq_getstring(v, 3, &text))) {
      return sq_throwerror(v, _SC("failed to get text"));
    }
    std::string s(text);
    obj.setText(s);
    if (numArgs == 4) {
      SQInteger alignment;
      if (SQ_FAILED(sq_getinteger(v, 4, &alignment))) {
        return sq_throwerror(v, _SC("failed to get alignment"));
      }
      obj.setAlignment((TextAlignment) (alignment & (int) TextAlignment::All));
      int otherOptions = (alignment & ~(int) TextAlignment::All);
      // TODO: auto lessSpacing = (otherOptions & 0x2000000);
      auto maxWidth = (otherOptions & ~0x2000000);
      obj.setMaxWidth(maxWidth);
    }
    ScriptEngine::push<Object*>(v, &obj);
    return 1;
  }

  static SQInteger setDefaultObject(HSQUIRRELVM v) {
    auto &defaultObj = g_pEngine->getDefaultObject();
    sq_getstackobj(v, 2, &defaultObj);
    sq_addref(v, &defaultObj);
    return 0;
  }

  static SQInteger shakeObject(HSQUIRRELVM) {
    error("TODO: shakeObject: not implemented");
    return 0;
  }

  static SQInteger stopObjectMotors(HSQUIRRELVM v) {
    auto *obj = ScriptEngine::getEntity(v, 2);
    if (!obj) {
      return sq_throwerror(v, _SC("failed to get object or actor"));
    }
    obj->stopObjectMotors();
    return 0;
  }

  static SQInteger deleteObject(HSQUIRRELVM v) {
    auto *obj = ScriptEngine::getObject(v, 2);
    if (!obj) {
      // this function can be called with null
      return 0;
    }
    g_pEngine->getRoom()->deleteObject(*obj);
    return 0;
  }

  static SQInteger createObject(HSQUIRRELVM v) {
    auto numArgs = sq_gettop(v);
    if (numArgs == 1) {
      auto &obj = g_pEngine->getRoom()->createObject();
      ScriptEngine::push(v, &obj);
      return 1;
    }

    if (numArgs == 2) {
      std::vector<std::string> anims;
      for (int i = 0; i < numArgs - 1; i++) {
        const SQChar *animName;
        sq_getstring(v, 2 + i, &animName);
        anims.emplace_back(animName);
      }
      auto &obj = g_pEngine->getRoom()->createObject(anims);
      ScriptEngine::push(v, &obj);
      return 1;
    }

    HSQOBJECT obj;
    const SQChar *sheet;
    sq_getstring(v, 2, &sheet);
    sq_getstackobj(v, 3, &obj);
    if (sq_isarray(obj)) {
      const SQChar *name;
      std::vector<std::string> anims;
      sq_push(v, 3);
      sq_pushnull(v); // null iterator
      while (SQ_SUCCEEDED(sq_next(v, -2))) {
        sq_getstring(v, -1, &name);
        anims.emplace_back(name);
        sq_pop(v, 2);
      }
      sq_pop(v, 1); // pops the null iterator
      auto &object = g_pEngine->getRoom()->createObject(sheet, anims);
      ScriptEngine::push(v, &object);
      return 1;
    }

    if (sq_isstring(obj)) {
      const SQChar *image;
      sq_getstring(v, 3, &image);
      std::string s;
      s.append(image);
      std::size_t pos = s.find('.');
      if (pos == std::string::npos) {
        std::vector<std::string> anims{s};
        auto &object = g_pEngine->getRoom()->createObject(sheet, anims);
        ScriptEngine::push(v, &object);
        return 1;
      }

      s = s.substr(0, pos);
      auto &object = g_pEngine->getRoom()->createObject(s);
      ScriptEngine::push(v, &object);
      return 1;
    }

    return sq_throwerror(v, _SC("createObject called with invalid number of arguments"));
  }
};

Engine *_ObjectPack::g_pEngine = nullptr;

} // namespace ng
