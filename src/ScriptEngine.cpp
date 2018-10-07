#include <squirrel3/squirrel.h>
#include <squirrel3/sqstdio.h>
#include <squirrel3/sqstdaux.h>
#include <squirrel3/sqstdstring.h>
#include <squirrel3/sqstdmath.h>
#include "ScriptEngine.h"
#include "_SystemCommands.h"
#include "_GeneralCommands.h"
#include "_ObjectCommands.h"
#include "_ActorCommands.h"
#include "_RoomCommands.h"
#include "_SoundCommands.h"

#ifdef SQUNICODE
#define scvprintf vfwprintf
#else
#define scvprintf vfprintf
#endif

namespace gg
{
template <>
void ScriptEngine::registerConstant(const SQChar *name, bool value)
{
    sq_pushconsttable(v);
    sq_pushstring(v, name, -1);
    sq_pushbool(v, value ? SQTrue : SQFalse);
    sq_newslot(v, -3, SQTrue);
    sq_pop(v, 1);
}

template <>
void ScriptEngine::registerConstant(const SQChar *name, int value)
{
    sq_pushconsttable(v);
    sq_pushstring(v, name, -1);
    sq_pushinteger(v, value);
    sq_newslot(v, -3, SQTrue);
    sq_pop(v, 1);
}

template <>
void ScriptEngine::registerConstant(const SQChar *name, const SQChar *value)
{
    sq_pushconsttable(v);
    sq_pushstring(v, name, -1);
    sq_pushstring(v, value, -1);
    sq_newslot(v, -3, SQTrue);
    sq_pop(v, 1);
}

ScriptEngine::ScriptEngine(GGEngine &engine)
    : _engine(engine)
{
    v = sq_open(1024);
    sq_setcompilererrorhandler(v, errorHandler);
    sq_newclosure(v, aux_printerror, 0);
    sq_seterrorhandler(v);
    sq_setprintfunc(v, printfunc, errorfunc); //sets the print function

    sq_pushroottable(v);
    sqstd_register_mathlib(v);
    sqstd_register_stringlib(v);
    registerConstant(_SC("NO"), false);
    registerConstant(_SC("YES"), true);
    registerConstant(_SC("VERB_CLOSE"), _SC("close"));
    registerConstant(_SC("VERB_GIVE"), _SC("give"));
    registerConstant(_SC("VERB_LOOKAT"), _SC("lookat"));
    registerConstant(_SC("VERB_OPEN"), _SC("open"));
    registerConstant(_SC("VERB_PICKUP"), _SC("pickup"));
    registerConstant(_SC("VERB_PULL"), _SC("pull"));
    registerConstant(_SC("VERB_PUSH"), _SC("push"));
    registerConstant(_SC("VERB_TALKTO"), _SC("talkto"));
    registerConstant(_SC("VERB_USE"), _SC("use"));
    registerConstant(_SC("VERB_WALKTO"), _SC("walkto"));
    registerConstant(_SC("GONE"), 1);
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
    registerConstant(_SC("LINEAR"), 0);
    registerConstant(_SC("EASE_IN"), 1);
    registerConstant(_SC("EASE_INOUT"), 2);
    registerConstant(_SC("EASE_OUT"), 3);
    registerConstant(_SC("SLOW_EASE_IN"), 4);
    registerConstant(_SC("SLOW_EASE_OUT"), 5);

    addPack<_ActorPack>();
    addPack<_GeneralPack>();
    addPack<_ObjectPack>();
    addPack<_RoomPack>();
    addPack<_SoundPack>();
    addPack<_SystemPack>();
}

ScriptEngine::~ScriptEngine()
{
    sq_pop(v, 1);
    sq_close(v);
}

GGEngine &ScriptEngine::getEngine()
{
    return _engine;
}

GGObject *ScriptEngine::getObject(HSQUIRRELVM v, SQInteger index)
{
    return ScriptEngine::getEntity<GGObject>(v, index);
}

GGRoom *ScriptEngine::getRoom(HSQUIRRELVM v, SQInteger index)
{
    return ScriptEngine::getEntity<GGRoom>(v, index);
}

GGActor *ScriptEngine::getActor(HSQUIRRELVM v, SQInteger index)
{
    return ScriptEngine::getEntity<GGActor>(v, index);
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

std::function<float(float)> ScriptEngine::getInterpolationMethod(InterpolationMethod index)
{
    switch (index)
    {
    case InterpolationMethod::EaseIn:
        return Interpolations::easeIn;
    case InterpolationMethod::EaseInOut:
        return Interpolations::easeInOut;
    case InterpolationMethod::EaseOut:
        return Interpolations::easeOut;
    default:
        return Interpolations::linear;
    }
}
} // namespace gg
