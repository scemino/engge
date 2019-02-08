#pragma once
#include <time.h>
#include "squirrel.h"
#include "Function.h"
#include "Actor.h"
#include "../_NGUtil.h"

namespace ng
{
class _BreakFunction : public Function
{
  protected:
    Engine &_engine;
    HSQUIRRELVM _vm;
    bool _isElapsed;

  public:
    explicit _BreakFunction(Engine &engine, HSQUIRRELVM vm)
        : _engine(engine), _vm(vm), _isElapsed(false)
    {
    }

    virtual const std::string getName()
    {
        return "_BreakFunction";
    }

    void operator()() override
    {
        if (!_engine.isThreadAlive(_vm))
        {
            std::cerr << getName() << " failed: thread not alive: " << _vm << std::endl;
            return;
        }
        if (_isElapsed || !isElapsed())
            return;
        _isElapsed = true;
        if (sq_getvmstate(_vm) != SQ_VMSTATE_SUSPENDED)
        {
            std::cerr << getName() << " failed: thread not suspended: " << _vm << std::endl;
            return;
        }
        if (SQ_FAILED(sq_wakeupvm(_vm, SQFalse, SQFalse, SQTrue, SQFalse)))
        {
            std::cerr << getName() << ": failed to wakeup: " << _vm << std::endl;
            sqstd_printcallstack(_vm);
            return;
        }
        std::cout << getName() << ": OK to wakeup: " << _vm << std::endl;
    }
};

class _BreakHereFunction : public _BreakFunction
{
  public:
    explicit _BreakHereFunction(Engine &engine, HSQUIRRELVM vm)
        : _BreakFunction(engine, vm)
    {
    }

    const std::string getName() override
    {
        return "_BreakHereFunction";
    }
};

class _BreakWhileAnimatingFunction : public _BreakFunction
{
  private:
    Actor &_actor;
    std::string _name;

  public:
    explicit _BreakWhileAnimatingFunction(Engine &engine, HSQUIRRELVM vm, Actor &actor)
        : _BreakFunction(engine, vm), _actor(actor)
    {
        _name = actor.getCostume().getAnimation()->getName();
    }

    const std::string getName() override
    {
        return "_BreakWhileAnimatingFunction " + _name;
    }

    bool isElapsed() override
    {
        return !_actor.getCostume().getAnimation()->isPlaying();
    }
};

class _BreakWhileWalkingFunction : public _BreakFunction
{
  private:
    Actor &_actor;

  public:
    explicit _BreakWhileWalkingFunction(Engine &engine, HSQUIRRELVM vm, Actor &actor)
        : _BreakFunction(engine, vm), _actor(actor)
    {
    }

    const std::string getName() override
    {
        return "_BreakWhileWalkingFunction";
    }

    bool isElapsed() override
    {
        return !_actor.isWalking();
    }
};

class _BreakWhileTalkingFunction : public _BreakFunction
{
  private:
    Actor &_actor;

  public:
    explicit _BreakWhileTalkingFunction(Engine &engine, HSQUIRRELVM vm, Actor &actor)
        : _BreakFunction(engine, vm), _actor(actor)
    {
    }

    const std::string getName() override
    {
        return "_BreakWhileTalkingFunction";
    }

    bool isElapsed() override
    {
        return !_actor.isTalking();
    }
};

class _BreakWhileSoundFunction : public _BreakFunction
{
  private:
    SoundId &_soundId;

  public:
    _BreakWhileSoundFunction(Engine &engine, HSQUIRRELVM vm, SoundId &soundId)
        : _BreakFunction(engine, vm), _soundId(soundId)
    {
    }

    const std::string getName() override
    {
        return "_BreakWhileSoundFunction";
    }

    bool isElapsed() override
    {
        return !_soundId.isPlaying();
    }
};

class _BreakWhileRunningFunction : public _BreakFunction
{
  private:
    HSQUIRRELVM _thread;

  public:
    _BreakWhileRunningFunction(Engine &engine, HSQUIRRELVM vm, HSQUIRRELVM thread)
        : _BreakFunction(engine, vm), _thread(thread)
    {
    }

    const std::string getName() override
    {
        return "_BreakWhileRunningFunction";
    }

    bool isElapsed() override
    {
        return !_engine.isThreadAlive(_thread);
    }
};

class _BreakWhileDialogFunction : public _BreakFunction
{
  public:
    _BreakWhileDialogFunction(Engine &engine, HSQUIRRELVM vm)
        : _BreakFunction(engine, vm)
    {
    }

    const std::string getName() override
    {
        return "_BreakWhileDialogFunction";
    }

    bool isElapsed() override
    {
        return !_engine.getDialogManager().isActive();
    }
};

class _BreakTimeFunction : public TimeFunction
{
  private:
    HSQUIRRELVM _vm;
    Engine &_engine;

  public:
    _BreakTimeFunction(Engine &engine, HSQUIRRELVM vm, const sf::Time &time)
        : TimeFunction(time), _vm(vm), _engine(engine)
    {
    }

