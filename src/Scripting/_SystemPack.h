#pragma once
#include <time.h>
#include "squirrel.h"
#include "Actor.h"
#include "Camera.h"
#include "Animation.h"
#include "Dialog/DialogManager.h"
#include "Engine.h"
#include "Function.h"
#include "Logger.h"
#include "Preferences.h"
#include "Room.h"
#include "SoundId.h"
#include "SoundManager.h"
#include "Thread.h"
#include "../_Util.h"

namespace ng
{
class _StopThread : public Function
{
private:
    Engine &_engine;
    HSQOBJECT _threadObj;
    bool _done;

public:
    _StopThread(Engine &engine, HSQOBJECT threadObj)
        : _engine(engine), _threadObj(threadObj), _done(false)
    {
    }

    bool isElapsed() override
    {
        return _done;
    }

    void operator()(const sf::Time &elapsed) override
    {
        if (_done)
            return;
        _engine.stopThread(_threadObj._unVal.pThread);
        _done = true;
    }
};

class _WakeupThread : public Function
{
private:
    Engine& _engine;
    HSQUIRRELVM _vm;
    bool _done;

public:
    explicit _WakeupThread(Engine& engine, HSQUIRRELVM vm)
        : _engine(engine), _vm(vm), _done(false)
    {
    }

    bool isElapsed() override
    {
        return _done;
    }

    void operator()(const sf::Time &elapsed) override
    {
        if (_done)
            return;

        _done = true;
        if (!_engine.isThreadAlive(_vm) || sq_getvmstate(_vm) == SQ_VMSTATE_IDLE)
        {
            return;
        }

        if (SQ_FAILED(sq_wakeupvm(_vm, SQFalse, SQFalse, SQTrue, SQFalse)))
        {
            error("_WakeupThread: failed to wakeup: {}", (long)_vm);
            sqstd_printcallstack(_vm);
            return;
        }
    }
};

class _BreakFunction : public Function
{
protected:
    Engine &_engine;
    HSQUIRRELVM _vm;
    bool _done;

public:
    explicit _BreakFunction(Engine &engine, HSQUIRRELVM vm)
        : _engine(engine), _vm(vm), _done(false)
    {
    }

    virtual const std::string getName()
    {
        return "_BreakFunction";
    }

    void operator()(const sf::Time &elapsed) override
    {
        if (_done)
            return;

        if (!isElapsed())
            return;

        _done = true;
        auto wakeupThread = std::make_unique<_WakeupThread>(_engine, _vm);
        _engine.addFunction(std::move(wakeupThread));
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
    std::string _name;
    Actor &_actor;
    CostumeAnimation *_pAnimation;

public:
    _BreakWhileAnimatingFunction(Engine &engine, HSQUIRRELVM vm, Actor &actor)
        : _BreakFunction(engine, vm), _actor(actor), _pAnimation(actor.getCostume().getAnimation())
    {
        if (_pAnimation)
        {
            _name = _pAnimation->getName();
        }
    }

    const std::string getName() override
    {
        return "_BreakWhileAnimatingFunction " + _name;
    }

    bool isElapsed() override
    {
        auto pAnim = _actor.getCostume().getAnimation();
        return pAnim != _pAnimation || !_pAnimation->isPlaying();
    }
};

class _BreakWhileAnimatingObjectFunction : public _BreakFunction
{
private:
    std::optional<Animation> &_animation;

public:
    _BreakWhileAnimatingObjectFunction(Engine &engine, HSQUIRRELVM vm, Object &object)
        : _BreakFunction(engine, vm), _animation(object.getAnimation())
    {
    }

    const std::string getName() override
    {
        return "_BreakWhileAnimatingObjectFunction";
    }

