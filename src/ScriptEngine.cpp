#include <memory>

#include <string>
#include <memory>
#include <iostream>
#include <squirrel3/squirrel.h>
#include <squirrel3/sqstdio.h>
#include <squirrel3/sqstdaux.h>
#include <squirrel3/sqstdblob.h>
#include <squirrel3/sqstdstring.h>
#include <squirrel3/sqstdmath.h>
#include <squirrel3/sqstdsystem.h>
#include "GGEngine.h"
#include "Screen.h"
#include "ScriptEngine.h"
#include "GGLip.h"

#ifdef SQUNICODE
#define scvprintf vfwprintf
#else
#define scvprintf vfprintf
#endif

namespace gg
{
static GGEngine *g_pEngine;

template <typename Value>
class _ChangeProperty : public TimeFunction
{
  public:
    _ChangeProperty(std::function<Value()> get, std::function<void(const Value &)> set, Value destination, const sf::Time &time)
        : TimeFunction(time),
          _get(get),
          _set(set),
          _destination(destination),
          _init(get()),
          _delta(_destination - _init),
          _current(_init)
    {
    }

    void operator()() override
    {
        _set(_current);
        if (!isElapsed())
        {
            _current = _init + (_clock.getElapsedTime().asSeconds() / _time.asSeconds()) * _delta;
        }
    }

