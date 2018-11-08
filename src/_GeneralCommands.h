#pragma once
#include <squirrel3/squirrel.h>

namespace gg
{
class _GeneralPack : public Pack
{
  private:
    static GGEngine *g_pEngine;

  private:
    void addTo(ScriptEngine &engine) const override
    {
        g_pEngine = &engine.getEngine();
        engine.registerGlobalFunction(random, "random");
        engine.registerGlobalFunction(randomFrom, "randomfrom");
        engine.registerGlobalFunction(randomOdds, "randomOdds");
        engine.registerGlobalFunction(cameraInRoom, "cameraInRoom");
        engine.registerGlobalFunction(translate, "translate");
        engine.registerGlobalFunction(cameraAt, "cameraAt");
        engine.registerGlobalFunction(cameraPanTo, "cameraPanTo");
        engine.registerGlobalFunction(setVerb, "setVerb");
    }

    static SQInteger cameraAt(HSQUIRRELVM v)
    {
        SQInteger x, y;
        if (SQ_FAILED(sq_getinteger(v, 2, &x)))
        {
            return sq_throwerror(v, _SC("failed to get x"));
        }
        if (SQ_FAILED(sq_getinteger(v, 3, &y)))
        {
            return sq_throwerror(v, _SC("failed to get y"));
        }
        g_pEngine->setCameraAt(sf::Vector2f(x - Screen::HalfWidth, y - Screen::HalfHeight));
        return 0;
    }

    static SQInteger cameraPanTo(HSQUIRRELVM v)
    {
        SQInteger x, y, interpolation;
        SQFloat t;
        if (SQ_FAILED(sq_getinteger(v, 2, &x)))
        {
            return sq_throwerror(v, _SC("failed to get x"));
        }
        if (SQ_FAILED(sq_getinteger(v, 3, &y)))
        {
            return sq_throwerror(v, _SC("failed to get y"));
        }
        if (SQ_FAILED(sq_getfloat(v, 4, &t)))
        {
            return sq_throwerror(v, _SC("failed to get time"));
        }
        if (SQ_FAILED(sq_getinteger(v, 5, &interpolation)))
        {
            interpolation = 0;
        }
        auto get = std::bind(&GGEngine::getCameraAt, g_pEngine);
        auto set = std::bind(&GGEngine::setCameraAt, g_pEngine, std::placeholders::_1);
        auto method = ScriptEngine::getInterpolationMethod((InterpolationMethod)interpolation);

        auto cameraPanTo = std::make_unique<ChangeProperty<sf::Vector2f>>(get, set, sf::Vector2f(x - Screen::HalfWidth, y - Screen::HalfHeight), sf::seconds(t), method);
        g_pEngine->addFunction(std::move(cameraPanTo));
        return 0;
    }

    static SQInteger int_rand(SQInteger min, SQInteger max)
    {
        max++;
        auto value = rand() % (max - min) + min;
        return value;
    }

    static float float_rand(float min, float max)
    {
        float scale = rand() / (float)RAND_MAX; /* [0, 1.0] */
        return min + scale * (max - min);       /* [min, max] */
    }

    static SQInteger random(HSQUIRRELVM v)
    {
        if (sq_gettype(v, 2) == OT_INTEGER)
        {
            SQInteger min = 0;
            SQInteger max = 0;
            sq_getinteger(v, 2, &min);
            sq_getinteger(v, 3, &max);
            auto value = int_rand(min, max);
            sq_pushinteger(v, value);

            return 1;
        }
        {
            SQFloat min = 0;
            SQFloat max = 0;
            sq_getfloat(v, 2, &min);
            sq_getfloat(v, 3, &max);
            auto value = float_rand(min, max);
            sq_pushfloat(v, value);
            return 1;
        }
    }

    static SQInteger randomOdds(HSQUIRRELVM v)
    {
        SQFloat value = 0;
        if (SQ_FAILED(sq_getfloat(v, 2, &value)))
        {
            return sq_throwerror(v, _SC("failed to get value"));
        }
        auto rnd = float_rand(0, 1);
        sq_pushbool(v, static_cast<SQBool>(rnd <= value));
        return 1;
    }

    static SQInteger randomFrom(HSQUIRRELVM v)
    {
        auto size = sq_gettop(v);
        auto index = int_rand(0, size - 2);
        sq_push(v, 2 + index);
        return 1;
    }