    bool isElapsed() override
    {
        return !_animation.has_value() || !_animation.value().isPlaying();
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

class _BreakWhileAnyActorTalkingFunction : public _BreakFunction
{
public:
    explicit _BreakWhileAnyActorTalkingFunction(Engine &engine, HSQUIRRELVM vm)
        : _BreakFunction(engine, vm)
    {
    }

    const std::string getName() override
    {
        return "_BreakWhileAnyActorTalkingFunction";
    }

    bool isElapsed() override
    {
        for(auto&& actor : _engine.getActors())
        {
            if(actor->isTalking())
                return false;
        }
        return true;
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
        auto pSoundId = _engine.getSoundManager().getSoundFromId(&_soundId);
        return !pSoundId || !pSoundId->isPlaying();
    }
};

class _BreakWhileRunningFunction : public Function
{
private:
    Engine &_engine;
    HSQUIRRELVM _vm;
    HSQUIRRELVM _thread;
    bool _done;

public:
    _BreakWhileRunningFunction(Engine &engine, HSQUIRRELVM vm, HSQUIRRELVM thread)
        : _engine(engine), _vm(vm), _thread(thread), _done(false)
    {
    }

    void operator()(const sf::Time &elapsed) override
    {
        if (_done)
            return;

        if (!_engine.isThreadAlive(_thread) || sq_getvmstate(_thread) == SQ_VMSTATE_IDLE)
        {
            _engine.stopThread(_thread);
            auto wakeupThread = std::make_unique<_WakeupThread>(_engine, _vm);
            _engine.addFunction(std::move(wakeupThread));
            _done = true;
        }
    }

    bool isElapsed() override
    {
        return _done;
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
        return _engine.getDialogManager().getState() == DialogManagerState::None;
    }
};

class _BreakWhileCutsceneFunction : public _BreakFunction
{
public:
    _BreakWhileCutsceneFunction(Engine &engine, HSQUIRRELVM vm)
        : _BreakFunction(engine, vm)
    {
    }

    const std::string getName() override
    {
        return "_BreakWhileCutsceneFunction";
    }

    bool isElapsed() override
    {
        return !_engine.inCutscene();
    }
};

class _BreakWhileCameraFunction : public _BreakFunction
{
public:
    _BreakWhileCameraFunction(Engine &engine, HSQUIRRELVM vm)
        : _BreakFunction(engine, vm)
    {
    }

    const std::string getName() override
    {
        return "_BreakWhileCameraFunction";
    }

    bool isElapsed() override
    {
        return !_engine.getCamera().isMoving();
    }
};

class _BreakWhileInputOffFunction : public _BreakFunction
{
public:
    _BreakWhileInputOffFunction(Engine &engine, HSQUIRRELVM vm)
        : _BreakFunction(engine, vm)
    {
    }

    const std::string getName() override
    {
        return "_BreakWhileInputOffFunction";
    }

    bool isElapsed() override
    {
        return _engine.getInputActive();
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

    void operator()(const sf::Time &elapsed) override
    {
        if (_done)
            return;
        TimeFunction::operator()(elapsed);
        if (isElapsed())
        {
            _done = true;
            if (!_engine.isThreadAlive(_vm))
                return;
            if (sq_getvmstate(_vm) != SQ_VMSTATE_SUSPENDED)
                return;

            auto wakeupThread = std::make_unique<_WakeupThread>(_engine, _vm);
            _engine.addFunction(std::move(wakeupThread));
        }
    }
};

class _CallbackFunction : public TimeFunction
{
private:
    HSQUIRRELVM _v;
    bool _done;
    HSQOBJECT _method;

public:
    _CallbackFunction(HSQUIRRELVM v, sf::Time duration, HSQOBJECT method)
        : TimeFunction(duration), _v(v), _done(false), _method(method)
    {
        sq_addref(_v, &_method);
    }

private:
    void onElapsed() override
    {
        if (_done)
            return;
        _done = true;

        sq_pushobject(_v, _method);
        sq_pushroottable(_v);
        if (SQ_FAILED(sq_call(_v, 1, SQFalse, SQTrue)))
        {
            error("failed to call callback");
        }
        sq_release(_v, &_method);
    }
};

class _SystemPack : public Pack
{
private:
    static Engine *g_pEngine;
    static ScriptEngine *_pScriptEngine;

private:
    void addTo(ScriptEngine &engine) const override
    {
        g_pEngine = &engine.getEngine();
        _pScriptEngine = &engine;
        engine.registerGlobalFunction(activeController, "activeController");
        engine.registerGlobalFunction(addCallback, "addCallback");
        engine.registerGlobalFunction(addFolder, "addFolder");
        engine.registerGlobalFunction(breakhere, "breakhere");
        engine.registerGlobalFunction(breaktime, "breaktime");
        engine.registerGlobalFunction(breakwhileanimating, "breakwhileanimating");
        engine.registerGlobalFunction(breakwhilecamera, "breakwhilecamera");
        engine.registerGlobalFunction(breakwhilecutscene, "breakwhilecutscene");
        engine.registerGlobalFunction(breakwhiledialog, "breakwhiledialog");
        engine.registerGlobalFunction(breakwhileinputoff, "breakwhileinputoff");
        engine.registerGlobalFunction(breakwhilesound, "breakwhilesound");
        engine.registerGlobalFunction(breakwhilerunning, "breakwhilerunning");
        engine.registerGlobalFunction(breakwhiletalking, "breakwhiletalking");
        engine.registerGlobalFunction(breakwhilewalking, "breakwhilewalking");
        engine.registerGlobalFunction(chr, "chr");
        engine.registerGlobalFunction(cursorPosX, "cursorPosX");
        engine.registerGlobalFunction(cursorPosY, "cursorPosY");
        engine.registerGlobalFunction(exCommand, "exCommand");
        engine.registerGlobalFunction(gameTime, "gameTime");
        engine.registerGlobalFunction(getPrivatePref, "getPrivatePref");
        engine.registerGlobalFunction(getUserPref, "getUserPref");
        engine.registerGlobalFunction(include, "include");
        engine.registerGlobalFunction(inputHUD, "inputHUD");
        engine.registerGlobalFunction(inputOff, "inputOff");
        engine.registerGlobalFunction(inputOn, "inputOn");
        engine.registerGlobalFunction(inputSilentOff, "inputSilentOff");
        engine.registerGlobalFunction(inputState, "inputState");
        engine.registerGlobalFunction(isInputOn, "isInputOn");
        engine.registerGlobalFunction(is_string, "is_string");
        engine.registerGlobalFunction(is_table, "is_table");
        engine.registerGlobalFunction(ord, "ord");
        engine.registerGlobalFunction(inputController, "inputController");
        engine.registerGlobalFunction(inputVerbs, "inputVerbs");
        engine.registerGlobalFunction(logEvent, "logEvent");
        engine.registerGlobalFunction(logInfo, "logInfo");
        engine.registerGlobalFunction(logWarning, "logWarning");
        engine.registerGlobalFunction(microTime, "microTime");
        engine.registerGlobalFunction(moveCursorTo, "moveCursorTo");
        engine.registerGlobalFunction(pushSentence, "pushSentence");
        engine.registerGlobalFunction(removeCallback, "removeCallback");
        engine.registerGlobalFunction(setAmbientLight, "setAmbientLight");
        engine.registerGlobalFunction(setPrivatePref, "setPrivatePref");
        engine.registerGlobalFunction(setUserPref, "setUserPref");
        engine.registerGlobalFunction(startglobalthread, "startglobalthread");
        engine.registerGlobalFunction(startthread, "startthread");
        engine.registerGlobalFunction(stopthread, "stopthread");
        engine.registerGlobalFunction(threadid, "threadid");
        engine.registerGlobalFunction(threadpauseable, "threadpauseable");
    }

    static SQInteger activeController(HSQUIRRELVM v)
    {
        error("TODO: activeController: not implemented");
        // harcode mouse
        sq_pushinteger(v, 1);
        return 1;
    }

    static SQInteger addCallback(HSQUIRRELVM v)
    {
        SQFloat duration;
        if (SQ_FAILED(sq_getfloat(v, 2, &duration)))
        {
            return sq_throwerror(v, _SC("failed to get duration"));
        }
        HSQOBJECT method;
        sq_resetobject(&method);
        if (SQ_FAILED(sq_getstackobj(v, 3, &method)) || !sq_isclosure(method))
        {
            return sq_throwerror(v, _SC("failed to get method"));
        }
        auto callback = std::make_unique<_CallbackFunction>(v, sf::seconds(duration), method);
        g_pEngine->addFunction(std::move(callback));
        return 0;
    }

    static SQInteger addFolder(HSQUIRRELVM v)
    {
        // do nothing
        return 0;
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
        if (pActor)
        {
            auto result = sq_suspendvm(v);
            g_pEngine->addFunction(std::make_unique<_BreakWhileAnimatingFunction>(*g_pEngine, v, *pActor));
            return result;
        }

        auto *pObj = ScriptEngine::getObject(v, 2);
        if (pObj)
        {
            auto result = sq_suspendvm(v);
            g_pEngine->addFunction(std::make_unique<_BreakWhileAnimatingObjectFunction>(*g_pEngine, v, *pObj));
            return result;
        }
        return sq_throwerror(v, _SC("failed to get actor or object"));
    }

    static SQInteger breakwhilecamera(HSQUIRRELVM v)
    {
        auto result = sq_suspendvm(v);
        g_pEngine->addFunction(std::make_unique<_BreakWhileCameraFunction>(*g_pEngine, v));
        return result;
    }

    static SQInteger breakwhilecutscene(HSQUIRRELVM v)
    {
        auto result = sq_suspendvm(v);
        g_pEngine->addFunction(std::make_unique<_BreakWhileCutsceneFunction>(*g_pEngine, v));
        return result;
    }

    static SQInteger breakwhileinputoff(HSQUIRRELVM v)
    {
        auto result = sq_suspendvm(v);
        g_pEngine->addFunction(std::make_unique<_BreakWhileInputOffFunction>(*g_pEngine, v));
        return result;
    }

    static SoundId *_getSound(HSQUIRRELVM v, SQInteger index)
    {
        SoundId *pSound = nullptr;
        if (SQ_FAILED(sq_getuserpointer(v, index, (SQUserPointer *)&pSound)))
        {
            SQInteger i = 0;
            if (SQ_FAILED(sq_getinteger(v, index, &i)))
            {
                return nullptr;
            }
            return g_pEngine->getSoundManager().getSound(i);
        }
        return pSound;
    }

    static SQInteger breakwhilesound(HSQUIRRELVM v)
    {
        SoundId *pSound = _getSound(v, 2);
        if (!pSound)
        {
            return 0;
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
        if(sq_gettop(v) == 2)
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
        auto result = sq_suspendvm(v);
        g_pEngine->addFunction(std::make_unique<_BreakWhileAnyActorTalkingFunction>(*g_pEngine, v));
        return result;
    }

    static SQInteger breakwhilerunning(HSQUIRRELVM v)
    {
        if (sq_gettype(v, 2) == OT_THREAD)
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
        return breakwhilesound(v);
    }

    static SQInteger chr(HSQUIRRELVM v)
    {
        SQInteger number;
        if (SQ_FAILED(sq_getinteger(v, 2, &number)))
        {
            return sq_throwerror(v, "Failed to get number");
        }
        auto character = (char)number;
        char s[]{character,'\0'};
        sq_pushstring(v, s, -1);
        return 1;
    }

    static SQInteger cursorPosX(HSQUIRRELVM v)
    {
        auto pos = g_pEngine->getMousePositionInRoom();
        sq_pushinteger(v, static_cast<SQInteger>(pos.x));
        return 1;
    }

    static SQInteger cursorPosY(HSQUIRRELVM v)
    {
        auto pos = g_pEngine->getMousePositionInRoom();
        sq_pushinteger(v, static_cast<SQInteger>(pos.y));
        return 1;
    }

    static SQInteger exCommand(HSQUIRRELVM v)
    {
        error("TODO: exCommand: not implemented");
        return 0;
    }

    static SQInteger gameTime(HSQUIRRELVM v)
    {
        sq_pushfloat(v, g_pEngine->getTime().asSeconds());
        return 1;
    }

    static SQInteger logEvent(HSQUIRRELVM v)
    {
        auto numArgs = sq_gettop(v);
        const SQChar* event = nullptr;
        if (SQ_SUCCEEDED(sq_getstring(v, 2, &event)))
        {
            info(event);
        }
        if(numArgs == 3)
        {
            if (SQ_SUCCEEDED(sq_getstring(v, 3, &event)))
            {
                info(event);
            }
        }
        return 0;
    }

    static SQInteger logInfo(HSQUIRRELVM v)
    {
        const SQChar* msg = nullptr;
        if (SQ_SUCCEEDED(sq_getstring(v, 2, &msg)))
        {
            info(msg);
        }
        return 0;
    }

    static SQInteger logWarning(HSQUIRRELVM v)
    {
        const SQChar* msg = nullptr;
        if (SQ_SUCCEEDED(sq_getstring(v, 2, &msg)))
        {
            error(msg);
        }
        return 0;
    }

    static SQInteger microTime(HSQUIRRELVM v)
    {
        sq_pushfloat(v, g_pEngine->getTime().asMilliseconds());
        return 1;
    }

    static SQInteger moveCursorTo(HSQUIRRELVM v)
    {
        SQInteger x;
        if (SQ_FAILED(sq_getinteger(v, 2, &x)))
        {
            return sq_throwerror(v, _SC("Failed to get x"));
        }
        SQInteger y;
        if (SQ_FAILED(sq_getinteger(v, 3, &y)))
        {
            return sq_throwerror(v, _SC("Failed to get y"));
        }
        SQFloat t;
        if (SQ_FAILED(sq_getfloat(v, 4, &t)))
        {
            return sq_throwerror(v, _SC("Failed to get time"));
        }

        // WIP need to be check
        auto p = sf::Mouse::getPosition(g_pEngine->getWindow());
        auto pos = g_pEngine->getWindow().mapCoordsToPixel(sf::Vector2f(x,y)-g_pEngine->getCamera().getAt());
        sf::Mouse::setPosition(pos, g_pEngine->getWindow());
        error("moveCursorTo not implemented");
        return 0;
    }

    static SQInteger pushSentence(HSQUIRRELVM v)
    {
        auto numArgs = sq_gettop(v);
        SQInteger id;
        if (SQ_FAILED(sq_getinteger(v, 2, &id)))
        {
            return sq_throwerror(v, _SC("Failed to get verb id"));
        }
        
        if(id == VerbConstants::VERB_DIALOG)
        {
            SQInteger choice;
            if (SQ_FAILED(sq_getinteger(v, 3, &choice)))
            {
                return sq_throwerror(v, _SC("Failed to get choice"));
            }
            g_pEngine->getDialogManager().choose(choice);
            return 0;
        }

        Entity* pObj1{nullptr};
        Entity* pObj2{nullptr};
        if(numArgs > 2)
        {
            pObj1 = ScriptEngine::getEntity(v, 3);
            if (!pObj1)
            {
                return sq_throwerror(v, _SC("Failed to get obj1"));
            }
        }
        if(numArgs > 3)
        {
            pObj2 = ScriptEngine::getEntity(v, 4);
            if (!pObj2)
            {
                return sq_throwerror(v, _SC("Failed to get obj2"));
            }
        }
        g_pEngine->pushSentence(static_cast<int>(id), pObj1, pObj2);
        return 0;
    }

    static SQInteger stopthread(HSQUIRRELVM v)
    {
        SQInteger id;
        if (SQ_SUCCEEDED(sq_getinteger(v, 2, &id)) && id == 0)
        {
            // no thread id => nothing to stop
            return 0;
        }
        HSQOBJECT thread_obj;
        if (SQ_FAILED(sq_getstackobj(v, 2, &thread_obj)))
        {
            return sq_throwerror(v, _SC("Couldn't get coroutine thread from stack"));
        }
        trace("stopthread {}", (long)thread_obj._unVal.pThread);

        auto stopThread = std::make_unique<_StopThread>(*g_pEngine, thread_obj);
        g_pEngine->addFunction(std::move(stopThread));

        return 0;
    }

    static SQInteger startglobalthread(HSQUIRRELVM v)
    {
        return startthread(v, true);
    }

    static SQInteger startthread(HSQUIRRELVM v)
    {
        return startthread(v, false);
    }

    static SQInteger startthread(HSQUIRRELVM v, bool global)
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

        const SQChar *name = nullptr;
        if (SQ_SUCCEEDED(sq_getclosurename(v, 2)))
        {
            sq_getstring(v, -1, &name);
        }

        trace("start thread ({}): {}", (name ? name : "anonymous"), (long)thread);
        auto vm = g_pEngine->getVm();
        auto pUniquethread = std::make_unique<Thread>(vm, thread_obj, env_obj, closureObj, args);
        auto pThread = pUniquethread.get();

        if (global)
        {
            g_pEngine->addThread(std::move(pUniquethread));
        }
        else
        {
            g_pEngine->getRoom()->addThread(std::move(pUniquethread));
        }

        // call the closure in the thread
        if (!pThread->call())
        {
            return sq_throwerror(v, _SC("call failed"));
        }

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
        g_pEngine->addFunction(std::make_unique<_BreakTimeFunction>(*g_pEngine, v, sf::seconds(time)));
        return result;
    }

    static SQInteger setPrivatePref(HSQUIRRELVM v)
    {
        _setPref(v, 
            [](auto key,auto value) { return g_pEngine->getPreferences().setPrivatePreference(key,value); },
            [](auto key) { return g_pEngine->getPreferences().removePrivatePreference(key); });
        return 0;
    }

    static SQInteger getPrivatePref(HSQUIRRELVM v)
    {
        return _getPref(v, [](auto name, auto value){ return g_pEngine->getPreferences().getPrivatePreferenceCore(name,value);});
    }

    static SQInteger getUserPref(HSQUIRRELVM v)
    {
        return _getPref(v, [](auto name, auto value){ return g_pEngine->getPreferences().getUserPreferenceCore(name,value);});
    }

    static SQInteger _getPref(HSQUIRRELVM v, std::function<std::any(const std::string &name, std::any value)> func)
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

        auto value = func(key, defaultValue);
        const auto &valueType = value.type();
        if (valueType == typeid(std::string))
        {
            sq_pushstring(v, std::any_cast<std::string>(value).data(), -1);
        }
        else if (valueType == typeid(SQInteger))
        {
            sq_pushinteger(v, std::any_cast<SQInteger>(value));
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

    static SQInteger removeCallback(HSQUIRRELVM v)
    {
        error("TODO: removeCallback: not implemented");
        return 0;
    }

    static SQInteger setAmbientLight(HSQUIRRELVM v)
    {
        SQInteger c = 0;
        if (SQ_FAILED(sq_getinteger(v, 2, &c)))
        {
            return sq_throwerror(v, _SC("failed to get color"));
        }
        auto color = _fromRgb(c);
        g_pEngine->getRoom()->setAmbientLight(color);
        return 0;
    }

    static SQInteger setUserPref(HSQUIRRELVM v)
    {
        _setPref(v, 
            [](auto key,auto value) { return g_pEngine->getPreferences().setUserPreference(key,value); },
            [](auto key) { return g_pEngine->getPreferences().removeUserPreference(key); });
        return 0;
    }

    static SQInteger _setPref(HSQUIRRELVM v, std::function<void(const std::string&, std::any)> setPref, std::function<void(const std::string&)> removePref)
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
            setPref(key, value);
            return 0;
        }
        if (type == SQObjectType::OT_INTEGER)
        {
            SQInteger integer;
            sq_getinteger(v, 3, &integer);
            value = integer;
            setPref(key, value);
            return 0;
        }
        if (type == SQObjectType::OT_BOOL)
        {
            SQBool b;
            sq_getbool(v, 3, &b);
            value = b;
            setPref(key, value);
            return 0;
        }
        if (type == SQObjectType::OT_FLOAT)
        {
            SQFloat fl;
            sq_getfloat(v, 3, &fl);
            value = fl;
            setPref(key, value);
            return 0;
        }

        g_pEngine->getPreferences().removeUserPreference(key);

        return 0;
    }

    static SQInteger include(HSQUIRRELVM v)
    {
        const SQChar *filename = nullptr;
        if (SQ_FAILED(sq_getstring(v, 2, &filename)))
        {
            return sq_throwerror(v, "failed to get filename");
        }
        trace("include {}", filename);
        _pScriptEngine->executeNutScript(filename);
        return 0;
    }

    static SQInteger inputHUD(HSQUIRRELVM v)
    {
        SQInteger on;
        if (SQ_FAILED(sq_getinteger(v, 2, &on)))
        {
            return sq_throwerror(v, _SC("failed to get on"));
        }
        g_pEngine->setInputHUD(on);
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

    static SQInteger inputState(HSQUIRRELVM v)
    {
        auto numArgs = sq_gettop(v);
        if(numArgs == 1)
        {
            auto state = g_pEngine->getInputState();
            sq_pushinteger(v, state);
            return 1;
        }
        else if(numArgs == 2)
        {
             SQInteger state;
            if (SQ_FAILED(sq_getinteger(v, 2, &state)))
            {
                return sq_throwerror(v, _SC("failed to get state"));
            }
            g_pEngine->setInputState(state);
            return 0;
        }
        error("TODO: inputState: not implemented");
        return 0;
    }

    static SQInteger inputController(HSQUIRRELVM v)
    {
        error("TODO: inputController: not implemented");
        return 0;
    }

    static SQInteger inputVerbs(HSQUIRRELVM v)
    {
        SQInteger on;
        if (SQ_FAILED(sq_getinteger(v, 2, &on)))
        {
            return sq_throwerror(v, _SC("failed to get isActive"));
        }
        g_pEngine->setInputVerbs(on);
        return 1;
    }

    static SQInteger is_table(HSQUIRRELVM v)
    {
        sq_pushbool(v, sq_gettype(v, 2) == OT_TABLE ? SQTrue : SQFalse);
        return 1;
    }

    static SQInteger ord(HSQUIRRELVM v)
    {
        const SQChar *letter;
        if (SQ_FAILED(sq_getstring(v, 2, &letter)))
        {
            return sq_throwerror(v, "Failed to get letter");
        }
        sq_pushinteger(v, (SQInteger)letter[0]);
        return 1;
    }

    static SQInteger is_string(HSQUIRRELVM v)
    {
        sq_pushbool(v, sq_gettype(v, 2) == OT_STRING ? SQTrue : SQFalse);
        return 1;
    }

    static SQInteger threadid(HSQUIRRELVM v)
    {
        error("TODO: threadid: not implemented");
        return 0;
    }

    static SQInteger threadpauseable(HSQUIRRELVM v)
    {
        HSQOBJECT thread;
        sq_resetobject(&thread);
        if (SQ_FAILED(sq_getstackobj(v, 2, &thread)))
        {
            return sq_throwerror(v, _SC("failed to get thread"));
        }
        SQInteger pauseable = 0;
        if (SQ_FAILED(sq_getinteger(v, 3, &pauseable)))
        {
            return sq_throwerror(v, _SC("failed to get pauseable"));
        }
        error("TODO: threadpauseable: not implemented");
        return 0;
    }
};

Engine *_SystemPack::g_pEngine = nullptr;
ScriptEngine *_SystemPack::_pScriptEngine = nullptr;

} // namespace ng
