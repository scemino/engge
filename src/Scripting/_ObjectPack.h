#pragma once
#include "TextObject.h"
#include "squirrel.h"

namespace ng
{
class _PickupAnim : public Function
{
  public:
    _PickupAnim(Actor &actor, std::unique_ptr<Object> obj, const std::string &anim)
        : _actor(actor), _object(std::move(obj)), _animName(anim)
    {
    }

  private:
    bool isElapsed() override { return _state == 5; }

    void playAnim(const std::string &name)
    {
        trace("Play anim {}", name);
        _actor.getCostume().setState(name);
        _pAnim = _actor.getCostume().getAnimation();
        if (_pAnim)
        {
            _pAnim->play(false);
        }
    }

    void operator()(const sf::Time &elapsed) override
    {
        switch (_state)
        {
            case 0:
                playAnim(_animName);
                _state = 1;
                break;
            case 1:
                if (!_pAnim->isPlaying())
                    _state = 2;
                break;
            case 2:
                _actor.pickupObject(std::move(_object));
                _state = 3;
                break;
            case 3:
                playAnim("stand");
                _state = 4;
                break;
            case 4:
                if (!_pAnim->isPlaying())
                    _state = 5;
                break;
        }
    }

  private:
    int32_t _state{0};
    Actor &_actor;
    std::unique_ptr<Object> _object;
    std::string _animName;
    CostumeAnimation *_pAnim{nullptr};
};

class _ObjectPack : public Pack
{
  private:
    static Engine *g_pEngine;

  private:
    void addTo(ScriptEngine &engine) const override
    {
        g_pEngine = &engine.getEngine();
        engine.registerGlobalFunction(createObject, "createObject");
        engine.registerGlobalFunction(createTextObject, "createTextObject");
        engine.registerGlobalFunction(deleteObject, "deleteObject");
        engine.registerGlobalFunction(findObjectAt, "findObjectAt");
        engine.registerGlobalFunction(isObject, "is_object");
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
        engine.registerGlobalFunction(playObjectState, "playObjectState");
        engine.registerGlobalFunction(removeInventory, "removeInventory");
        engine.registerGlobalFunction(setDefaultObject, "setDefaultObject");
        engine.registerGlobalFunction(scale, "scale");
        engine.registerGlobalFunction(shakeObject, "shakeObject");
        engine.registerGlobalFunction(stopObjectMotors, "stopObjectMotors");
    }

    static SQInteger findObjectAt(HSQUIRRELVM v)
    {
        SQInteger x = 0;
        if (SQ_FAILED(sq_getinteger(v, 2, &x)))
        {
            return sq_throwerror(v, _SC("failed to get x"));
        }
        SQInteger y = 0;
        if (SQ_FAILED(sq_getinteger(v, 3, &y)))
        {
            return sq_throwerror(v, _SC("failed to get y"));
        }
        auto *room = g_pEngine->getRoom();
        auto &objects = room->getObjects();

        for (auto &obj : objects)
        {
            if (obj->getRealHotspot().contains(sf::Vector2i(x, y)))
            {
                sq_pushobject(v, obj->getTable());
                return 1;
            }
        }

        sq_pushnull(v);
        return 1;
    }

    static SQInteger isObject(HSQUIRRELVM v)
    {
        auto object = ScriptEngine::getObject(v, 2);
        sq_pushbool(v, object ? SQTrue : SQFalse);
        return 1;
    }

    static SQInteger jiggleInventory(HSQUIRRELVM v)
    {
        error("TODO: jiggleInventory: not implemented");
        return 0;
    }

    static SQInteger jiggleObject(HSQUIRRELVM v)
    {
        error("TODO: jiggleObject: not implemented");
        return 0;
    }

    static SQInteger scale(HSQUIRRELVM v)
    {
        SQFloat s = 0;
        if (SQ_FAILED(sq_getfloat(v, 3, &s)))
        {
            return sq_throwerror(v, _SC("failed to get scale"));
        }
        Object *self = ScriptEngine::getObject(v, 2);
        if (!self)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        self->setScale(s);
        return 0;
    }

