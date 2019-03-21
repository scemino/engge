#pragma once
#include <string>
#include "squirrel.h"
#include "Cutscene.h"
#include "Screen.h"

namespace ng
{
class _GeneralPack : public Pack
{
  private:
    static Engine *g_pEngine;

  private:
    void addTo(ScriptEngine &engine) const override
    {
        g_pEngine = &engine.getEngine();
        engine.registerGlobalFunction(arrayShuffle, "arrayShuffle");
        engine.registerGlobalFunction(assetExists, "assetExists");
        engine.registerGlobalFunction(cameraAt, "cameraAt");
        engine.registerGlobalFunction(cameraBounds, "cameraBounds");
        engine.registerGlobalFunction(cameraFollow, "cameraFollow");
        engine.registerGlobalFunction(cameraInRoom, "cameraInRoom");
        engine.registerGlobalFunction(cameraPanTo, "cameraPanTo");
        engine.registerGlobalFunction(cutscene, "cutscene");
        engine.registerGlobalFunction(cutsceneOverride, "cutsceneOverride");
        engine.registerGlobalFunction(distance, "distance");
        engine.registerGlobalFunction(incutscene, "incutscene");
        engine.registerGlobalFunction(indialog, "indialog");
        engine.registerGlobalFunction(loadArray, "loadArray");
        engine.registerGlobalFunction(random, "random");
        engine.registerGlobalFunction(randomFrom, "randomfrom");
        engine.registerGlobalFunction(randomOdds, "randomOdds");
        engine.registerGlobalFunction(markProgress, "markProgress");
        engine.registerGlobalFunction(markStat, "markStat");
        engine.registerGlobalFunction(screenSize, "screenSize");
        engine.registerGlobalFunction(strsplit, "strsplit");
        engine.registerGlobalFunction(translate, "translate");
        engine.registerGlobalFunction(setVerb, "setVerb");
        engine.registerGlobalFunction(startDialog, "startDialog");
        engine.registerGlobalFunction(stopSentence, "stopSentence");
    }

    static SQInteger arrayShuffle(HSQUIRRELVM v)
    {
        HSQOBJECT array;
        sq_resetobject(&array);
        if (SQ_FAILED(sq_getstackobj(v, 2, &array)))
        {
            return sq_throwerror(v, "Failed to get array");
        }

        std::vector<HSQOBJECT> objs;
        sq_pushobject(v, array);
        sq_pushnull(v); //null iterator
        while (SQ_SUCCEEDED(sq_next(v, -2)))
        {
            HSQOBJECT obj;
            sq_getstackobj(v, -1, &obj);
            objs.push_back(obj);
            sq_pop(v, 2); //pops key and val before the nex iteration
        }
        sq_pop(v, 1); //pops the null iterator

        sq_newarray(v, 0);
        while (!objs.empty())
        {
            auto index = int_rand(0, objs.size() - 1);
            sq_pushobject(v, objs[index]);
            objs.erase(objs.begin() + index);
            sq_arrayappend(v, -2);
        }

        return 1;
    }

    static SQInteger assetExists(HSQUIRRELVM v)
    {
        const SQChar *filename = nullptr;
        if (SQ_FAILED(sq_getstring(v, 2, &filename)))
        {
            return sq_throwerror(v, "failed to get filename");
        }
        sq_pushbool(v, g_pEngine->getSettings().hasEntry(filename) ? SQTrue : SQFalse);
        return 1;
    }

    static SQInteger distance(HSQUIRRELVM v)
    {
        if (sq_gettype(v, 2) == OT_INTEGER)
        {
            SQInteger num1;
            if (SQ_FAILED(sq_getinteger(v, 2, &num1)))
            {
                return sq_throwerror(v, "failed to get num1");
            }
            SQInteger num2;
            if (SQ_FAILED(sq_getinteger(v, 3, &num2)))
            {
                return sq_throwerror(v, "failed to get num2");
            }
            auto d = std::abs(num1 - num2);
            sq_pushinteger(v, d);
            return 1;
        }
        auto obj1 = ScriptEngine::getEntity<Entity>(v, 2);
        if (!obj1)
        {
            return sq_throwerror(v, "failed to get object1 or actor1");
        }
        auto obj2 = ScriptEngine::getEntity<Entity>(v, 3);
        if (!obj2)
        {
            return sq_throwerror(v, "failed to get object2 or actor2");
        }
        auto pos1 = obj1->getPosition();
        auto pos2 = obj1->getPosition();
        auto dx = pos1.x - pos2.x;
        auto dy = pos1.y - pos2.y;
        auto d = std::sqrt(dx * dx + dy * dy);
        sq_pushfloat(v, d);
        return 1;
    }

    static SQInteger incutscene(HSQUIRRELVM v)
    {
        sq_pushinteger(v, g_pEngine->inCutscene() ? 1 : 0);
        return 1;
    }