    void operator()() override
    {
        if (isElapsed())
        {
            if (!_engine.isThreadAlive(_vm))
                return;
            if (sq_getvmstate(_vm) != SQ_VMSTATE_SUSPENDED)
                return;
            if (SQ_FAILED(sq_wakeupvm(_vm, SQFalse, SQFalse, SQTrue, SQFalse)))
            {
                std::cerr << "_BreakTimeFunction: failed to wakeup: " << _vm << std::endl;
                sqstd_printcallstack(_vm);
                return;
            }
            // std::cout << "_BreakTimeFunction: OK to wakeup: " << _vm << std::endl;
        }
    }
};

class _SystemPack : public Pack
{
  private:
    static Engine *g_pEngine;

  private:
    void addTo(ScriptEngine &engine) const override
    {
        g_pEngine = &engine.getEngine();
        engine.registerGlobalFunction(activeController, "activeController");
        engine.registerGlobalFunction(breakhere, "breakhere");
        engine.registerGlobalFunction(breakwhileanimating, "breakwhileanimating");
        engine.registerGlobalFunction(breakwhiledialog, "breakwhiledialog");
        engine.registerGlobalFunction(breakwhilesound, "breakwhilesound");
        engine.registerGlobalFunction(breakwhilewalking, "breakwhilewalking");
        engine.registerGlobalFunction(breakwhiletalking, "breakwhiletalking");
        engine.registerGlobalFunction(breakwhilerunning, "breakwhilerunning");
        engine.registerGlobalFunction(stopthread, "stopthread");
        engine.registerGlobalFunction(startthread, "startthread");
        engine.registerGlobalFunction(cutscene, "cutscene");
        engine.registerGlobalFunction(breaktime, "breaktime");
        engine.registerGlobalFunction(getUserPref, "getUserPref");
        engine.registerGlobalFunction(inputOff, "inputOff");
        engine.registerGlobalFunction(inputOn, "inputOn");
        engine.registerGlobalFunction(inputSilentOff, "inputSilentOff");
        engine.registerGlobalFunction(isInputOn, "isInputOn");
        engine.registerGlobalFunction(isString, "is_string");
        engine.registerGlobalFunction(isTable, "is_table");
        engine.registerGlobalFunction(inputVerbs, "inputVerbs");
        engine.registerGlobalFunction(setUserPref, "setUserPref");
        engine.registerGlobalFunction(setAmbientLight, "setAmbientLight");
        engine.registerGlobalFunction(systemTime, "systemTime");
        engine.registerGlobalFunction(threadpauseable, "threadpauseable");
    }

    static SQInteger activeController(HSQUIRRELVM v)
    {
        return 1;
    }

    static SQInteger breakhere(HSQUIRRELVM v)
    {
        auto result = sq_suspendvm(v);
        g_pEngine->addFunction(std::make_unique<_BreakHereFunction>(*g_pEngine, v));
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
        g_pEngine->addFunction(std::make_unique<_BreakWhileAnimatingFunction>(*g_pEngine, v, *pActor));
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
        g_pEngine->addFunction(std::make_unique<_BreakWhileSoundFunction>(*g_pEngine, v, *pSound));
        return result;
    }

    static SQInteger breakwhiledialog(HSQUIRRELVM v)
    {
        auto result = sq_suspendvm(v);
        g_pEngine->addFunction(std::make_unique<_BreakWhileDialogFunction>(*g_pEngine, v));
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
        g_pEngine->addFunction(std::make_unique<_BreakWhileWalkingFunction>(*g_pEngine, v, *pActor));
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
        g_pEngine->addFunction(std::make_unique<_BreakWhileTalkingFunction>(*g_pEngine, v, *pActor));
        return result;
    }

    static SQInteger breakwhilerunning(HSQUIRRELVM v)
    {
        HSQUIRRELVM thread;
        if (SQ_FAILED(sq_getthread(v, 2, &thread)))
        {
            return sq_throwerror(v, _SC("Couldn't get coroutine thread from stack"));
        }
        auto result = sq_suspendvm(v);
        g_pEngine->addFunction(std::make_unique<_BreakWhileRunningFunction>(*g_pEngine, v, thread));
        return result;
    }

    static SQInteger stopthread(HSQUIRRELVM v)
    {
        HSQOBJECT thread_obj;
        if (SQ_FAILED(sq_getstackobj(v, 2, &thread_obj)))
        {
            return sq_throwerror(v, _SC("Couldn't get coroutine thread from stack"));
        }
        std::cout << "stopthread " << thread_obj._unVal.pThread << std::endl;
        g_pEngine->stopThread(thread_obj._unVal.pThread);
        sq_release(v, &thread_obj);

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

        // create a table for a thread
        sq_addref(v, &thread_obj);
        sq_pushobject(v, thread_obj);

        std::cout << "start thread: " << thread_obj._unVal.pThread << std::endl;
        g_pEngine->addThread(thread_obj._unVal.pThread);

        return 1;
    }

