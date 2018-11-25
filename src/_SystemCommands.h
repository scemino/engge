#pragma once
#include <time.h>
#include <squirrel3/squirrel.h>
#include "Function.h"
#include "GGActor.h"

namespace gg
{
class _BreakHereFunction : public Function
{
  private:
    HSQUIRRELVM _vm;

  public:
    explicit _BreakHereFunction(HSQUIRRELVM vm)
        : _vm(vm)
    {
    }

    void operator()() override
    {
        if (sq_getvmstate(_vm) != SQ_VMSTATE_SUSPENDED)
            return;
        if (SQ_FAILED(sq_wakeupvm(_vm, SQFalse, SQFalse, SQTrue, SQFalse)))
        {
            std::cerr << "_BreakHereFunction: failed to wakeup: " << _vm << std::endl;
            sqstd_printcallstack(_vm);
        }
    }
};

class _BreakWhileAnimatingFunction : public Function
{
  private:
    HSQUIRRELVM _vm;
    GGActor &_actor;

  public:
    explicit _BreakWhileAnimatingFunction(HSQUIRRELVM vm, GGActor &actor)
        : _vm(vm), _actor(actor)
    {
    }

    bool isElapsed() override
    {
        bool isPlaying = _actor.getCostume().getAnimation()->isPlaying();
        return !isPlaying;
    }

    void operator()() override
    {
        if (!isElapsed())
            return;

        if (sq_getvmstate(_vm) != SQ_VMSTATE_SUSPENDED)
            return;
        if (SQ_FAILED(sq_wakeupvm(_vm, SQFalse, SQFalse, SQTrue, SQFalse)))
        {
            std::cerr << "_BreakWhileAnimatingFunction: failed to wakeup: " << _vm << std::endl;
            sqstd_printcallstack(_vm);
        }
    }
};

class _BreakWhileWalkingFunction : public Function
{
  private:
    HSQUIRRELVM _vm;
    GGActor &_actor;

  public:
    explicit _BreakWhileWalkingFunction(HSQUIRRELVM vm, GGActor &actor)
        : _vm(vm), _actor(actor)
    {
    }

    bool isElapsed() override
    {
        auto name = _actor.getCostume().getAnimationName();
        bool isElapsed = name != "walk";
        return isElapsed;
    }

    void operator()() override
    {
        if (!isElapsed())
            return;

        if (sq_getvmstate(_vm) != SQ_VMSTATE_SUSPENDED)
            return;
        if (SQ_FAILED(sq_wakeupvm(_vm, SQFalse, SQFalse, SQTrue, SQFalse)))
        {
            std::cerr << "_BreakWhileWalkingFunction: failed to wakeup: " << _vm << std::endl;
            sqstd_printcallstack(_vm);
        }
    }
};

class _BreakWhileTalkingFunction : public Function
{
  private:
    HSQUIRRELVM _vm;
    GGActor &_actor;

  public:
    explicit _BreakWhileTalkingFunction(HSQUIRRELVM vm, GGActor &actor)
        : _vm(vm), _actor(actor)
    {
    }

    bool isElapsed() override
    {
        return !_actor.isTalking();
    }

    void operator()() override
    {
        if (!isElapsed())
            return;

        if (sq_getvmstate(_vm) != SQ_VMSTATE_SUSPENDED)
            return;
        if (SQ_FAILED(sq_wakeupvm(_vm, SQFalse, SQFalse, SQTrue, SQFalse)))
        {
            std::cerr << "_BreakWhileTalkingFunction: failed to wakeup: " << _vm << std::endl;
            sqstd_printcallstack(_vm);
        }
    }
};

class _BreakWhileSoundFunction : public Function
{
  private:
    HSQUIRRELVM _vm;
    SoundId &_soundId;

  public:
    explicit _BreakWhileSoundFunction(HSQUIRRELVM vm, SoundId &soundId)
        : _vm(vm), _soundId(soundId)
    {
    }

    bool isElapsed() override
    {
        return !_soundId.isPlaying();
    }

    void operator()() override
    {
        if (!isElapsed())
            return;

        if (sq_getvmstate(_vm) != SQ_VMSTATE_SUSPENDED)
            return;
        if (SQ_FAILED(sq_wakeupvm(_vm, SQFalse, SQFalse, SQTrue, SQFalse)))
        {
            std::cerr << "_BreakWhileSoundFunction: failed to wakeup: " << _vm << std::endl;
            sqstd_printcallstack(_vm);
        }
    }
};

class _BreakTimeFunction : public TimeFunction
{
  private:
    HSQUIRRELVM _vm;