  private:
    std::function<Value()> _get;
    std::function<void(const Value &)> _set;
    Value _destination;
    Value _init;
    Value _delta;
    Value _current;
};

template <typename TEntity>
static TEntity *_getEntity(HSQUIRRELVM v, SQInteger index)
{
    auto type = sq_gettype(v, index);
    // is it a table?
    if (type != OT_TABLE)
    {
        sq_pushbool(v, SQFalse);
        return nullptr;
    }

    HSQOBJECT object;
    if (SQ_FAILED(sq_getstackobj(v, index, &object)))
    {
        return nullptr;
    }

    sq_pushobject(v, object);
    sq_pushstring(v, _SC("instance"), -1);
    if (SQ_FAILED(sq_get(v, -2)))
    {
        sq_pop(v, 2);
        return nullptr;
    }

    GGEntity *pObj = nullptr;
    if (SQ_FAILED(sq_getuserpointer(v, -1, (SQUserPointer *)&pObj)))
    {
        sq_pop(v, 2);
        return nullptr;
    }

    return dynamic_cast<TEntity *>(pObj);
}

static GGObject *_getObject(HSQUIRRELVM v, int index)
{
    return _getEntity<GGObject>(v, index);
}

static GGRoom *_getRoom(HSQUIRRELVM v, int index)
{
    return _getEntity<GGRoom>(v, index);
}

static GGActor *_getActor(HSQUIRRELVM v, int index)
{
    return _getEntity<GGActor>(v, index);
}

template <class T>
static void _pushObject(HSQUIRRELVM v, T &object)
{
    sq_newtable(v);
    sq_pushstring(v, _SC("instance"), -1);
    sq_pushuserpointer(v, &object);
    sq_newslot(v, -3, SQFalse);
}

#include "_SystemCommands.h"
#include "_GeneralCommands.h"
#include "_ObjectCommands.h"
#include "_ActorCommands.h"
#include "_RoomCommands.h"
#include "_SoundCommands.h"

static SQInteger _isObject(HSQUIRRELVM v)
{
    auto object = _getEntity<GGObject>(v, 2);
    sq_pushbool(v, object ? SQTrue : SQFalse);
    return 1;
}

ScriptEngine::ScriptEngine(GGEngine &engine)
{
    g_pEngine = &engine;
    v = sq_open(1024);
    sq_setcompilererrorhandler(v, errorHandler);
    sq_newclosure(v, aux_printerror, 0);
    sq_seterrorhandler(v);
    sq_setprintfunc(v, printfunc, errorfunc); //sets the print function

    sq_pushroottable(v);
    sqstd_register_bloblib(v);
    sqstd_register_iolib(v);
    sqstd_register_systemlib(v);
    sqstd_register_mathlib(v);
    sqstd_register_stringlib(v);
    registerBoolConstant(_SC("NO"), false);
    registerBoolConstant(_SC("YES"), true);
    registerStringConstant(_SC("VERB_CLOSE"), _SC("close"));
    registerStringConstant(_SC("VERB_GIVE"), _SC("give"));
    registerStringConstant(_SC("VERB_LOOKAT"), _SC("lookat"));
    registerStringConstant(_SC("VERB_OPEN"), _SC("open"));
    registerStringConstant(_SC("VERB_PICKUP"), _SC("pickup"));
    registerStringConstant(_SC("VERB_PULL"), _SC("pull"));
    registerStringConstant(_SC("VERB_PUSH"), _SC("push"));
    registerStringConstant(_SC("VERB_TALKTO"), _SC("talkto"));
    registerStringConstant(_SC("VERB_USE"), _SC("use"));
    registerStringConstant(_SC("VERB_WALKTO"), _SC("walkto"));
    registerConstant(_SC("GONE"), 4);
    registerConstant(_SC("HERE"), 0);
    registerConstant(_SC("OFF"), 0);
    registerConstant(_SC("ON"), 1);
    registerConstant(_SC("OPEN"), 1);
    registerConstant(_SC("CLOSED"), 0);
    registerConstant(_SC("FULL"), 0);
    registerConstant(_SC("EMPTY"), 1);
    registerConstant(_SC("FADE_IN"), 0);
    registerConstant(_SC("FADE_OUT"), 1);
    registerConstant(_SC("FACE_FRONT"), 0);
    registerConstant(_SC("FACE_BACK"), 1);
    registerConstant(_SC("FACE_LEFT"), 2);
    registerConstant(_SC("FACE_RIGHT"), 3);
    registerConstant(_SC("DIR_FRONT"), 0);
    registerConstant(_SC("DIR_BACK"), 1);
    registerConstant(_SC("DIR_LEFT"), 2);
    registerConstant(_SC("DIR_RIGHT"), 3);
    registerGlobalFunction(_random, "random");
    registerGlobalFunction(_randomFrom, "randomfrom");
    registerGlobalFunction(_randomOdds, "randomOdds");

    registerGlobalFunction(_loopMusic, "loopMusic");
    registerGlobalFunction(_loopSound, "loopSound");
    registerGlobalFunction(_defineSound, "defineSound");
    registerGlobalFunction(_playSound, "playSound");
    registerGlobalFunction(_stopSound, "stopSound");
    registerGlobalFunction(_fadeOutSound, "fadeOutSound");

    registerGlobalFunction(_roomFade, "roomFade");
    registerGlobalFunction(_defineRoom, "defineRoom");
    registerGlobalFunction(_cameraInRoom, "cameraInRoom");

    registerGlobalFunction(_startThread, "startthread");
    registerGlobalFunction(_stopThread, "stopthread");
    registerGlobalFunction(_breakHere, "breakhere");
    registerGlobalFunction(_breakTime, "breaktime");
    registerGlobalFunction(_breakwhileanimating, "breakwhileanimating");
    registerGlobalFunction(_breakwhilewalking, "breakwhilewalking");
    registerGlobalFunction(_breakwhiletalking, "breakwhiletalking");

    registerGlobalFunction(_isObject, "isObject");
    registerGlobalFunction(_isObject, "is_object");

    registerGlobalFunction(_scale, "scale");
    registerGlobalFunction(_playState, "playObjectState");
    registerGlobalFunction(_objectHidden, "objectHidden");
    registerGlobalFunction(_objectAlpha, "objectAlpha");
    registerGlobalFunction(_objectAlphaTo, "objectAlphaTo");
    registerGlobalFunction(_objectHotspot, "objectHotspot");
    registerGlobalFunction(_objectOffset, "objectOffset");
    registerGlobalFunction(_objectOffsetTo, "objectOffsetTo");
    registerGlobalFunction(_objectMoveTo, "objectMoveTo");
    registerGlobalFunction(_objectState, "objectState");
    registerGlobalFunction(_objectScale, "objectScale");
    registerGlobalFunction(_objectAt, "objectAt");
    registerGlobalFunction(_objectPosX, "_objectPosX");
    registerGlobalFunction(_objectPosY, "_objectPosY");
    registerGlobalFunction(_objectSort, "objectSort");
    registerGlobalFunction(_objectRotate, "objectRotate");
    registerGlobalFunction(_objectRotateTo, "objectRotateTo");
    registerGlobalFunction(_objectParallaxLayer, "objectParallaxLayer");
    registerGlobalFunction(_objectTouchable, "objectTouchable");
    registerGlobalFunction(_objectLit, "objectLit");
    registerGlobalFunction(_objectOwner, "objectOwner");
    registerGlobalFunction(_objectUsePos, "objectUsePos");
    registerGlobalFunction(_objectColor, "objectColor");
    registerGlobalFunction(_objectIcon, "objectIcon");
    registerGlobalFunction(_objectFPS, "objectFPS");
    registerGlobalFunction(_createObject, "createObject");
    registerGlobalFunction(_createTextObject, "createTextObject");
    registerGlobalFunction(_deleteObject, "deleteObject");
    registerGlobalFunction(_translate, "translate");

    registerGlobalFunction(_actorAlpha, "actorAlpha");
    registerGlobalFunction(_actorAnimationNames, "actorAnimationNames");
    registerGlobalFunction(_actorAt, "actorAt");
    registerGlobalFunction(_actorBlinkRate, "actorBlinkRate");
    registerGlobalFunction(_actorColor, "actorColor");
    registerGlobalFunction(_actorCostume, "actorCostume");
    registerGlobalFunction(_actorDistanceTo, "actorDistanceTo");
    registerGlobalFunction(_actorDistanceWithin, "actorDistanceWithin");
    registerGlobalFunction(_actorFace, "actorFace");
    registerGlobalFunction(_actorHidden, "actorHidden");
    registerGlobalFunction(_actorHideLayer, "actorHideLayer");
    registerGlobalFunction(_actorInTrigger, "actorInTrigger");
    registerGlobalFunction(_actorLockFacing, "actorLockFacing");
    registerGlobalFunction(_actorPlayAnimation, "actorPlayAnimation");
    registerGlobalFunction(_actorPosX, "actorPosX");
    registerGlobalFunction(_actorPosY, "actorPosY");
    registerGlobalFunction(_actorRenderOffset, "actorRenderOffset");
    registerGlobalFunction(_actorRoom, "actorRoom");
    registerGlobalFunction(_actorShowLayer, "actorShowLayer");
    registerGlobalFunction(_actorTalkColors, "actorTalkColors");
    registerGlobalFunction(_actorTalking, "actorTalking");
    registerGlobalFunction(_actorTalkOffset, "actorTalkOffset");
    registerGlobalFunction(_actorUsePos, "actorUsePos");
    registerGlobalFunction(_actorUseWalkboxes, "actorUseWalkboxes");
    registerGlobalFunction(_actorWalkForward, "actorWalkForward");
    registerGlobalFunction(_actorWalkTo, "actorWalkTo");
    registerGlobalFunction(_createActor, "createActor");
    registerGlobalFunction(_currentActor, "currentActor");
    registerGlobalFunction(_isActor, "is_actor");
    registerGlobalFunction(_masterActorArray, "masterActorArray");
    registerGlobalFunction(_sayLine, "sayLine");
    registerGlobalFunction(_selectActor, "selectActor");
    registerGlobalFunction(_triggerActors, "triggerActors");

    registerGlobalFunction(_cameraAt, "cameraAt");
    registerGlobalFunction(_cameraPanTo, "cameraPanTo");
    registerGlobalFunction(_setVerb, "setVerb");
    registerGlobalFunction(_verbUIColors, "verbUIColors");
    registerGlobalFunction(_getUserPref, "getUserPref");
    registerGlobalFunction(_systemTime, "systemTime");
    registerGlobalFunction(_inputOff, "inputOff");
    registerGlobalFunction(_inputOn, "inputOn");
    registerGlobalFunction(_inputSilentOff, "inputSilentOff");
    registerGlobalFunction(_isInputOn, "isInputOn");
    registerGlobalFunction(_inputVerbs, "inputVerbs");
}

SQInteger ScriptEngine::aux_printerror(HSQUIRRELVM v)
{
    auto pf = sq_geterrorfunc(v);
    if (!pf)
        return 0;

    if (sq_gettop(v) < 1)
        return 0;

    const SQChar *error = nullptr;
    if (SQ_FAILED(sq_getstring(v, 2, &error)))
    {
        error = "unknown";
    }
    pf(v, _SC("\nAn error occured in the script: %s\n"), error);
    sqstd_printcallstack(v);

    return 0;
}

void ScriptEngine::errorHandler(HSQUIRRELVM v, const SQChar *desc, const SQChar *source, SQInteger line, SQInteger column)
{
    std::cerr << desc << source << '(' << line << ',' << column << ')' << std::endl;
}

void ScriptEngine::errorfunc(HSQUIRRELVM v, const SQChar *s, ...)
{
    va_list vl;
    va_start(vl, s);
    scvprintf(stderr, s, vl);
    va_end(vl);
}

void ScriptEngine::printfunc(HSQUIRRELVM v, const SQChar *s, ...)
{
    va_list vl;
    va_start(vl, s);
    scvprintf(stdout, s, vl);
    va_end(vl);
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
    sq_pushbool(v, static_cast<SQBool>(value));
    sq_newslot(v, -3, SQTrue);
    sq_pop(v, 1);
}

void ScriptEngine::registerStringConstant(const SQChar *name, const SQChar *value)
{
    sq_pushconsttable(v);
    sq_pushstring(v, name, -1);
    sq_pushstring(v, value, -1);
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

void ScriptEngine::registerGlobalFunction(SQFUNCTION f, const SQChar *functionName, SQInteger nparamscheck, const SQChar *typemask)
{
    sq_pushroottable(v);
    sq_pushstring(v, functionName, -1);
    sq_newclosure(v, f, 0); //create a new function
    sq_setparamscheck(v, nparamscheck, typemask);
    sq_newslot(v, -3, SQFalse);
    sq_pop(v, 1); //pops the root table
}

void ScriptEngine::executeScript(const std::string &name)
{
    std::cout << "execute " << name << std::endl;
    if (SQ_FAILED(sqstd_dofile(v, name.c_str(), SQFalse, SQTrue)))
    {
        std::cerr << "failed to execute " << name << std::endl;
        return;
    }
}
} // namespace gg
