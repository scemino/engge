#pragma once
#include "squirrel.h"
#include "InventoryObject.h"
#include "TextObject.h"

namespace ng
{
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
        engine.registerGlobalFunction(playState, "loopObjectState");
        engine.registerGlobalFunction(objectAlpha, "objectAlpha");
        engine.registerGlobalFunction(objectAlphaTo, "objectAlphaTo");
        engine.registerGlobalFunction(objectAt, "objectAt");
        engine.registerGlobalFunction(objectBumperCycle, "objectBumperCycle");
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
        engine.registerGlobalFunction(objectPosX, "objectPosX");
        engine.registerGlobalFunction(objectPosY, "objectPosY");
        engine.registerGlobalFunction(objectRoom, "objectRoom");
        engine.registerGlobalFunction(objectRotate, "objectRotate");
        engine.registerGlobalFunction(objectRotateTo, "objectRotateTo");
        engine.registerGlobalFunction(objectScale, "objectScale");
        engine.registerGlobalFunction(objectSort, "objectSort");
        engine.registerGlobalFunction(objectState, "objectState");
        engine.registerGlobalFunction(objectTouchable, "objectTouchable");
        engine.registerGlobalFunction(objectParallaxLayer, "objectParallaxLayer");
        engine.registerGlobalFunction(objectUsePos, "objectUsePos");
        engine.registerGlobalFunction(objectValidUsePos, "objectValidUsePos");
        engine.registerGlobalFunction(objectValidVerb, "objectValidVerb");
        engine.registerGlobalFunction(objectShader, "objectShader");
        engine.registerGlobalFunction(pickupObject, "pickupObject");
        engine.registerGlobalFunction(playState, "playObjectState");
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
        auto object = ScriptEngine::getEntity<Object>(v, 2);
        sq_pushbool(v, object ? SQTrue : SQFalse);
        return 1;
    }

    static SQInteger jiggleInventory(HSQUIRRELVM v)
    {
        std::cerr << "TODO: jiggleInventory: not implemented" << std::endl;
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

        auto method = ScriptEngine::getInterpolationMethod((InterpolationMethod)interpolation);

        auto getAlpha = [](const Object &o) {
            return (o.getColor().a / 255.f);
        };
        auto setAlpha = [](Object &o, float a) {
            const auto &c = o.getColor();
            return o.setColor(sf::Color(c.r, c.g, c.b, (sf::Uint8)(a * 255.f)));
        };
        auto getalpha = std::bind(getAlpha, std::cref(*obj));
        auto setalpha = std::bind(setAlpha, std::ref(*obj), std::placeholders::_1);
        auto alphaTo = std::make_unique<ChangeProperty<float>>(getalpha, setalpha, alpha, sf::seconds(time), method);
        g_pEngine->addFunction(std::move(alphaTo));

        return 0;
    }

    static SQInteger objectBumperCycle(HSQUIRRELVM v)
    {
        std::cerr << "TODO: objectBumperCycle: not implemented" << std::endl;
        return 0;
    }

    static SQInteger objectHotspot(HSQUIRRELVM v)
    {
        auto numArgs = sq_gettop(v) - 1;

        // TODO: with actor
        SQInteger left = 0;
        SQInteger top = 0;
        SQInteger right = 0;
        SQInteger bottom = 0;

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
        Entity *obj = ScriptEngine::getEntity<Entity>(v, 2);
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
        obj->move(sf::Vector2f(x, y));
        return 0;
    }

    static SQInteger objectState(HSQUIRRELVM v)
    {
        auto obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        auto numArgs = sq_gettop(v) - 3;
        if (numArgs == 1)
        {
            sq_pushinteger(v, obj->getStateAnimIndex());
            return 1;
        }

        SQInteger state;
        if (SQ_FAILED(sq_getinteger(v, 3, &state)))
        {
            return sq_throwerror(v, _SC("failed to get state"));
        }
        obj->setStateAnimIndex(state);
        std::cout << obj->getName() << "setStateAnimIndex(" << state << ")" << std::endl;

        return 0;
    }

    static SQInteger objectOffsetTo(HSQUIRRELVM v)
    {
        SQInteger x = 0;
        SQInteger y = 0;
        SQFloat t = 0;
        Entity *obj = ScriptEngine::getEntity<Entity>(v, 2);
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
        auto method = ScriptEngine::getInterpolationMethod((InterpolationMethod)interpolation);
        auto get = std::bind(&Entity::getPosition, obj);
        auto set = std::bind(&Entity::setPosition, obj, std::placeholders::_1);
        auto destination = obj->getDefaultPosition() + sf::Vector2f(x, y);
        auto offsetTo = std::make_unique<ChangeProperty<sf::Vector2f>>(get, set, destination, sf::seconds(t), method);
        g_pEngine->addFunction(std::move(offsetTo));

        return 0;
    }

    static SQInteger objectMoveTo(HSQUIRRELVM v)
    {
        SQInteger x = 0;
        SQInteger y = 0;
        SQFloat t = 0;
        Object *obj = ScriptEngine::getObject(v, 2);
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
        auto method = ScriptEngine::getInterpolationMethod((InterpolationMethod)interpolation);
        auto get = std::bind(&Object::getPosition, obj);
        auto set = std::bind(&Object::setPosition, obj, std::placeholders::_1);
        auto offsetTo = std::make_unique<ChangeProperty<sf::Vector2f>>(get, set, sf::Vector2f(x, y), sf::seconds(t), method);
        g_pEngine->addFunction(std::move(offsetTo));

        return 0;
    }

    static SQInteger playState(HSQUIRRELVM v)
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
        obj->setStateAnimIndex(static_cast<int>(index));
        return 0;
    }

    static SQInteger removeInventory(HSQUIRRELVM v)
    {
        std::cerr << "TODO: removeInventory: not implemented" << std::endl;
        return 0;
    }

    static SQInteger objectAt(HSQUIRRELVM v)
    {
        SQInteger x, y;
        auto numArgs = sq_gettop(v) - 1;
        Object *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        if (numArgs == 2)
        {
            Object *spot = ScriptEngine::getObject(v, 3);
            if (!spot)
            {
                return sq_throwerror(v, _SC("failed to get spot"));
            }
            x = spot->getPosition().x;
            y = spot->getPosition().y;
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
        SQFloat scale;
        Object *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        if (SQ_FAILED(sq_getfloat(v, 3, &scale)))
        {
            return sq_throwerror(v, _SC("failed to get scale"));
        }
        obj->setScale(scale);
        return 0;
    }

    static SQInteger objectPosX(HSQUIRRELVM v)
    {
        Object *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        auto pos = obj->getPosition();
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
        auto pos = obj->getPosition();
        sq_pushinteger(v, static_cast<SQInteger>(pos.y));
        return 1;
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
        sq_pushobject(v, *pRoom->getTable());
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
        SQInteger dir;
        SQInteger t;
        Object *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        if (SQ_FAILED(sq_getinteger(v, 3, &dir)))
        {
            return sq_throwerror(v, _SC("failed to get direction"));
        }
        if (SQ_FAILED(sq_getinteger(v, 4, &t)))
        {
            return sq_throwerror(v, _SC("failed to get time"));
        }
        SQInteger interpolation;
        if (SQ_FAILED(sq_getinteger(v, 5, &interpolation)))
        {
            interpolation = 0;
        }
        auto method = ScriptEngine::getInterpolationMethod((InterpolationMethod)interpolation);
        auto get = std::bind(&Object::getRotation, obj);
        auto set = std::bind(&Object::setRotation, obj, std::placeholders::_1);
        auto rotateTo = std::make_unique<ChangeProperty<float>>(get, set, dir, sf::seconds(t), method, (InterpolationMethod)interpolation == InterpolationMethod::Looping);
        g_pEngine->addFunction(std::move(rotateTo));
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

    static SQInteger objectTouchable(HSQUIRRELVM v)
    {
        SQBool isTouchable;
        auto *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        sq_tobool(v, 3, &isTouchable);
        obj->setTouchable(static_cast<bool>(isTouchable));
        return 0;
    }

    static SQInteger objectLit(HSQUIRRELVM v)
    {
        SQInteger isLit;
        auto *obj = ScriptEngine::getEntity<Entity>(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
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
        HSQOBJECT obj;
        sq_resetobject(&obj);
        if (SQ_FAILED(sq_getstackobj(v, 2, &obj)))
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }

        Actor *pActor = nullptr;
        for (const auto &actor : g_pEngine->getActors())
        {
            for (const auto &inventoryObj : actor->getObjects())
            {
                if (inventoryObj->getHandle()->_unVal.pTable == obj._unVal.pTable)
                {
                    pActor = actor.get();
                    break;
                }
            }
        }

        if (!pActor)
        {
            std::cerr << "Inventory object not found" << std::endl;
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
        std::cerr << "TODO: objectDependentOn: not implemented" << std::endl;
        return 0;
    }

    static SQInteger objectIcon(HSQUIRRELVM v)
    {
        Object *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        const SQChar *icon;
        if (SQ_FAILED(sq_getstring(v, 3, &icon)))
        {
            return sq_throwerror(v, _SC("failed to get icon"));
        }
        // TODO: obj->setIcon(icon);
        return 0;
    }

    static SQInteger objectFPS(HSQUIRRELVM v)
    {
        auto obj = ScriptEngine::getEntity<Entity>(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        SQInteger fps;
        if (SQ_FAILED(sq_getinteger(v, 3, &fps)))
        {
            return sq_throwerror(v, _SC("failed to get fps"));
        }
        // TODO: obj->setFps(icon);
        std::cerr << "TODO: setFps: not implemented" << std::endl;
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
        std::cerr << "TODO: objectValidVerb: not implemented" << std::endl;
        return 0;
    }

    static SQInteger objectShader(HSQUIRRELVM v)
    {
        std::cerr << "TODO: objectShader: not implemented" << std::endl;
        return 0;
    }

    static SQInteger objectHidden(HSQUIRRELVM v)
    {
        SQBool hidden;
        Object *obj = ScriptEngine::getObject(v, 2);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        sq_tobool(v, 3, &hidden);
        obj->setVisible(!hidden);
        return 0;
    }

    static SQInteger pickupObject(HSQUIRRELVM v)
    {
        auto pObj = std::make_unique<HSQOBJECT>();
        sq_resetobject(pObj.get());
        if (SQ_FAILED(sq_getstackobj(v, 2, pObj.get())))
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        sq_addref(v, pObj.get());
        sq_pushobject(v, *pObj.get());
        sq_pushstring(v, _SC("icon"), -1);
        if (SQ_FAILED(sq_get(v, -2)))
        {
            sq_pop(v, 2);
            return sq_throwerror(v, _SC("failed to get object icon"));
        }

        const SQChar *icon;
        if (SQ_FAILED(sq_getstring(v, -1, &icon)))
        {
            sq_pop(v, 2);
            return sq_throwerror(v, _SC("failed to get object icon"));
        }

        sq_pushobject(v, *pObj.get());
        sq_pushstring(v, _SC("name"), -1);
        if (SQ_FAILED(sq_get(v, -2)))
        {
            sq_pop(v, 2);
            return sq_throwerror(v, _SC("failed to get object name"));
        }

        std::string name;
        const SQChar *strName = nullptr;
        if (SQ_FAILED(sq_getstring(v, -1, &strName)))
        {
            sq_pop(v, 2);
            return sq_throwerror(v, _SC("failed to get object name"));
        }

        if (strlen(strName) > 0 && strName[0] == '@')
        {
            std::string s(strName);
            s = s.substr(1);
            auto id = std::strtol(s.c_str(), nullptr, 10);
            name = g_pEngine->getText(id);
        }
        else
        {
            name = strName;
        }

        auto actor = ScriptEngine::getActor(v, 3);
        if (!actor)
        {
            actor = g_pEngine->getCurrentActor();
        }
        if (!actor)
        {
            std::cerr << "There is no actor to pickup object " << icon << std::endl;
            return 0;
        }

        auto pObject = std::make_unique<InventoryObject>();
        pObject->setRoom(g_pEngine->getRoom());
        pObject->setName(name);
        pObject->setIcon(icon);
        pObject->setHandle(std::move(pObj));
        actor->pickupObject(std::move(pObject));
        return 0;
    }

    static SQInteger createTextObject(HSQUIRRELVM v)
    {
        auto numArgs = sq_gettop(v) - 1;

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
        if (numArgs == 3)
        {
            SQInteger alignment;
            if (SQ_FAILED(sq_getinteger(v, 4, &alignment)))
            {
                return sq_throwerror(v, _SC("failed to get alignment"));
            }
            obj.setAlignment((TextAlignment)alignment);
        }
        ScriptEngine::pushObject(v, obj);
        return 1;
    }

    static SQInteger setDefaultObject(HSQUIRRELVM v)
    {
        std::cerr << "TODO: setDefaultObject: not implemented" << std::endl;
        return 0;
    }

    static SQInteger shakeObject(HSQUIRRELVM v)
    {
        std::cerr << "TODO: shakeObject: not implemented" << std::endl;
        return 0;
    }

    static SQInteger stopObjectMotors(HSQUIRRELVM v)
    {
        std::cerr << "TODO: stopObjectMotors: not implemented" << std::endl;
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
        auto numArgs = sq_gettop(v) - 1;
        if (numArgs == 1)
        {
            std::vector<std::string> anims;
            for (int i = 0; i < numArgs; i++)
            {
                const SQChar *animName;
                sq_getstring(v, 2 + i, &animName);
                anims.emplace_back(animName);
            }
            auto &obj = g_pEngine->getRoom()->createObject(anims);
            ScriptEngine::pushObject(v, obj);
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
            sq_pushnull(v); //null iterator
            while (SQ_SUCCEEDED(sq_next(v, -2)))
            {
                sq_getstring(v, -1, &name);
                anims.emplace_back(name);
                sq_pop(v, 2);
            }
            sq_pop(v, 1); //pops the null iterator
            auto &object = g_pEngine->getRoom()->createObject(sheet, anims);
            ScriptEngine::pushObject(v, object);
        }
        else if (sq_isstring(obj))
        {
            const SQChar *image;
            sq_getstring(v, 3, &image);
            std::string s;
            s.append(image);
            std::size_t pos = s.find(".");
            s = s.substr(0, pos);
            auto &object = g_pEngine->getRoom()->createObject(s);
            ScriptEngine::pushObject(v, object);
        }
        return 1;
    }
};

Engine *_ObjectPack::g_pEngine = nullptr;

} // namespace ng