    static SQInteger objectAlpha(HSQUIRRELVM v)
    {
        if (sq_gettype(v, 2) == OT_NULL)
            return 0;

        Object *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        SQFloat alpha = 0;
        if (SQ_FAILED(sq_getfloat(v, 3, &alpha)))
        {
            return sq_throwerror(v, _SC("failed to get alpha"));
        }
        alpha = alpha > 1.f ? 1.f : alpha;
        alpha = alpha < 0.f ? 0.f : alpha;
        auto a = (sf::Uint8)(alpha * 255);
        auto color = obj->getColor();
        obj->setColor(sf::Color(color.r, color.g, color.b, a));
        return 0;
    }

    static SQInteger objectAlphaTo(HSQUIRRELVM v)
    {
        if (sq_gettype(v, 2) == OT_NULL)
            return 0;

        Object *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        SQFloat alpha = 0;
        if (SQ_FAILED(sq_getfloat(v, 3, &alpha)))
        {
            return sq_throwerror(v, _SC("failed to get alpha"));
        }
        alpha = alpha > 1.f ? 1.f : alpha;
        alpha = alpha < 0.f ? 0.f : alpha;
        SQFloat time = 0;
        if (SQ_FAILED(sq_getfloat(v, 4, &time)))
        {
            return sq_throwerror(v, _SC("failed to get time"));
        }
        SQInteger interpolation;
        if (SQ_FAILED(sq_getinteger(v, 5, &interpolation)))
        {
            interpolation = 0;
        }
        obj->alphaTo(alpha, sf::seconds(time), (InterpolationMethod)interpolation);
        return 0;
    }

    static SQInteger objectBumperCycle(HSQUIRRELVM v)
    {
        error("TODO: objectBumperCycle: not implemented");
        return 0;
    }

    static SQInteger objectHotspot(HSQUIRRELVM v)
    {
        SQInteger left = 0;
        SQInteger top = 0;
        SQInteger right = 0;
        SQInteger bottom = 0;

        auto numArgs = sq_gettop(v);

        Object *obj = ScriptEngine::getObject(v, 2);
        Actor *actor = nullptr;
        if (!obj)
        {
            actor = ScriptEngine::getActor(v, 2);
            if (!actor)
            {
                return sq_throwerror(v, _SC("failed to get object or actor"));
            }
        }
        if (numArgs == 2)
        {
            const auto &hotspot = obj->getHotspot();
            sq_newtable(v);
            sq_pushstring(v, _SC("x1"), -1);
            sq_pushinteger(v, hotspot.left);
            sq_newslot(v, -3, SQFalse);
            sq_pushstring(v, _SC("y1"), -1);
            sq_pushinteger(v, hotspot.top);
            sq_newslot(v, -3, SQFalse);
            sq_pushstring(v, _SC("x2"), -1);
            sq_pushinteger(v, hotspot.left + hotspot.width);
            sq_newslot(v, -3, SQFalse);
            sq_pushstring(v, _SC("y2"), -1);
            sq_pushinteger(v, hotspot.top + hotspot.height);
            sq_newslot(v, -3, SQFalse);
            return 1;
        }
        if (SQ_FAILED(sq_getinteger(v, 3, &left)))
        {
            return sq_throwerror(v, _SC("failed to get left"));
        }
        if (SQ_FAILED(sq_getinteger(v, 4, &top)))
        {
            return sq_throwerror(v, _SC("failed to get top"));
        }
        if (SQ_FAILED(sq_getinteger(v, 5, &right)))
        {
            return sq_throwerror(v, _SC("failed to get right"));
        }
        if (SQ_FAILED(sq_getinteger(v, 6, &bottom)))
        {
            return sq_throwerror(v, _SC("failed to get bottom"));
        }

        if (obj)
        {
            obj->setHotspot(sf::IntRect(static_cast<int>(left), static_cast<int>(top), static_cast<int>(right - left),
                                        static_cast<int>(bottom - top)));
        }
        else
        {
            actor->setHotspot(sf::IntRect(static_cast<int>(left), static_cast<int>(top), static_cast<int>(right - left),
                                          static_cast<int>(bottom - top)));
        }
        return 0;
    }