  public:
    _BreakTimeFunction(HSQUIRRELVM vm, const sf::Time &time)
        : TimeFunction(time), _vm(vm)
    {
    }

    void operator()() override
    {
        if (isElapsed())
        {
            if (sq_getvmstate(_vm) != SQ_VMSTATE_SUSPENDED)
                return;
            if (SQ_FAILED(sq_wakeupvm(_vm, SQFalse, SQFalse, SQTrue, SQFalse)))
            {
                std::cerr << "_BreakTimeFunction: failed to wakeup: " << _vm << std::endl;
                sqstd_printcallstack(_vm);
            }
        }
    }
};

class _SystemPack : public Pack
{
  private:
    static GGEngine *g_pEngine;

  private:
    void addTo(ScriptEngine &engine) const override
    {
        g_pEngine = &engine.getEngine();
        engine.registerGlobalFunction(breakhere, "breakhere");
        engine.registerGlobalFunction(breakwhileanimating, "breakwhileanimating");
        engine.registerGlobalFunction(breakwhilesound, "breakwhilesound");
        engine.registerGlobalFunction(breakwhilewalking, "breakwhilewalking");
        engine.registerGlobalFunction(breakwhiletalking, "breakwhiletalking");
        engine.registerGlobalFunction(stopthread, "stopthread");
        engine.registerGlobalFunction(startthread, "startthread");
        engine.registerGlobalFunction(breaktime, "breaktime");
        engine.registerGlobalFunction(getUserPref, "getUserPref");
        engine.registerGlobalFunction(inputOff, "inputOff");
        engine.registerGlobalFunction(inputOn, "inputOn");
        engine.registerGlobalFunction(isInputOn, "isInputOn");
        engine.registerGlobalFunction(inputVerbs, "inputVerbs");
        engine.registerGlobalFunction(systemTime, "systemTime");
        engine.registerGlobalFunction(threadpauseable, "threadpauseable");
    }

    static SQInteger breakhere(HSQUIRRELVM v)
    {
        auto result = sq_suspendvm(v);
        g_pEngine->addFunction(std::make_unique<_BreakHereFunction>(v));
        return result;
    }