    static SQInteger indialog(HSQUIRRELVM v)
    {
        sq_pushinteger(v, g_pEngine->getDialogManager().isActive() ? 1 : 0);
        return 1;
    }

    static SQInteger loadArray(HSQUIRRELVM v)
    {
        sq_newarray(v, 0);
        const SQChar *filename;
        if (SQ_FAILED(sq_getstring(v, 2, &filename)))
        {
            return sq_throwerror(v, "Failed to get filename");
        }
        std::vector<char> buffer;
        g_pEngine->getSettings().readEntry(filename, buffer);
        GGPackBufferStream input(buffer);
        std::string line;
        while (getLine(input, line))
        {
            sq_pushstring(v, line.data(), -1);
            sq_arrayappend(v, -2);
        }
        return 1;
    }

    static SQInteger strsplit(HSQUIRRELVM v)
    {
        const SQChar *text;
        if (SQ_FAILED(sq_getstring(v, 2, &text)))
        {
            return sq_throwerror(v, "Failed to get text");
        }

        const SQChar *delimiter;
        if (SQ_FAILED(sq_getstring(v, 3, &delimiter)))
        {
            return sq_throwerror(v, "Failed to get delimiter");
        }

        sq_newarray(v, 0);
        std::string token;
        std::istringstream tokenStream(text);
        while (std::getline(tokenStream, token, delimiter[0]))
        {
            sq_pushstring(v, token.data(), -1);
            sq_arrayappend(v, -2);
        }
        return 1;
    }

    static SQInteger cameraAt(HSQUIRRELVM v)
    {
        SQInteger x, y;
        auto numArgs = sq_gettop(v) - 1;
        if (numArgs == 2)
        {
            if (SQ_FAILED(sq_getinteger(v, 2, &x)))
            {
                return sq_throwerror(v, _SC("failed to get x"));
            }
            if (SQ_FAILED(sq_getinteger(v, 3, &y)))
            {
                return sq_throwerror(v, _SC("failed to get y"));
            }
            g_pEngine->setCameraAt(sf::Vector2f(x - Screen::HalfWidth, y - Screen::HalfHeight));
        }
        else
        {
            auto spot = ScriptEngine::getObject(v, 2);
            auto pos = spot->getPosition();
            g_pEngine->setCameraAt(sf::Vector2f(pos.x - Screen::HalfWidth, pos.y - Screen::HalfHeight));
        }
        return 0;
    }

    static SQInteger cameraBounds(HSQUIRRELVM v)
    {
        std::cerr << "TODO: cameraBounds: not implemented" << std::endl;
        return 0;
    }

    static SQInteger cameraFollow(HSQUIRRELVM v)
    {
        auto *pActor = ScriptEngine::getActor(v, 3);
        g_pEngine->follow(pActor);
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
        auto get = std::bind(&Engine::getCameraAt, g_pEngine);
        auto set = std::bind(&Engine::setCameraAt, g_pEngine, std::placeholders::_1);
        auto method = ScriptEngine::getInterpolationMethod((InterpolationMethod)interpolation);

        auto cameraPanTo = std::make_unique<ChangeProperty<sf::Vector2f>>(get, set, sf::Vector2f(x - Screen::HalfWidth, y - Screen::HalfHeight), sf::seconds(t), method);
        g_pEngine->addFunction(std::move(cameraPanTo));
        return 0;
    }

    static SQInteger cutscene(HSQUIRRELVM v)
    {
        auto numArgs = sq_gettop(v);
        HSQOBJECT env_obj;
        sq_resetobject(&env_obj);
        if (SQ_FAILED(sq_getstackobj(v, 1, &env_obj)))
        {
            return sq_throwerror(v, _SC("Couldn't get environment from stack"));
        }

        // create thread and store it on the stack
        auto thread = sq_newthread(v, 1024);
        HSQOBJECT threadObj;
        sq_resetobject(&threadObj);
        if (SQ_FAILED(sq_getstackobj(v, -1, &threadObj)))
        {
            return sq_throwerror(v, _SC("Couldn't get coroutine thread from stack"));
        }

        // get the cutscene  closure
        HSQOBJECT closureObj;
        sq_resetobject(&closureObj);
        if (SQ_FAILED(sq_getstackobj(v, 2, &closureObj)))
        {
            return sq_throwerror(v, _SC("failed to get cutscene closure"));
        }

        // get the cutscene override closure
        HSQOBJECT closureCutsceneOverrideObj;
        sq_resetobject(&closureCutsceneOverrideObj);
        if (numArgs == 3)
        {
            if (SQ_FAILED(sq_getstackobj(v, 3, &closureCutsceneOverrideObj)))
            {
                return sq_throwerror(v, _SC("failed to get cutscene override closure"));
            }
        }

        g_pEngine->addThread(thread);

        auto scene = std::make_unique<Cutscene>(*g_pEngine, v, threadObj, closureObj, closureCutsceneOverrideObj, env_obj);
        g_pEngine->addFunction(std::move(scene));

        return sq_suspendvm(v);
    }

