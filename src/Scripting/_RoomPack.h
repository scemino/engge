#pragma once
#include "squirrel3/squirrel.h"
#include "Engine.h"
#include "_RoomTrigger.h"

namespace ng
{
class _RoomPack : public Pack
{
  private:
    static Engine *g_pEngine;

  private:
    void addTo(ScriptEngine &engine) const override
    {
        g_pEngine = &engine.getEngine();
        engine.registerGlobalFunction(addTrigger, "addTrigger");
        engine.registerGlobalFunction(removeTrigger, "removeTrigger");
        engine.registerGlobalFunction(roomFade, "roomFade");
        engine.registerGlobalFunction(defineRoom, "defineRoom");
        engine.registerGlobalFunction(walkboxHidden, "walkboxHidden");
    }

    static void _fadeTo(float a, const sf::Time &time)
    {
        auto get = std::bind(&Engine::getFadeAlpha, g_pEngine);
        auto set = std::bind(&Engine::setFadeAlpha, g_pEngine, std::placeholders::_1);
        auto fadeTo = std::make_unique<ChangeProperty<float>>(get, set, a, time);
        g_pEngine->addFunction(std::move(fadeTo));
    }

    static SQInteger addTrigger(HSQUIRRELVM v)
    {
        auto object = ScriptEngine::getObject(v, 2);
        if (!object)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        HSQOBJECT inside;
        if (SQ_FAILED(sq_getstackobj(v, 3, &inside)))
        {
            return sq_throwerror(v, _SC("failed to get insideTriggerFunction"));
        }
        HSQOBJECT outside;
        sq_resetobject(&outside);
        auto numArgs = sq_gettop(v) - 2;
        if (numArgs == 4)
        {
            if (SQ_FAILED(sq_getstackobj(v, 4, &outside)))
            {
                return sq_throwerror(v, _SC("failed to get outsideTriggerFunction"));
            }
        }
        auto trigger = std::make_shared<_RoomTrigger>(*g_pEngine, *object, v, inside, outside);
        object->addTrigger(trigger);

        return 0;
    }

    static SQInteger walkboxHidden(HSQUIRRELVM v)
    {
        const SQChar *name = nullptr;
        if (SQ_FAILED(sq_getstring(v, 2, &name)))
        {
            return sq_throwerror(v, _SC("failed to get walkbox name"));
        }
        SQBool hidden;
        if (SQ_FAILED(sq_getbool(v, 3, &hidden)))
        {
            return sq_throwerror(v, _SC("failed to get hidden value"));
        }
        auto &walkboxes = g_pEngine->getRoom().getWalkboxes();
        auto it = std::find_if(walkboxes.begin(), walkboxes.end(), [&name](std::unique_ptr<Walkbox> &walkbox) 
        {
            return walkbox->getName() == name; 
        });
        if (it == walkboxes.end())
        {
            std::string s;
            s.append("walkbox ").append(name).append(" has not been found");
            return sq_throwerror(v, s.data());
        }
        it->get()->setEnabled(hidden == SQFalse ? true : false);
        return 0;
    }

    static SQInteger removeTrigger(HSQUIRRELVM v)
    {
        auto object = ScriptEngine::getObject(v, 2);
        if (!object)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        object->removeTrigger();
        return 0;
    }

    static SQInteger roomFade(HSQUIRRELVM v)
    {
        SQInteger type;
        SQFloat t;
        if (SQ_FAILED(sq_getinteger(v, 2, &type)))
        {
            return sq_throwerror(v, _SC("failed to get type"));
        }
        if (SQ_FAILED(sq_getfloat(v, 3, &t)))
        {
            return sq_throwerror(v, _SC("failed to get time"));
        }
        _fadeTo(type == 0 ? 0.f : 1.f, sf::seconds(t));
        return 0;
    }

    static void setObjectSlot(HSQUIRRELVM v, const SQChar *name, Object &object)
    {
        sq_pushstring(v, name, -1);
        ScriptEngine::pushObject(v, object);
        sq_pushstring(v, _SC("name"), -1);
        sq_pushstring(v, object.getName().data(), -1);
        sq_newslot(v, -3, SQFalse);
        sq_newslot(v, -3, SQFalse);
    }