    static SQInteger objectOffset(HSQUIRRELVM v)
    {
        SQInteger x = 0;
        SQInteger y = 0;
        auto *obj = ScriptEngine::getEntity(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object or actor"));
        }
        if (SQ_FAILED(sq_getinteger(v, 3, &x)))
        {
            return sq_throwerror(v, _SC("failed to get x"));
        }
        if (SQ_FAILED(sq_getinteger(v, 4, &y)))
        {
            return sq_throwerror(v, _SC("failed to get y"));
        }
        obj->setOffset(sf::Vector2f(x, y));
        return 0;
    }

    static SQInteger objectScreenSpace(HSQUIRRELVM v)
    {
        error("TODO: objectScreenSpace: not implemented");
        return 0;
    }

    static SQInteger objectState(HSQUIRRELVM v)
    {
        auto obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        auto numArgs = sq_gettop(v) - 2;
        if (numArgs == 0)
        {
            sq_pushinteger(v, obj->getState());
            return 1;
        }

        SQInteger state;
        if (SQ_FAILED(sq_getinteger(v, 3, &state)))
        {
            return sq_throwerror(v, _SC("failed to get state"));
        }
        obj->setStateAnimIndex(state);

        return 0;
    }

    static SQInteger objectOffsetTo(HSQUIRRELVM v)
    {
        SQInteger x = 0;
        SQInteger y = 0;
        SQFloat t = 0;
        auto *obj = ScriptEngine::getEntity(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get entity"));
        }
        if (SQ_FAILED(sq_getinteger(v, 3, &x)))
        {
            return sq_throwerror(v, _SC("failed to get x"));
        }
        if (SQ_FAILED(sq_getinteger(v, 4, &y)))
        {
            return sq_throwerror(v, _SC("failed to get y"));
        }
        if (SQ_FAILED(sq_getfloat(v, 5, &t)))
        {
            return sq_throwerror(v, _SC("failed to get t"));
        }
        SQInteger interpolation;
        if (SQ_FAILED(sq_getinteger(v, 6, &interpolation)))
        {
            interpolation = 0;
        }
        obj->offsetTo(sf::Vector2f(x, y), sf::seconds(t), (InterpolationMethod)interpolation);
        return 0;
    }