    static SQInteger cutscene(HSQUIRRELVM v)
    {
        auto inputActive = g_pEngine->getInputActive();
        g_pEngine->setInputActive(false);
        auto currentActor = g_pEngine->getCurrentActor();

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

        if (SQ_FAILED(sq_call(thread, 1, SQFalse, SQTrue)))
        {
            sq_throwerror(v, _SC("call failed"));
            sq_pop(thread, 1); // pop the compiled closure
            return SQ_ERROR;
        }

        // create a table for a thread
        sq_addref(v, &thread_obj);
        sq_pushobject(v, thread_obj);

        std::cout << "start cutscene: " << thread_obj._unVal.pThread << std::endl;
        g_pEngine->addThread(thread_obj._unVal.pThread);

        g_pEngine->setInputActive(inputActive);
        if (currentActor)
        {
            g_pEngine->follow(currentActor);
        }

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
        g_pEngine->addFunction(std::make_unique<_BreakTimeFunction>(*g_pEngine, v, sf::seconds(time)));
        return result;
    }

    static SQInteger getUserPref(HSQUIRRELVM v)
    {
        const SQChar *key;
        if (SQ_FAILED(sq_getstring(v, 2, &key)))
        {
            return sq_throwerror(v, _SC("failed to get key"));
        }
        auto numArgs = sq_gettop(v) - 1;
        std::any defaultValue;
        if (numArgs > 1)
        {
            auto type = sq_gettype(v, 3);
            if (type == SQObjectType::OT_STRING)
            {
                const SQChar *str = nullptr;
                sq_getstring(v, 3, &str);
                std::string strValue = str;
                defaultValue = strValue;
            }
            else if (type == SQObjectType::OT_INTEGER)
            {
                SQInteger integer;
                sq_getinteger(v, 3, &integer);
                defaultValue = integer;
            }
            else if (type == SQObjectType::OT_BOOL)
            {
                SQBool b;
                sq_getbool(v, 3, &b);
                defaultValue = b;
            }
            else if (type == SQObjectType::OT_FLOAT)
            {
                SQFloat fl;
                sq_getfloat(v, 3, &fl);
                defaultValue = fl;
            }
        }

        auto value = g_pEngine->getPreferences().getUserPreference(key, defaultValue);
        const auto &valueType = value.type();
        if (valueType == typeid(std::string))
        {
            sq_pushstring(v, std::any_cast<std::string>(value).data(), -1);
        }
        else if (valueType == typeid(int))
        {
            sq_pushinteger(v, std::any_cast<int>(value));
        }
        else if (valueType == typeid(float))
        {
            sq_pushfloat(v, std::any_cast<float>(value));
        }
        else if (valueType == typeid(bool))
        {
            sq_pushbool(v, std::any_cast<bool>(value));
        }
        else
        {
            sq_pushnull(v);
        }

        return 1;
    }

    static SQInteger setAmbientLight(HSQUIRRELVM v)
    {
        SQInteger c = 0;
        if (SQ_FAILED(sq_getinteger(v, 2, &c)))
        {
            return sq_throwerror(v, _SC("failed to get color"));
        }
        auto color = _fromRgb(c);
        g_pEngine->getRoom().setAmbientLight(color);
        return 0;
    }

    static SQInteger setUserPref(HSQUIRRELVM v)
    {
        const SQChar *key;
        if (SQ_FAILED(sq_getstring(v, 2, &key)))
        {
            return sq_throwerror(v, _SC("failed to get key"));
        }
        std::any value;
        auto type = sq_gettype(v, 3);
        if (type == SQObjectType::OT_STRING)
        {
            const SQChar *str = nullptr;
            sq_getstring(v, 3, &str);
            std::string strValue = str;
            value = strValue;
            g_pEngine->getPreferences().setUserPreference(key, value);
            return 0;
        }
        if (type == SQObjectType::OT_INTEGER)
        {
            SQInteger integer;
            sq_getinteger(v, 3, &integer);
            value = integer;
            g_pEngine->getPreferences().setUserPreference(key, value);
            return 0;
        }
        if (type == SQObjectType::OT_BOOL)
        {
            SQBool b;
            sq_getbool(v, 3, &b);
            value = b;
            g_pEngine->getPreferences().setUserPreference(key, value);
            return 0;
        }
        if (type == SQObjectType::OT_FLOAT)
        {
            SQFloat fl;
            sq_getfloat(v, 3, &fl);
            value = fl;
            g_pEngine->getPreferences().setUserPreference(key, value);
            return 0;
        }

        g_pEngine->getPreferences().removeUserPreference(key);

        return 0;
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

    static SQInteger isTable(HSQUIRRELVM v)
    {
        HSQOBJECT object;
        sq_resetobject(&object);
        if (SQ_FAILED(sq_getstackobj(v, 2, &object)))
        {
            sq_push(v, SQFalse);
            return 1;
        }

        sq_push(v, sq_istable(object) ? SQTrue : SQFalse);
        return 1;
    }

    static SQInteger isString(HSQUIRRELVM v)
    {
        HSQOBJECT object;
        sq_resetobject(&object);
        if (SQ_FAILED(sq_getstackobj(v, 2, &object)))
        {
            sq_push(v, SQFalse);
            return 1;
        }

        sq_push(v, sq_isstring(object) ? SQTrue : SQFalse);
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

Engine *_SystemPack::g_pEngine = nullptr;

} // namespace ng