    template <typename T>
    static T _get(HSQUIRRELVM v);

    template <typename T>
    static void _getField(HSQUIRRELVM v, HSQOBJECT object, const SQChar *name, std::function<void(T)> func)
    {
        sq_pushobject(v, object);
        sq_pushstring(v, _SC(name), -1);
        if (SQ_SUCCEEDED(sq_get(v, -2)))
        {
            T value = _get<T>(v);
            func(value);
        }
    }

    static SQInteger defineRoom(HSQUIRRELVM v)
    {
        auto pRoom = std::make_unique<Room>(g_pEngine->getTextureManager(), g_pEngine->getSettings());
        HSQOBJECT table;
        sq_getstackobj(v, 2, &table);

        // loadRoom
        sq_pushobject(v, table);
        sq_pushstring(v, _SC("background"), 10);
        if (SQ_FAILED(sq_get(v, -2)))
        {
            return sq_throwerror(v, _SC("can't find background entry"));
        }
        const SQChar *name;
        sq_getstring(v, -1, &name);
        sq_pop(v, 2);
        pRoom->load(name);

        // define instance
        sq_pushobject(v, table);
        sq_pushstring(v, _SC("instance"), -1);
        sq_pushuserpointer(v, pRoom.get());
        sq_newslot(v, -3, SQFalse);

        // define room objects
        for (auto &obj : pRoom->getObjects())
        {
            sq_pushobject(v, table);
            sq_pushstring(v, obj->getName().data(), -1);
            if (SQ_FAILED(sq_get(v, -2)))
            {
                setObjectSlot(v, obj->getName().data(), *obj);
                continue;
            }

            HSQOBJECT object;
            sq_resetobject(&object);
            sq_getstackobj(v, -1, &object);
            if (!sq_istable(object))
            {
                return sq_throwerror(v, _SC("object should be a table entry"));
            }

            _getField<SQInteger>(v, object, _SC("initState"), [&obj](SQInteger value) { obj->setStateAnimIndex(value); });
            _getField<SQBool>(v, object, _SC("initTouchable"), [&obj](SQBool value) { obj->setTouchable(value == SQTrue); });
            _getField<const SQChar *>(v, object, _SC("defaultVerb"), [&obj](const SQChar *value) { obj->setDefaultVerb(value); });
            _getField<const SQChar *>(v, object, _SC("name"), [&obj](const SQChar *value) {
                if (strlen(value) > 0 && value[0] == '@')
                {
                    std::string s(value);
                    s = s.substr(1);
                    auto id = std::strtol(s.c_str(), nullptr, 10);
                    auto text = g_pEngine->getText(id);
                    obj->setId(obj->getName());
                    obj->setName(text);
                }
                else
                {
                    obj->setId(obj->getName());
                    obj->setName(value);
                }
            });

            sq_pushobject(v, object);
            sq_pushstring(v, _SC("instance"), -1);
            sq_pushuserpointer(v, obj.get());
            sq_newslot(v, -3, SQFalse);

            sq_pushobject(v, object);
            sq_pushobject(v, table);
            sq_setdelegate(v, -2);
        }

        g_pEngine->addRoom(std::move(pRoom));
        return 0;
    }
};

template <>
SQInteger _RoomPack::_get(HSQUIRRELVM v)
{
    SQInteger value = 0;
    sq_getinteger(v, -1, &value);
    return value;
}

template <>
SQBool _RoomPack::_get(HSQUIRRELVM v)
{
    SQBool value = false;
    sq_getbool(v, -1, &value);
    return value;
}

template <>
const SQChar *_RoomPack::_get(HSQUIRRELVM v)
{
    const SQChar *value = nullptr;
    sq_getstring(v, -1, &value);
    return value;
}

Engine *_RoomPack::g_pEngine = nullptr;

} // namespace ng