    static SQInteger breakwhileanimating(HSQUIRRELVM v)
    {
        auto *pActor = ScriptEngine::getActor(v, 2);
        if (!pActor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        auto result = sq_suspendvm(v);
        g_pEngine->addFunction(std::make_unique<_BreakWhileAnimatingFunction>(v, *pActor));
        return result;
    }

    static SQInteger breakwhilesound(HSQUIRRELVM v)
    {
        SoundId *pSound = nullptr;
        if (SQ_FAILED(sq_getuserpointer(v, 2, (SQUserPointer *)&pSound)))
        {
            return sq_throwerror(v, _SC("failed to get sound"));
        }
        auto result = sq_suspendvm(v);
        g_pEngine->addFunction(std::make_unique<_BreakWhileSoundFunction>(v, *pSound));
        return result;
    }

    static SQInteger breakwhilewalking(HSQUIRRELVM v)
    {
        auto *pActor = ScriptEngine::getActor(v, 2);
        if (!pActor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        auto result = sq_suspendvm(v);
        g_pEngine->addFunction(std::make_unique<_BreakWhileWalkingFunction>(v, *pActor));
        return result;
    }

    static SQInteger breakwhiletalking(HSQUIRRELVM v)
    {
        auto *pActor = ScriptEngine::getActor(v, 2);
        if (!pActor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        auto result = sq_suspendvm(v);
        g_pEngine->addFunction(std::make_unique<_BreakWhileTalkingFunction>(v, *pActor));
        return result;
    }

    static SQInteger stopthread(HSQUIRRELVM v)
    {
        HSQOBJECT thread_obj;
        sq_resetobject(&thread_obj);
        if (SQ_FAILED(sq_getstackobj(v, 2, &thread_obj)))
        {
            return sq_throwerror(v, _SC("Couldn't get coroutine thread from stack"));
        }
        sq_release(v, &thread_obj);
        std::cout << "stopthread " << std::endl;
        // sq_suspendvm(thread_obj._unVal.pThread);
        return 0;
    }

    static SQInteger startthread(HSQUIRRELVM v)
    {
        SQInteger size = sq_gettop(v);

        HSQOBJECT env_obj;
        sq_resetobject(&env_obj);
        if (SQ_FAILED(sq_getstackobj(v, 1, &env_obj)))
        {
            return sq_throwerror(v, _SC("Couldn't get environment from stack"));
        }

        // create thread and store it on the stack
        auto thread = sq_newthread(v, 1024);
        HSQOBJECT thread_obj;
        sq_resetobject(&thread_obj);
        if (SQ_FAILED(sq_getstackobj(v, -1, &thread_obj)))
        {
            return sq_throwerror(v, _SC("Couldn't get coroutine thread from stack"));
        }

        std::vector<HSQOBJECT> args;
        for (auto i = 0; i < size - 2; i++)
        {
            HSQOBJECT arg;
            sq_resetobject(&arg);
            if (SQ_FAILED(sq_getstackobj(v, 3 + i, &arg)))
            {
                return sq_throwerror(v, _SC("Couldn't get coroutine args from stack"));
            }
            args.push_back(arg);
        }

        // get the closure
        HSQOBJECT closureObj;
        sq_resetobject(&closureObj);
        if (SQ_FAILED(sq_getstackobj(v, 2, &closureObj)))
        {
            return sq_throwerror(v, _SC("Couldn't get coroutine thread from stack"));
        }

        // call the closure in the thread
        sq_pushobject(thread, closureObj);
        sq_pushobject(thread, env_obj);
        for (auto arg : args)
        {
            sq_pushobject(thread, arg);
        }
        if (SQ_FAILED(sq_call(thread, 1 + args.size(), SQFalse, SQTrue)))
        {
            sq_throwerror(v, _SC("call failed"));
            sq_pop(thread, 1); // pop the compiled closure
            return SQ_ERROR;
        }

        sq_addref(v, &thread_obj);
        sq_pushobject(v, thread_obj);

        return 1;
    }

    static SQInteger breaktime(HSQUIRRELVM v)
    {
        SQFloat time = 0;
        if (SQ_FAILED(sq_getfloat(v, 2, &time)))
        {
            return sq_throwerror(v, _SC("failed to get time"));
        }
        auto result = sq_suspendvm(v);
        g_pEngine->addFunction(std::make_unique<_BreakTimeFunction>(v, sf::seconds(time)));
        return result;
    }

    static SQInteger getUserPref(HSQUIRRELVM v)
    {
        const SQChar *key;
        if (SQ_FAILED(sq_getstring(v, 2, &key)))
        {
            return sq_throwerror(v, _SC("failed to get key"));
        }
        const SQChar *defaultValue;
        if (SQ_FAILED(sq_getstring(v, 3, &defaultValue)))
        {
            return sq_throwerror(v, _SC("failed to get defaultValue"));
        }
        // TODO: get here the value from the preferences file
        sq_pushstring(v, defaultValue, -1);
        return 1;
    }

    static SQInteger inputOff(HSQUIRRELVM v)
    {
        g_pEngine->setInputActive(false);
        return 0;
    }

    static SQInteger inputOn(HSQUIRRELVM v)
    {
        g_pEngine->setInputActive(true);
        return 0;
    }

    static SQInteger inputSilentOff(HSQUIRRELVM v)
    {
        g_pEngine->inputSilentOff();
        return 0;
    }

    static SQInteger isInputOn(HSQUIRRELVM v)
    {
        bool isActive = g_pEngine->getInputActive();
        sq_push(v, isActive ? SQTrue : SQFalse);
        return 1;
    }

    static SQInteger inputVerbs(HSQUIRRELVM v)
    {
        SQInteger on;
        if (SQ_FAILED(sq_getinteger(v, 2, &on)))
        {
            return sq_throwerror(v, _SC("failed to get isActive"));
        }
        // TODO: g_pEngine->setInputVerbs(on);
        return 1;
    }

    static SQInteger systemTime(HSQUIRRELVM v)
    {
        time_t t;
        time(&t);
        sq_pushinteger(v, t);
        return 1;
    }

    static SQInteger threadpauseable(HSQUIRRELVM v)
    {
        HSQOBJECT thread;
        sq_resetobject(&thread);
        if (SQ_FAILED(sq_getstackobj(v, 2, &thread)))
        {
            return sq_throwerror(v, _SC("failed to get thread"));
        }
        SQBool pauseable;
        if (SQ_FAILED(sq_getbool(v, 3, &pauseable)))
        {
            pauseable = true;
        }
        // TODO: set thread pauseable
        return 0;
    }
};

GGEngine *_SystemPack::g_pEngine = nullptr;

} // namespace gg