    static SQInteger cutsceneOverride(HSQUIRRELVM v)
    {
        g_pEngine->cutsceneOverride();
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
        if (sq_gettype(v, 2) == OT_ARRAY)
        {
            HSQOBJECT obj;
            sq_resetobject(&obj);

            auto len = sq_getsize(v, 2);
            auto index = int_rand(0, len);
            sq_push(v, 2);
            sq_pushnull(v); //null iterator
            while (SQ_SUCCEEDED(sq_next(v, -2)))
            {
                sq_getstackobj(v, -1, &obj);
                sq_pop(v, 2); //pops key and val before the nex iteration
                break;
            }

            sq_pop(v, 1); //pops the null iterator

            sq_pushobject(v, obj);
            return 1;
        }
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
        Room *pRoom = nullptr;
        sq_getuserpointer(v, -1, (SQUserPointer *)&pRoom);
        sq_pop(v, 2);

        auto pOldRoom = g_pEngine->getRoom();
        if (pOldRoom && pRoom != pOldRoom)
        {
            // call exit room function
            std::cout << "call exit room function of " << pOldRoom->getId() << std::endl;
            
            sq_pushobject(v, *pOldRoom->getTable());
            sq_pushstring(v, _SC("exit"), -1);
            if (SQ_FAILED(sq_get(v, -2)))
            {
                return sq_throwerror(v, _SC("can't find exit function"));
            }
            sq_remove(v, -2);
            sq_pushobject(v, *pOldRoom->getTable());
            if (SQ_FAILED(sq_call(v, 1, SQFalse, SQTrue)))
            {
                return sq_throwerror(v, _SC("function exit call failed"));
            }
        }

        // set camera in room
        g_pEngine->setRoom(pRoom);

        // call enter room function
        std::cout << "call enter room function of " << pRoom->getId() << std::endl;
        sq_pushobject(v, table);
        sq_pushstring(v, _SC("enter"), -1);
        if (SQ_FAILED(sq_get(v, -2)))
        {
            return sq_throwerror(v, _SC("can't find enter function"));
        }

        SQInteger nparams, nfreevars;
        sq_getclosureinfo(v, -1, &nparams, &nfreevars);
        std::cout << "enter function found with " << nparams << " parameters" << std::endl;

        sq_remove(v, -2);
        sq_pushobject(v, table);
        if (nparams == 2)
        {
            sq_pushnull(v); // push here the door
        }
        if (SQ_FAILED(sq_call(v, nparams, SQTrue, SQTrue)))
        {
            return sq_throwerror(v, _SC("function enter call failed"));
        }
        return 0;
    }

    static SQInteger markProgress(HSQUIRRELVM v)
    {
        std::cerr << "TODO: markProgress: not implemented" << std::endl;
        return 0;
    }

    static SQInteger markStat(HSQUIRRELVM v)
    {
        std::cerr << "TODO: markStat: not implemented" << std::endl;
        return 0;
    }

    static SQInteger screenSize(HSQUIRRELVM v)
    {
        std::cerr << "TODO: screenSize: not implemented" << std::endl;
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

        SQInteger id = 0;
        if (SQ_FAILED(sq_getinteger(v, -1, &id)))
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

        // func
        sq_pushstring(v, _SC("func"), -1);
        if (SQ_FAILED(sq_get(v, -2)))
        {
            sq_pop(v, 2);
            return sq_throwerror(v, _SC("failed to get func"));
        }

        const SQChar *func = nullptr;
        if (SQ_FAILED(sq_getstring(v, -1, &func)))
        {
            sq_pop(v, 2);
            return sq_throwerror(v, _SC("failed to get func"));
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
        verb.func = func;
        verb.image = image;
        verb.text = text;
        verb.key = key;
        g_pEngine->setVerb(actorSlot - 1, verbSlot, verb);
        return 0;
    }

    static SQInteger startDialog(HSQUIRRELVM v)
    {
        const SQChar *dialog;
        if (SQ_FAILED(sq_getstring(v, 2, &dialog)))
        {
            return sq_throwerror(v, _SC("failed to get dialog"));
        }
        const SQChar *node = "start";
        sq_getstring(v, 3, &node);
        g_pEngine->startDialog(dialog, node);
        return 0;
    }

    static SQInteger stopSentence(HSQUIRRELVM v)
    {
        std::cerr << "TODO: stopSentence: not implemented" << std::endl;
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
        sq_pushstring(v, (const SQChar*)text.c_str(), -1);
        return 1;
    }
};

Engine *_GeneralPack::g_pEngine = nullptr;

} // namespace ng
