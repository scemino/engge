#include <string>
#include <memory>
#include <squirrel3/squirrel.h>
#include <squirrel3/sqstdio.h>
#include <squirrel3/sqstdaux.h>
#include <squirrel3/sqstdblob.h>
#include <squirrel3/sqstdstring.h>
#include <squirrel3/sqstdmath.h>
#include <squirrel3/sqstdsystem.h>
#include "GGEngine.h"
#include "ScriptEngine.h"

#ifdef SQUNICODE
#define scvprintf vfwprintf
#else
#define scvprintf vfprintf
#endif

namespace gg
{
static GGEngine *g_pEngine;

class _BreakHereFunction : public Function
{
  private:
    HSQUIRRELVM _vm;

  public:
    _BreakHereFunction(HSQUIRRELVM vm)
        : _vm(vm)
    {
    }

    void operator()() override
    {
        if (sq_getvmstate(_vm) != SQ_VMSTATE_SUSPENDED)
        {
            printf("_BreakHereFunction: thread not suspended\n");
            sqstd_printcallstack(_vm);
        }
        if (SQ_FAILED(sq_wakeupvm(_vm, SQFalse, SQFalse, SQTrue, SQFalse)))
        {
            printf("_BreakHereFunction: failed to wakeup: %p\n", _vm);
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
            {
                printf("_BreakTimeFunction: thread not suspended\n");
                sqstd_printcallstack(_vm);
            }
            if (SQ_FAILED(sq_wakeupvm(_vm, SQFalse, SQFalse, SQTrue, SQFalse)))
            {
                printf("_BreakTimeFunction: failed to wakeup: %p\n", _vm);
                sqstd_printcallstack(_vm);
            }
        }
    }
};

class _ObjectRotateToFunction : public TimeFunction
{
  private:
    GGObject &_object;
    int _rotate;
    int _rotateInit;
    float _delta;

  public:
    _ObjectRotateToFunction(GGObject &object, int rotate, sf::Time time)
        : TimeFunction(time),
          _object(object),
          _rotate(_object.getRotation()),
          _rotateInit(_object.getRotation()),
          _delta(rotate - _rotateInit)
    {
    }

    bool isElapsed() override
    {
        return false;
    }