    static SQInteger objectMoveTo(HSQUIRRELVM v)
    {
        SQInteger x = 0;
        SQInteger y = 0;
        SQFloat t = 0;
        Entity *obj = ScriptEngine::getEntity(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        if (SQ_FAILED(sq_getinteger(v, 3, &x)))
        {
            return sq_throwerror(v, _SC("failed to get x"));
        }
        if (SQ_FAILED(sq_getinteger(v, 4, &y)))
        {
            return sq_throwerror(v, _SC("failed to get y"));
        }
        if (SQ_FAILED(sq_getfloat(v, 5, &t)))
        {
            return sq_throwerror(v, _SC("failed to get t"));
        }
        SQInteger interpolation;
        if (SQ_FAILED(sq_getinteger(v, 6, &interpolation)))
        {
            interpolation = 0;
        }

        obj->moveTo(sf::Vector2f(x, y), sf::seconds(t), (InterpolationMethod)interpolation);
        return 0;
    }

    static SQInteger loopObjectState(HSQUIRRELVM v)
    {
        SQInteger index;
        Object *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        if (SQ_FAILED(sq_getinteger(v, 3, &index)))
        {
            return sq_throwerror(v, _SC("failed to get state"));
        }
        obj->playAnim(static_cast<int>(index), true);
        return 0;
    }

    static SQInteger playObjectState(HSQUIRRELVM v)
    {
        Object *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        if (sq_gettype(v, 3) == OT_INTEGER)
        {
            SQInteger index;
            if (SQ_FAILED(sq_getinteger(v, 3, &index)))
            {
                return sq_throwerror(v, _SC("failed to get state"));
            }
            obj->playAnim(static_cast<int>(index), false);
            return 0;
        }
        if (sq_gettype(v, 3) == OT_STRING)
        {
            const SQChar *state;
            if (SQ_FAILED(sq_getstring(v, 3, &state)))
            {
                return sq_throwerror(v, _SC("failed to get state"));
            }
            obj->playAnim(state, false);
            return 0;
        }
        return sq_throwerror(v, _SC("failed to get state"));
    }

    static SQInteger removeInventory(HSQUIRRELVM v)
    {
        Object *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        auto owner = obj->getOwner();
        if(owner)
        {
            owner->removeInventory(obj);
        }
        return 0;
    }

    static SQInteger objectAt(HSQUIRRELVM v)
    {
        SQInteger x, y;
        auto numArgs = sq_gettop(v);
        Object *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        if (numArgs == 3)
        {
            Object *spot = ScriptEngine::getObject(v, 3);
            if (!spot)
            {
                return sq_throwerror(v, _SC("failed to get spot"));
            }
            x = spot->getRealPosition().x;
            y = spot->getRealPosition().y;
        }
        else
        {
            if (SQ_FAILED(sq_getinteger(v, 3, &x)))
            {
                return sq_throwerror(v, _SC("failed to get x"));
            }
            if (SQ_FAILED(sq_getinteger(v, 4, &y)))
            {
                return sq_throwerror(v, _SC("failed to get y"));
            }
        }
        obj->setPosition(sf::Vector2f(x, y));
        return 0;
    }

    static SQInteger objectScale(HSQUIRRELVM v)
    {
        SQFloat value;
        Object *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        if (SQ_FAILED(sq_getfloat(v, 3, &value)))
        {
            return sq_throwerror(v, _SC("failed to get scale"));
        }
        obj->setScale(value);
        return 0;
    }

    static SQInteger objectScaleTo(HSQUIRRELVM v)
    {
        Object *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        SQFloat scale;
        if (SQ_FAILED(sq_getfloat(v, 3, &scale)))
        {
            return sq_throwerror(v, _SC("failed to get scale"));
        }
        SQFloat t;
        if (SQ_FAILED(sq_getfloat(v, 4, &t)))
        {
            return sq_throwerror(v, _SC("failed to get time"));
        }
        SQInteger interpolation;
        if (SQ_FAILED(sq_getinteger(v, 5, &interpolation)))
        {
            interpolation = 0;
        }
        obj->scaleTo(scale, sf::seconds(t), (InterpolationMethod)interpolation);
        return 0;
    }

    static SQInteger objectPosX(HSQUIRRELVM v)
    {
        Object *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        auto pos = obj->getRealPosition();
        sq_pushinteger(v, static_cast<SQInteger>(pos.x));
        return 1;
    }

    static SQInteger objectPosY(HSQUIRRELVM v)
    {
        Object *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        auto pos = obj->getRealPosition();
        sq_pushinteger(v, static_cast<SQInteger>(pos.y));
        return 1;
    }

    static SQInteger objectRenderOffset(HSQUIRRELVM v)
    {
        Object *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        SQInteger x;
        if (SQ_FAILED(sq_getinteger(v, 3, &x)))
        {
            return sq_throwerror(v, _SC("failed to get x"));
        }
        SQInteger y;
        if (SQ_FAILED(sq_getinteger(v, 4, &y)))
        {
            return sq_throwerror(v, _SC("failed to get y"));
        }
        obj->setRenderOffset({static_cast<int>(x), static_cast<int>(y)});
        return 0;
    }

    static SQInteger objectRoom(HSQUIRRELVM v)
    {
        Object *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        auto pRoom = obj->getRoom();
        if (!pRoom)
        {
            sq_pushnull(v);
            return 1;
        }
        sq_pushobject(v, pRoom->getTable());
        return 1;
    }

    static SQInteger objectSort(HSQUIRRELVM v)
    {
        SQInteger zOrder;
        Object *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        if (SQ_FAILED(sq_getinteger(v, 3, &zOrder)))
        {
            return sq_throwerror(v, _SC("failed to get zOrder"));
        }
        obj->setZOrder(static_cast<int>(zOrder));
        return 0;
    }

    static SQInteger objectRotate(HSQUIRRELVM v)
    {
        SQInteger angle;
        Object *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        if (SQ_FAILED(sq_getinteger(v, 3, &angle)))
        {
            return sq_throwerror(v, _SC("failed to get angle"));
        }
        obj->setRotation(static_cast<float>(angle));
        return 0;
    }

    static SQInteger objectRotateTo(HSQUIRRELVM v)
    {
        SQFloat value;
        SQFloat t;
        Object *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        if (SQ_FAILED(sq_getfloat(v, 3, &value)))
        {
            return sq_throwerror(v, _SC("failed to get value"));
        }
        if (SQ_FAILED(sq_getfloat(v, 4, &t)))
        {
            return sq_throwerror(v, _SC("failed to get time"));
        }
        SQInteger interpolation;
        if (SQ_FAILED(sq_getinteger(v, 5, &interpolation)))
        {
            interpolation = 0;
        }
        obj->rotateTo(value, sf::seconds(t), (InterpolationMethod)interpolation);
        return 0;
    }

    static SQInteger objectParallaxLayer(HSQUIRRELVM v)
    {
        SQInteger layer;
        Object *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        if (SQ_FAILED(sq_getinteger(v, 3, &layer)))
        {
            return sq_throwerror(v, _SC("failed to get layer number"));
        }
        g_pEngine->getRoom()->setAsParallaxLayer(obj, static_cast<int>(layer));
        return 0;
    }

    static SQInteger objectParent(HSQUIRRELVM v) { return sq_throwerror(v, _SC("objectParent not implemented")); }

    static SQInteger objectTouchable(HSQUIRRELVM v)
    {
        SQInteger isTouchable;
        auto *obj = ScriptEngine::getEntity(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object or actor"));
        }
        auto numArgs = sq_gettop(v);
        if (numArgs == 2)
        {
            isTouchable = obj->isTouchable() ? 1 : 0;
            sq_pushinteger(v, isTouchable);
            return 1;
        }
        if (SQ_FAILED(sq_getinteger(v, 3, &isTouchable)))
        {
            return sq_throwerror(v, _SC("failed to get isTouchable"));
        }
        obj->setTouchable(isTouchable != 0);
        return 0;
    }

    static SQInteger objectLit(HSQUIRRELVM v)
    {
        SQInteger isLit;
        auto *obj = ScriptEngine::getEntity(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get actor or object"));
        }
        if (SQ_FAILED(sq_getinteger(v, 3, &isLit)))
        {
            return sq_throwerror(v, _SC("failed to get isLit parameter"));
        }
        obj->setLit(isLit != 0);
        return 0;
    }

    static SQInteger objectOwner(HSQUIRRELVM v)
    {
        auto *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }

        Actor *pActor = obj->getOwner();
        if (!pActor)
        {
            sq_pushnull(v);
            return 1;
        }

        sq_pushobject(v, pActor->getTable());
        return 1;
    }

