#pragma once
#include "squirrel3/squirrel.h"
#include "GGEngine.h"

namespace gg
{
class _RoomPack : public Pack
{
  private:
    static GGEngine *g_pEngine;

  private:
    void addTo(ScriptEngine &engine) const override
    {
        g_pEngine = &engine.getEngine();
        engine.registerGlobalFunction(roomFade, "roomFade");
        engine.registerGlobalFunction(defineRoom, "defineRoom");
    }

    static void _fadeTo(float a, const sf::Time &time)
    {
        auto get = std::bind(&GGEngine::getFadeAlpha, g_pEngine);
        auto set = std::bind(&GGEngine::setFadeAlpha, g_pEngine, std::placeholders::_1);
        auto fadeTo = std::make_unique<ChangeProperty<float>>(get, set, a, time);
        g_pEngine->addFunction(std::move(fadeTo));
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

    static void setObjectSlot(HSQUIRRELVM v, const SQChar *name, GGObject &object)
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
        auto pRoom = std::make_unique<GGRoom>(g_pEngine->getTextureManager(), g_pEngine->getSettings());
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
            if (SQ_SUCCEEDED(sq_setdelegate(v, -2)))
            {
                std::cout << "sq_setdelegate: " << obj->getName() << " OK" << std::endl;
            }
            else
            {
                std::cout << "sq_setdelegate: " << obj->getName() << " failed!" << std::endl;
            }
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

GGEngine *_RoomPack::g_pEngine = nullptr;

} // namespace gg