    void operator()() override
    {
        _object.setRotation(_rotate);
        _rotate = _rotateInit + (_clock.getElapsedTime().asSeconds() / _time.asSeconds()) * _delta;
    }
};

void _errorHandler(HSQUIRRELVM v, const SQChar *desc, const SQChar *source, SQInteger line, SQInteger column)
{
    printf("%s %s (%lld,%lld)\n", desc, source, line, column);
}

void _printfunc(HSQUIRRELVM v, const SQChar *s, ...)
{
    va_list vl;
    va_start(vl, s);
    scvprintf(stdout, s, vl);
    va_end(vl);
}

void _errorfunc(HSQUIRRELVM v, const SQChar *s, ...)
{
    va_list vl;
    va_start(vl, s);
    scvprintf(stderr, s, vl);
    va_end(vl);
}

static GGObject *_getObject(HSQUIRRELVM v, int index)
{
    GGObject *obj = nullptr;
    const SQChar *name;

    sq_push(v, index);
    sq_pushnull(v); //null iterator
    while (SQ_SUCCEEDED(sq_next(v, -2)))
    {
        //here -1 is the value and -2 is the key
        sq_getstring(v, -2, &name);
        sq_getuserpointer(v, -1, (SQUserPointer *)&obj);
        sq_pop(v, 2); //pops key and val before the nex iteration
    }

    sq_pop(v, 1); //pops the null iterator
    return obj;
}

static GGActor *_getActor(HSQUIRRELVM v, int index)
{
    GGActor *actor = nullptr;
    const SQChar *name;

    sq_push(v, index);
    sq_pushnull(v); //null iterator
    while (SQ_SUCCEEDED(sq_next(v, -2)))
    {
        //here -1 is the value and -2 is the key
        sq_getstring(v, -2, &name);
        sq_getuserpointer(v, -1, (SQUserPointer *)&actor);
        sq_pop(v, 2); //pops key and val before the nex iteration
    }

    sq_pop(v, 1); //pops the null iterator
    return actor;
}

static SQInteger _isObject(HSQUIRRELVM v)
{
    auto type = sq_gettype(v, 2);
    if (type != OT_TABLE)
    {
        sq_pushbool(v, SQFalse);
        return 1;
    }

    GGObject *obj = _getObject(v, 2);
    sq_pushbool(v, obj != nullptr ? SQTrue : SQFalse);
    return 1;
}

static SQInteger _scale(HSQUIRRELVM v)
{
    SQFloat s = 0;
    sq_getfloat(v, 3, &s);
    GGObject *self = _getObject(v, 2);
    self->setScale(s);
    return 0;
}

static SQInteger _objectAlpha(HSQUIRRELVM v)
{
    GGObject *obj = _getObject(v, 2);
    SQFloat alpha = 0;
    if (SQ_FAILED(sq_getfloat(v, 3, &alpha)))
    {
        return sq_throwerror(v, _SC("failed to get alpha\n"));
    }
    alpha = alpha > 1.f ? 1.f : alpha;
    alpha = alpha < 0.f ? 0.f : alpha;
    auto a = (sf::Uint8)(alpha * 255);
    auto color = obj->getColor();
    obj->setColor(sf::Color(color.r, color.g, color.b, a));
    return 0;
}

static SQInteger _objectAlphaTo(HSQUIRRELVM v)
{
    GGObject *obj = _getObject(v, 2);
    SQFloat alpha = 0;
    if (SQ_FAILED(sq_getfloat(v, 3, &alpha)))
    {
        return sq_throwerror(v, _SC("failed to get alpha\n"));
    }
    alpha = alpha > 1.f ? 1.f : alpha;
    alpha = alpha < 0.f ? 0.f : alpha;
    SQFloat time = 0;
    if (SQ_FAILED(sq_getfloat(v, 4, &time)))
        time = 1.f;
    auto a = (sf::Uint8)(alpha * 255);
    g_pEngine->alphaTo(*obj, a, time);
    return 0;
}

static SQInteger _objectHotspot(HSQUIRRELVM v)
{
    SQInteger left = 0;
    SQInteger top = 0;
    SQInteger right = 0;
    SQInteger bottom = 0;
    if (SQ_FAILED(sq_getinteger(v, 3, &left)))
    {
        return sq_throwerror(v, _SC("failed to get left\n"));
    }
    if (SQ_FAILED(sq_getinteger(v, 4, &top)))
    {
        return sq_throwerror(v, _SC("failed to get top\n"));
    }
    if (SQ_FAILED(sq_getinteger(v, 5, &right)))
    {
        return sq_throwerror(v, _SC("failed to get right\n"));
    }
    if (SQ_FAILED(sq_getinteger(v, 6, &bottom)))
    {
        return sq_throwerror(v, _SC("failed to get bottom\n"));
    }
    GGObject *obj = _getObject(v, 2);
    obj->setHotspot(sf::IntRect(left, top, right - left, bottom - top));
    return 0;
}

static SQInteger _objectOffset(HSQUIRRELVM v)
{
    SQInteger x = 0;
    SQInteger y = 0;
    GGObject *obj = _getObject(v, 2);
    if (SQ_FAILED(sq_getinteger(v, 3, &x)))
    {
        return sq_throwerror(v, _SC("failed to get x\n"));
    }
    if (SQ_FAILED(sq_getinteger(v, 4, &y)))
    {
        return sq_throwerror(v, _SC("failed to get y\n"));
    }
    obj->move(x, y);
    return 0;
}

static SQInteger _objectState(HSQUIRRELVM v)
{
    GGObject *obj = _getObject(v, 2);
    auto numArgs = sq_gettop(v) - 2;
    if (numArgs == 1)
    {
        sq_pushbool(v, obj->isVisible());
        return 1;
    }
    else
    {
        SQBool isVisible;
        sq_getbool(v, 3, &isVisible);
        obj->setVisible(isVisible);
        return 0;
    }
}

static SQInteger _objectOffsetTo(HSQUIRRELVM v)
{
    SQInteger x = 0;
    SQInteger y = 0;
    SQFloat t = 0;
    GGObject *obj = _getObject(v, 2);
    if (SQ_FAILED(sq_getinteger(v, 3, &x)))
    {
        return sq_throwerror(v, _SC("failed to get x\n"));
    }
    if (SQ_FAILED(sq_getinteger(v, 4, &y)))
    {
        return sq_throwerror(v, _SC("failed to get y\n"));
    }
    if (SQ_FAILED(sq_getfloat(v, 5, &t)))
    {
        return sq_throwerror(v, _SC("failed to get t\n"));
    }
    g_pEngine->offsetTo(*obj, x, y, t);
    return 0;
}

static SQInteger _play_state(HSQUIRRELVM v)
{
    SQInteger index;
    GGObject *obj = _getObject(v, 2);
    sq_getinteger(v, 3, &index);
    printf("push playState: %lld\n", index);
    g_pEngine->playState(*obj, index);
    return 0;
}

static SQInteger _actorCostume(HSQUIRRELVM v)
{
    const SQChar *name;
    GGActor *actor = _getActor(v, 2);
    sq_getstring(v, 3, &name);
    actor->setCostume(name);
    return 0;
}

static SQInteger _actorLockFacing(HSQUIRRELVM v)
{
    SQInteger facing;
    GGActor *actor = _getActor(v, 2);
    sq_getinteger(v, 3, &facing);
    actor->getCostume().lockFacing((Facing)facing);
    return 0;
}

static SQInteger _actorPlayAnimation(HSQUIRRELVM v)
{
    const SQChar *name;
    GGActor *actor = _getActor(v, 2);
    sq_getstring(v, 3, &name);
    actor->getCostume().setState(name);
    return 0;
}

static SQInteger _actorAt(HSQUIRRELVM v)
{
    GGActor *actor = _getActor(v, 2);
    GGObject *obj = _getObject(v, 3);
    auto pos = obj->getPosition();
    actor->setPosition(pos.x, pos.y);
    return 0;
}

static SQInteger _sayLine(HSQUIRRELVM v)
{
    GGActor *actor = _getActor(v, 2);
    const SQChar *text;
    sq_getstring(v, 3, &text);
    // TOSO: actor->say(text);
    return 0;
}

static SQInteger _objectAt(HSQUIRRELVM v)
{
    SQInteger x, y;
    GGObject *obj = _getObject(v, 2);
    sq_getinteger(v, 3, &x);
    sq_getinteger(v, 4, &y);
    obj->setPosition(x, y);
    return 0;
}

static SQInteger _objectPosX(HSQUIRRELVM v)
{
    GGObject *obj = _getObject(v, 2);
    auto pos = obj->getPosition();
    sq_pushinteger(v, pos.x);
    return 1;
}

static SQInteger _objectPosY(HSQUIRRELVM v)
{
    GGObject *obj = _getObject(v, 2);
    auto pos = obj->getPosition();
    sq_pushinteger(v, pos.y);
    return 1;
}

static SQInteger _objectSort(HSQUIRRELVM v)
{
    SQInteger zorder;
    GGObject *obj = _getObject(v, 2);
    sq_getinteger(v, 3, &zorder);
    obj->setZOrder(zorder);
    return 0;
}

static SQInteger _objectRotateTo(HSQUIRRELVM v)
{
    SQInteger dir;
    SQInteger t;
    GGObject *obj = _getObject(v, 2);
    sq_getinteger(v, 3, &dir);
    sq_getinteger(v, 4, &t);
    g_pEngine->addFunction(std::make_unique<_ObjectRotateToFunction>(*obj, dir, sf::seconds(t)));
    return 0;
}

static SQInteger _actorUsePos(HSQUIRRELVM v)
{
    GGActor *actor = _getActor(v, 2);
    GGObject *obj = _getObject(v, 3);
    auto pos = obj->getUsePosition();
    actor->setPosition(pos.x, pos.y);
    return 0;
}

static SQInteger _actorTalkColors(HSQUIRRELVM v)
{
    auto actor = _getActor(v, 2);
    SQInteger color;
    sq_getinteger(v, 3, &color);
    actor->setTalkColor(sf::Color(color << 8 | 0xff));
    return 0;
}

static SQInteger _cameraAt(HSQUIRRELVM v)
{
    SQInteger x, y;
    sq_getinteger(v, 2, &x);
    sq_getinteger(v, 3, &y);
    g_pEngine->setCameraAt(x - 160, y - 90);
    return 0;
}

static SQInteger _cameraPanTo(HSQUIRRELVM v)
{
    SQInteger x, y;
    SQFloat t;
    sf::View view(sf::FloatRect(0, 0, 320, 180));
    sq_getinteger(v, 2, &x);
    sq_getinteger(v, 3, &y);
    sq_getfloat(v, 4, &t);
    g_pEngine->cameraPanTo(x - 160, y - 90, t);
    return 0;
}

static SQInteger _state(HSQUIRRELVM v)
{
    SQInteger index;
    GGObject *obj = _getObject(v, 2);
    sq_getinteger(v, 3, &index);
    obj->setStateAnimIndex(index);
    return 0;
}

static SQInteger _hidden(HSQUIRRELVM v)
{
    SQBool hidden;
    GGObject *obj = _getObject(v, 2);
    sq_getbool(v, 3, &hidden);
    obj->setVisible(!hidden);
    return 0;
}

static SQInteger _break_here(HSQUIRRELVM v)
{
    // printf("push BreakHereFunction\n");
    auto result = sq_suspendvm(v);
    g_pEngine->addFunction(std::unique_ptr<_BreakHereFunction>(new _BreakHereFunction(v)));
    return result;
}

static SQInteger _break_time(HSQUIRRELVM v)
{
    SQFloat time = 0;
    sq_getfloat(v, 2, &time);
    // printf("push BreakTimeFunction\n");
    auto result = sq_suspendvm(v);
    g_pEngine->addFunction(std::unique_ptr<_BreakTimeFunction>(new _BreakTimeFunction(v, sf::seconds(time))));
    return result;
}

static void _push_object(HSQUIRRELVM v, GGObject &object)
{
    sq_newtable(v);
    sq_pushstring(v, _SC("instance"), -1);
    sq_pushuserpointer(v, &object);
    sq_newslot(v, -3, SQFalse);
}

static void _set_object_slot(HSQUIRRELVM v, const SQChar *name, GGObject &object)
{
    printf("set room object: %s\n", name);
    sq_pushstring(v, name, -1);
    _push_object(v, object);
    sq_newslot(v, -3, SQFalse);
}

static SQInteger _translate(HSQUIRRELVM v)
{
    const SQChar *idText;
    sq_getstring(v, 2, &idText);
    std::string s(idText);
    s = s.substr(1);
    auto id = std::atoi(s.c_str());
    auto text = g_pEngine->getText(id);
    sq_pushstring(v, text.c_str(), -1);
    return 1;
}

static SQInteger _createTextObject(HSQUIRRELVM v)
{
    const SQChar *name;
    const SQChar *text;
    sq_getstring(v, 2, &name);
    std::string n(name);
    auto &obj = g_pEngine->getRoom().createTextObject(name, g_pEngine->getFont());

    sq_getstring(v, 3, &text);
    std::string s(text);
    obj.setText(s);

    _push_object(v, obj);
    return 1;
}

static SQInteger _deleteObject(HSQUIRRELVM v)
{
    auto *obj = _getObject(v, 2);
    g_pEngine->getRoom().deleteObject(*obj);
    return 0;
}

static SQInteger _createObject(HSQUIRRELVM v)
{
    auto numArgs = sq_gettop(v) - 1;
    if (numArgs == 1)
    {
        std::vector<std::string> anims;
        for (int i = 0; i < numArgs; i++)
        {
            const SQChar *animName;
            sq_getstring(v, 2 + i, &animName);
            anims.push_back(animName);
        }
        auto &obj = g_pEngine->getRoom().createObject(anims);
        _push_object(v, obj);
        return 1;
    }

    HSQOBJECT obj;
    const SQChar *name;
    sq_getstring(v, 2, &name);
    sq_getstackobj(v, 3, &obj);
    if (sq_isarray(obj))
    {
        std::vector<std::string> anims;
        sq_push(v, 3);
        sq_pushnull(v); //null iterator
        while (SQ_SUCCEEDED(sq_next(v, -2)))
        {
            sq_getstring(v, -1, &name);
            anims.push_back(name);
            sq_pop(v, 2);
        }
        sq_pop(v, 1); //pops the null iterator
        auto &obj = g_pEngine->getRoom().createObject(anims);
        _push_object(v, obj);
    }
    return 1;
}

static SQInteger _loadRoom(HSQUIRRELVM v)
{
    const SQChar *name;
    sq_getstring(v, 2, &name);
    auto &room = g_pEngine->getRoom();
    room.load(name);

    sq_newtable(v);
    for (auto &obj : room.getObjects())
    {
        _set_object_slot(v, obj.get()->getName().c_str(), *obj.get());
    }

    return 1;
}

static SQInteger _createActor(HSQUIRRELVM v)
{
    auto pActor = new GGActor(g_pEngine->getTextureManager());
    sq_newtable(v);
    sq_pushstring(v, _SC("instance"), -1);
    sq_pushuserpointer(v, pActor);
    sq_newslot(v, -3, SQFalse);

    g_pEngine->addActor(*pActor);

    return 1;
}

static SQInteger _stop_thread(HSQUIRRELVM v)
{
    HSQOBJECT thread_obj;
    sq_resetobject(&thread_obj);
    if (sq_getstackobj(v, 2, &thread_obj) < 0)
    {
        return sq_throwerror(v, _SC("Couldn't get coroutine thread from stack"));
    }
    sq_release(thread_obj._unVal.pThread, &thread_obj);
    sq_pop(thread_obj._unVal.pThread, 1);
    sq_suspendvm(thread_obj._unVal.pThread);
    // sq_close(thread_obj._unVal.pThread);
    return 0;
}

static SQInteger _start_thread(HSQUIRRELVM v)
{
    SQInteger size = sq_gettop(v);

    // create thread and store it on the stack
    auto thread = sq_newthread(v, 1024);
    printf("startThread: %p\n", thread);
    sq_setcompilererrorhandler(thread, _errorHandler);
    sq_setprintfunc(thread, _printfunc, _errorfunc); //sets the print function
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
    sq_pushroottable(thread);
    for (auto arg : args)
    {
        sq_pushobject(thread, arg);
    }
    if (SQ_FAILED(sq_call(thread, 1 + args.size(), SQFalse, SQTrue)))
    {
        sq_throwerror(v, _SC("call failed\n"));
        sq_pop(thread, 1); // pop the compiled closure
        return SQ_ERROR;
    }

    sq_addref(v, &thread_obj);
    sq_pushobject(v, thread_obj);

    return 1;
}

SQInteger _int_rand(SQInteger min, SQInteger max)
{
    max++;
    auto value = (rand() % (max - min)) + min;
    return value;
}

float float_rand(float min, float max)
{
    float scale = rand() / (float)RAND_MAX; /* [0, 1.0] */
    return min + scale * (max - min);       /* [min, max] */
}

SQInteger _random(HSQUIRRELVM v)
{
    if (sq_gettype(v, 2) == OT_INTEGER)
    {
        SQInteger min = 0;
        SQInteger max = 0;
        sq_getinteger(v, 2, &min);
        sq_getinteger(v, 3, &max);
        auto value = _int_rand(min, max);
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

SQInteger _randomOdds(HSQUIRRELVM v)
{
    SQFloat value = 0;
    sq_getfloat(v, 2, &value);
    auto rnd = float_rand(0, 1);
    sq_pushbool(v, rnd <= value);
    return 1;
}

SQInteger _randomFrom(HSQUIRRELVM v)
{
    auto size = sq_gettop(v);
    auto index = _int_rand(0, size - 2);
    sq_push(v, 2 + index);
    return 1;
}

SQInteger _loopMusic(HSQUIRRELVM v)
{
    const SQChar *filename;
    sq_getstring(v, 2, &filename);
    g_pEngine->loopMusic(filename);
    return 0;
}

SQInteger _loopSound(HSQUIRRELVM v)
{
    const SQChar *filename;
    sq_getstring(v, 2, &filename);
    SQUserPointer ptr = g_pEngine->playSound(filename, true);
    sq_pushuserpointer(v, ptr);
    return 1;
}

SQInteger _fadeOutSound(HSQUIRRELVM v)
{
    SQUserPointer ptr;
    sq_getuserpointer(v, 2, &ptr);
    auto pSound = static_cast<SoundId *>(ptr);
    if (pSound == nullptr)
        return 0;
    float t;
    sq_getfloat(v, 3, &t);
    g_pEngine->fadeOutSound(*pSound, t);
    return 0;
}

SQInteger _roomFade(HSQUIRRELVM v)
{
    SQInteger type;
    SQFloat t;
    sq_getinteger(v, 2, &type);
    sq_getfloat(v, 3, &t);
    if (type == 0)
    {
        g_pEngine->fadeTo(0, t);
    }
    else
    {
        g_pEngine->fadeTo(255, t);
    }
    return 0;
}

SQInteger _playSound(HSQUIRRELVM v)
{
    const SQChar *filename;
    sq_getstring(v, 2, &filename);
    SQUserPointer ptr = g_pEngine->playSound(filename, false);
    sq_pushuserpointer(v, ptr);
    return 0;
}

SQInteger _stopSound(HSQUIRRELVM v)
{
    SoundId *soundId;
    sq_getuserpointer(v, 2, (SQUserPointer *)&soundId);
    g_pEngine->stopSound(*soundId);
    return 0;
}

ScriptEngine::ScriptEngine(GGEngine &engine)
{
    g_pEngine = &engine;
    v = sq_open(1024);
    printf("start main thread: %p\n", v);
    sq_seterrorhandler(v);
    sq_setcompilererrorhandler(v, _errorHandler);
    sq_setprintfunc(v, _printfunc, _errorfunc); //sets the print function

    sq_pushroottable(v);
    sqstd_register_bloblib(v);
    sqstd_register_iolib(v);
    sqstd_register_systemlib(v);
    sqstd_register_mathlib(v);
    sqstd_register_stringlib(v);
    registerBoolConstant(_SC("NO"), false);
    registerBoolConstant(_SC("YES"), true);
    registerBoolConstant(_SC("GONE"), false);
    registerBoolConstant(_SC("HERE"), true);
    registerConstant(_SC("FADE_IN"), 0);
    registerConstant(_SC("FADE_OUT"), 1);
    registerConstant(_SC("FACE_FRONT"), 0);
    registerConstant(_SC("FACE_BACK"), 1);
    registerConstant(_SC("FACE_LEFT"), 2);
    registerConstant(_SC("FACE_RIGHT"), 3);
    registerGlobalFunction(_random, "random");
    registerGlobalFunction(_randomFrom, "randomfrom");
    registerGlobalFunction(_randomOdds, "randomOdds");
    // registerGlobalFunction(_dump, "dump");
    registerGlobalFunction(_loopMusic, "loopMusic");
    registerGlobalFunction(_loopSound, "loopSound");
    registerGlobalFunction(_playSound, "playSound");
    registerGlobalFunction(_stopSound, "stopSound");
    registerGlobalFunction(_fadeOutSound, "fadeOutSound");
    registerGlobalFunction(_roomFade, "roomFade");

    registerGlobalFunction(_start_thread, "startthread");
    registerGlobalFunction(_stop_thread, "stopthread");
    registerGlobalFunction(_break_here, "breakhere");
    registerGlobalFunction(_break_time, "breaktime");
    registerGlobalFunction(_loadRoom, "loadRoom");

    registerGlobalFunction(_isObject, "isObject");
    registerGlobalFunction(_hidden, "objectHidden");
    registerGlobalFunction(_scale, "scale");
    registerGlobalFunction(_state, "objectState");
    registerGlobalFunction(_play_state, "playObjectState");
    registerGlobalFunction(_objectAlpha, "objectAlpha");
    registerGlobalFunction(_objectAlphaTo, "objectAlphaTo");
    registerGlobalFunction(_objectHotspot, "objectHotspot");
    registerGlobalFunction(_objectOffset, "objectOffset");
    registerGlobalFunction(_objectOffsetTo, "objectOffsetTo");
    registerGlobalFunction(_objectState, "objectState");
    registerGlobalFunction(_objectAt, "objectAt");
    registerGlobalFunction(_objectPosX, "_objectPosX");
    registerGlobalFunction(_objectPosY, "_objectPosY");
    registerGlobalFunction(_objectSort, "objectSort");
    registerGlobalFunction(_objectRotateTo, "objectRotateTo");
    registerGlobalFunction(_createObject, "createObject");
    registerGlobalFunction(_createTextObject, "createTextObject");
    registerGlobalFunction(_deleteObject, "deleteObject");
    registerGlobalFunction(_translate, "translate");

    registerGlobalFunction(_createActor, "createActor");
    registerGlobalFunction(_actorCostume, "actorCostume");
    registerGlobalFunction(_actorLockFacing, "actorLockFacing");
    registerGlobalFunction(_actorLockFacing, "actorLockFacing");
    registerGlobalFunction(_actorPlayAnimation, "actorPlayAnimation");
    registerGlobalFunction(_actorAt, "actorAt");
    registerGlobalFunction(_sayLine, "sayLine");
    registerGlobalFunction(_actorUsePos, "actorUsePos");
    registerGlobalFunction(_actorTalkColors, "actorTalkColors");
    registerGlobalFunction(_cameraAt, "cameraAt");
    registerGlobalFunction(_cameraPanTo, "cameraPanTo");
}

ScriptEngine::~ScriptEngine()
{
    sq_pop(v, 1);
    sq_close(v);
}

void ScriptEngine::registerBoolConstant(const SQChar *name, bool value)
{
    sq_pushconsttable(v);
    sq_pushstring(v, name, -1);
    sq_pushbool(v, value);
    sq_newslot(v, -3, SQTrue);
    sq_pop(v, 1);
}

void ScriptEngine::registerConstant(const SQChar *name, SQInteger value)
{
    sq_pushconsttable(v);
    sq_pushstring(v, name, -1);
    sq_pushinteger(v, value);
    sq_newslot(v, -3, SQTrue);
    sq_pop(v, 1);
}

void ScriptEngine::registerGlobalFunction(SQFUNCTION f, const SQChar *fname)
{
    sq_pushroottable(v);
    sq_pushstring(v, fname, -1);
    sq_newclosure(v, f, 0); //create a new function
    sq_newslot(v, -3, SQFalse);
    sq_pop(v, 1); //pops the root table
}

void ScriptEngine::executeScript(const std::string &name)
{
    printf("execute %s\n", name.c_str());
    if (SQ_FAILED(sqstd_dofile(v, name.c_str(), SQFalse, SQTrue)))
    {
        printf("failed to execute %s\n", name.c_str());
        return;
    }
}
} // namespace gg