    static SQInteger cameraInRoom(HSQUIRRELVM v)
    {
        HSQOBJECT table;
        sq_getstackobj(v, 2, &table);

        // get room instance
        sq_pushobject(v, table);
        sq_pushstring(v, _SC("instance"), -1);
        if (SQ_FAILED(sq_get(v, -2)))
        {
            return sq_throwerror(v, _SC("can't find instance entry"));
        }
        GGRoom *pRoom = nullptr;
        sq_getuserpointer(v, -1, (SQUserPointer *)&pRoom);
        sq_pop(v, 2);

        // set camera in room
        g_pEngine->setRoom(pRoom);

        // call enter room function
        sq_pushobject(v, table);
        sq_pushstring(v, "enter", 5);
        if (SQ_FAILED(sq_get(v, -2)))
        {
            return sq_throwerror(v, _SC("can't find enter function"));
        }

        sq_remove(v, -2);
        sq_pushobject(v, table);
        if (SQ_FAILED(sq_call(v, 1, SQTrue, SQTrue)))
        {
            return sq_throwerror(v, _SC("function enter call failed"));
        }
        return 0;
    }

    static SQInteger setVerb(HSQUIRRELVM v)
    {
        SQInteger actorSlot;
        if (SQ_FAILED(sq_getinteger(v, 2, &actorSlot)))
        {
            return sq_throwerror(v, _SC("failed to get actor slot"));
        }
        SQInteger verbSlot;
        if (SQ_FAILED(sq_getinteger(v, 3, &verbSlot)))
        {
            return sq_throwerror(v, _SC("failed to get verb slot"));
        }
        HSQOBJECT table;
        if (SQ_FAILED(sq_getstackobj(v, 4, &table)))
        {
            return sq_throwerror(v, _SC("failed to get verb definitionTable"));
        }
        if (!sq_istable(table))
        {
            return sq_throwerror(v, _SC("failed to get verb definitionTable"));
        }

        sq_pushobject(v, table);
        // id
        sq_pushstring(v, _SC("verb"), -1);
        if (SQ_FAILED(sq_get(v, -2)))
        {
            sq_pop(v, 2);
            return sq_throwerror(v, _SC("failed to get verb"));
        }

        const SQChar *id = nullptr;
        if (SQ_FAILED(sq_getstring(v, -1, &id)))
        {
            sq_pop(v, 2);
            return sq_throwerror(v, _SC("failed to get verb"));
        }
        sq_pop(v, 1);

        // image
        sq_pushstring(v, _SC("image"), -1);
        if (SQ_FAILED(sq_get(v, -2)))
        {
            sq_pop(v, 2);
            return sq_throwerror(v, _SC("failed to get image"));
        }

        const SQChar *image = nullptr;
        if (SQ_FAILED(sq_getstring(v, -1, &image)))
        {
            sq_pop(v, 2);
            return sq_throwerror(v, _SC("failed to get image"));
        }
        sq_pop(v, 1);

        // text
        sq_pushstring(v, _SC("text"), -1);
        if (SQ_FAILED(sq_get(v, -2)))
        {
            sq_pop(v, 2);
            return sq_throwerror(v, _SC("failed to get text"));
        }

        const SQChar *text = nullptr;
        if (SQ_FAILED(sq_getstring(v, -1, &text)))
        {
            sq_pop(v, 2);
            return sq_throwerror(v, _SC("failed to get text"));
        }
        sq_pop(v, 1);

        // key
        sq_pushstring(v, _SC("key"), -1);
        if (SQ_FAILED(sq_get(v, -2)))
        {
            sq_pop(v, 2);
            return sq_throwerror(v, _SC("failed to get key"));
        }

        const SQChar *key = nullptr;
        if (SQ_FAILED(sq_getstring(v, -1, &key)))
        {
            sq_pop(v, 2);
            return sq_throwerror(v, _SC("failed to get key"));
        }
        sq_pop(v, 2);

        VerbSlot slot;
        Verb verb;
        verb.id = id;
        verb.image = image;
        verb.text = text;
        verb.key = key;
        g_pEngine->setVerb(actorSlot - 1, verbSlot, verb);
        return 0;
    }

    static SQInteger translate(HSQUIRRELVM v)
    {
        const SQChar *idText;
        if (SQ_FAILED(sq_getstring(v, 2, &idText)))
        {
            return sq_throwerror(v, _SC("failed to get idText"));
        }
        std::string s(idText);
        s = s.substr(1);
        auto id = std::strtol(s.c_str(), nullptr, 10);
        auto text = g_pEngine->getText(id);
        sq_pushstring(v, text.c_str(), -1);
        return 1;
    }
};

GGEngine *_GeneralPack::g_pEngine = nullptr;

} // namespace gg