    static SQInteger objectUsePos(HSQUIRRELVM v)
    {
        auto *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        SQInteger x, y, dir;
        if (SQ_FAILED(sq_getinteger(v, 3, &x)))
        {
            return sq_throwerror(v, _SC("failed to get x"));
        }
        if (SQ_FAILED(sq_getinteger(v, 4, &y)))
        {
            return sq_throwerror(v, _SC("failed to get y"));
        }
        if (SQ_FAILED(sq_getinteger(v, 5, &dir)))
        {
            return sq_throwerror(v, _SC("failed to get direction"));
        }
        obj->setUsePosition(sf::Vector2f(x, y));
        obj->setUseDirection(static_cast<UseDirection>(dir));
        return 0;
    }

    static SQInteger objectUsePosX(HSQUIRRELVM v)
    {
        auto *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        sq_pushinteger(v, (SQInteger)obj->getUsePosition().x);
        return 1;
    }

    static SQInteger objectUsePosY(HSQUIRRELVM v)
    {
        auto *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        sq_pushinteger(v, (SQInteger)obj->getUsePosition().y);
        return 1;
    }

    static SQInteger objectCenter(HSQUIRRELVM v)
    {
        Object *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        auto rect = obj->getRealHotspot();
        sf::Vector2f pos(rect.left + rect.width / 2, rect.top + rect.height / 2);
        sq_newtable(v);
        sq_pushstring(v, _SC("x"), -1);
        sq_pushinteger(v, pos.x);
        sq_newslot(v, -3, SQFalse);
        sq_pushstring(v, _SC("y"), -1);
        sq_pushinteger(v, pos.y);
        sq_newslot(v, -3, SQFalse);
        return 1;
    }

    static SQInteger objectColor(HSQUIRRELVM v)
    {
        Object *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        SQInteger color;
        if (SQ_FAILED(sq_getinteger(v, 3, &color)))
        {
            return sq_throwerror(v, _SC("failed to get color"));
        }
        sf::Uint8 r, g, b;
        r = (color & 0x00FF0000) >> 16;
        g = (color & 0x0000FF00) >> 8;
        b = (color & 0x000000FF);
        obj->setColor(sf::Color(r, g, b));
        return 0;
    }

    static SQInteger objectDependentOn(HSQUIRRELVM v)
    {
        Object *childObject = ScriptEngine::getObject(v, 2);
        if (!childObject)
        {
            return sq_throwerror(v, _SC("failed to get childObject"));
        }
        Object *parentObject = ScriptEngine::getObject(v, 3);
        if (!parentObject)
        {
            return sq_throwerror(v, _SC("failed to get parentObject"));
        }
        SQInteger state;
        if (SQ_FAILED(sq_getinteger(v, 4, &state)))
        {
            return sq_throwerror(v, _SC("failed to get state"));
        }
        childObject->dependentOn(parentObject, state);
        return 0;
    }

    static SQInteger objectIcon(HSQUIRRELVM v)
    {
        Object *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        if(sq_gettype(v, 3) == OT_STRING)
        {
            const SQChar *icon;
            if (SQ_FAILED(sq_getstring(v, 3, &icon)))
            {
                return sq_throwerror(v, _SC("failed to get icon"));
            }
            obj->setIcon(icon);
            return 0;
        }
        if(sq_gettype(v, 3) == OT_ARRAY)
        {
            SQInteger fps = 0;
            const SQChar* icon = nullptr;
            std::vector<std::string> icons;
            sq_push(v, 3);
            sq_pushnull(v); // null iterator
            if(SQ_SUCCEEDED(sq_next(v, -2)))
            {
                sq_getinteger(v, -1, &fps);
                sq_pop(v, 2);
            }
            while (SQ_SUCCEEDED(sq_next(v, -2)))
            {
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

    static SQInteger objectFPS(HSQUIRRELVM v)
    {
        auto obj = ScriptEngine::getEntity(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object or actor"));
        }
        SQInteger fps;
        if (SQ_FAILED(sq_getinteger(v, 3, &fps)))
        {
            return sq_throwerror(v, _SC("failed to get fps"));
        }
        obj->setFps(fps);
        return 0;
    }

    static SQInteger objectValidUsePos(HSQUIRRELVM v)
    {
        Object *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        sq_pushbool(v, obj->getUsePosition() != sf::Vector2f() ? SQTrue : SQFalse);
        return 1;
    }

    static SQInteger objectValidVerb(HSQUIRRELVM v)
    {
        Object *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        SQInteger verb;
        if (SQ_FAILED(sq_getinteger(v, 3, &verb)))
        {
            return sq_throwerror(v, _SC("failed to get verb"));
        }
        std::string name = Verb::getName(verb);
        sq_pushobject(v, obj->getTable());
        sq_pushstring(v, name.data(), -1);
        SQInteger isValid = 0;
        if (SQ_SUCCEEDED(sq_get(v, -2)))
        {
            isValid = 1;
        }
        sq_pop(v, 2);
        sq_pushinteger(v, isValid);
        return 1;
    }

    static SQInteger objectShader(HSQUIRRELVM v)
    {
        error("TODO: objectShader: not implemented");
        return 0;
    }

    static SQInteger objectHidden(HSQUIRRELVM v)
    {
        SQInteger hidden;
        Object *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        if (SQ_FAILED(sq_getinteger(v, 3, &hidden)))
        {
            return sq_throwerror(v, _SC("failed to get hidden"));
        }
        obj->setVisible(hidden == 0);
        return 0;
    }

    static SQInteger pickupObject(HSQUIRRELVM v)
    {
        auto object = std::make_unique<Object>();
        auto& table = object->getTable();
        sq_getstackobj(v, 2, &table);

        sq_pushobject(v, table);
        sq_pushstring(v, _SC("instance"), -1);
        sq_pushuserpointer(v, object.get());
        sq_newslot(v, -3, SQFalse);

        sq_pushobject(v, table);
        sq_pushstring(v, _SC("name"), -1);
        if (SQ_SUCCEEDED(sq_get(v, -2)))
        {
            const SQChar* name;
            sq_getstring(v, -1, &name);

            if (strlen(name) > 0 && name[0] == '@')
            {
                std::string s(name);
                s = s.substr(1);
                auto id = std::strtol(s.c_str(), nullptr, 10);
                auto text = g_pEngine->getText(id);
                object->setId(text);
                object->setName(text);
            }
            else
            {
                object->setId(towstring(name));
                object->setName(towstring(name));
            }
        }

        sq_pushobject(v, table);
        sq_pushstring(v, _SC("icon"), -1);
        if (SQ_SUCCEEDED(sq_get(v, -2)))
        {
            const SQChar* icon;
            sq_getstring(v, -1, &icon);
            object->setIcon(icon);
        }

        auto actor = ScriptEngine::getActor(v, 3);
        if (actor)
        {
            actor->pickupObject(std::move(object));
            return 0;
        }

        actor = g_pEngine->getCurrentActor();
        if (!actor)
        {
            error("There is no actor to pickup object");
            return 0;
        }

        sq_pushobject(v, table);
        sq_pushstring(v, _SC("flags"), -1);
        if (SQ_FAILED(sq_rawget(v, -2)))
        {
            sq_pushstring(v, _SC("flags"), -1);
            sq_pushinteger(v, 0);
            sq_newslot(v, -3, SQFalse);
        }
        SQInteger flags = 0;
        if (SQ_SUCCEEDED(sq_rawget(v, -2)))
        {
            sq_getinteger(v, -1, &flags);
        }

        std::string anim;
        if ((flags & ObjectFlagConstants::REACH_HIGH) == ObjectFlagConstants::REACH_HIGH)
        {
            anim = "reach_high";
        }
        else if ((flags & ObjectFlagConstants::REACH_MED) == ObjectFlagConstants::REACH_MED)
        {
            anim = "reach_med";
        }
        else if ((flags & ObjectFlagConstants::REACH_LOW) == ObjectFlagConstants::REACH_LOW)
        {
            anim = "reach_low";
        }

        auto pPickupAnim = std::make_unique<_PickupAnim>(*actor, std::move(object), anim);
        g_pEngine->addFunction(std::move(pPickupAnim));
        
        return 0;
    }

    static SQInteger createTextObject(HSQUIRRELVM v)
    {
        auto numArgs = sq_gettop(v);

        const SQChar *fontName;
        if (SQ_FAILED(sq_getstring(v, 2, &fontName)))
        {
            return sq_throwerror(v, _SC("failed to get fontName"));
        }
        auto &obj = g_pEngine->getRoom()->createTextObject(fontName);

        const SQChar *text;
        if (SQ_FAILED(sq_getstring(v, 3, &text)))
        {
            return sq_throwerror(v, _SC("failed to get text"));
        }
        std::string s(text);
        obj.setText(s);
        if (numArgs == 4)
        {
            SQInteger alignment;
            if (SQ_FAILED(sq_getinteger(v, 4, &alignment)))
            {
                return sq_throwerror(v, _SC("failed to get alignment"));
            }
            obj.setAlignment((TextAlignment)alignment);
        }
        _createObject(v, obj);
        return 1;
    }

    static SQInteger setDefaultObject(HSQUIRRELVM v)
    {
        auto &defaultObj = g_pEngine->getDefaultObject();
        sq_getstackobj(v, 2, &defaultObj);
        sq_addref(v, &defaultObj);
        return 0;
    }

    static SQInteger shakeObject(HSQUIRRELVM v)
    {
        error("TODO: shakeObject: not implemented");
        return 0;
    }

    static SQInteger stopObjectMotors(HSQUIRRELVM v)
    {
        error("TODO: stopObjectMotors: not implemented");
        return 0;
    }

    static SQInteger deleteObject(HSQUIRRELVM v)
    {
        auto *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            // this function can be called with null
            return 0;
        }
        g_pEngine->getRoom()->deleteObject(*obj);
        return 0;
    }

    static SQInteger createObject(HSQUIRRELVM v)
    {
        auto numArgs = sq_gettop(v);
        if (numArgs == 2)
        {
            std::vector<std::string> anims;
            for (int i = 0; i < numArgs - 1; i++)
            {
                const SQChar *animName;
                sq_getstring(v, 2 + i, &animName);
                anims.emplace_back(animName);
            }
            auto &obj = g_pEngine->getRoom()->createObject(anims);
            _createObject(v, obj);
            return 1;
        }

        HSQOBJECT obj;
        const SQChar *sheet;
        sq_getstring(v, 2, &sheet);
        sq_getstackobj(v, 3, &obj);
        if (sq_isarray(obj))
        {
            const SQChar *name;
            std::vector<std::string> anims;
            sq_push(v, 3);
            sq_pushnull(v); // null iterator
            while (SQ_SUCCEEDED(sq_next(v, -2)))
            {
                sq_getstring(v, -1, &name);
                anims.emplace_back(name);
                sq_pop(v, 2);
            }
            sq_pop(v, 1); // pops the null iterator
            auto &object = g_pEngine->getRoom()->createObject(sheet, anims);
            _createObject(v, object);
            return 1;
        }

        if (sq_isstring(obj))
        {
            const SQChar *image;
            sq_getstring(v, 3, &image);
            std::string s;
            s.append(image);
            std::size_t pos = s.find('.');
            if (pos == std::string::npos)
            {
                std::vector<std::string> anims{s};
                auto &object = g_pEngine->getRoom()->createObject(sheet, anims);
                _createObject(v, object);
                return 1;
            }

            s = s.substr(0, pos);
            auto &object = g_pEngine->getRoom()->createObject(s);
            _createObject(v, object);
            return 1;
        }

        return sq_throwerror(v, _SC("createObject called with invalid number of arguments"));
    }

    static void _createObject(HSQUIRRELVM v, Object &object)
    {
        ScriptEngine::pushObject(v, object);
        auto &table = object.getTable();
        sq_getstackobj(v, -1, &table);
    }
};

Engine *_ObjectPack::g_pEngine = nullptr;

} // namespace ng
