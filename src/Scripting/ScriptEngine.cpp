#include "squirrel.h"
#include "sqstdio.h"
#include "sqstdaux.h"
#include "sqstdstring.h"
#include "sqstdmath.h"
#include "ScriptEngine.h"
#include "_SystemPack.h"
#include "_GeneralPack.h"
#include "_ObjectPack.h"
#include "_ActorPack.h"
#include "_RoomPack.h"
#include "_SoundPack.h"
#include "_DefaultVerbExecute.h"

#ifdef SQUNICODE
#define scvprintf vfwprintf
#else
#define scvprintf vfprintf
#endif

namespace ng
{
template <>
void ScriptEngine::pushValue(bool value)
{
    sq_pushbool(v, value ? SQTrue : SQFalse);
}

template <>
void ScriptEngine::pushValue(int value)
{
    sq_pushinteger(v, value);
}

template <>
void ScriptEngine::pushValue(const char *value)
{
    sq_pushstring(v, value, -1);
}

template <>
void ScriptEngine::pushValue(SQFloat value)
{
    sq_pushfloat(v, value);
}

template <typename TConstant>
void ScriptEngine::registerConstant(const SQChar *name, TConstant value)
{
    sq_pushconsttable(v);
    sq_pushstring(v, name, -1);
    pushValue(value);
    sq_newslot(v, -3, SQTrue);
    sq_pop(v, 1);
}

template <typename TConstant>
void ScriptEngine::registerConstants(std::initializer_list<std::tuple<const SQChar *, TConstant>> list)
{
    for (auto t : list)
    {
        sq_pushconsttable(v);
        sq_pushstring(v, std::get<0>(t), -1);
        pushValue(std::get<1>(t));
        sq_newslot(v, -3, SQTrue);
        sq_pop(v, 1);
    }
}

template <typename TEntity>
TEntity *ScriptEngine::getEntity(HSQUIRRELVM v, SQInteger index)
{
    auto type = sq_gettype(v, index);
    // is it a table?
    if (type != OT_TABLE)
    {
        sq_pushbool(v, SQFalse);
        return nullptr;
    }

    HSQOBJECT object;
    sq_resetobject(&object);
    if (SQ_FAILED(sq_getstackobj(v, index, &object)))
    {
        return nullptr;
    }

    sq_pushobject(v, object);
    sq_pushstring(v, _SC("instance"), -1);
    if (SQ_FAILED(sq_get(v, -2)))
    {
        return nullptr;
    }

    Entity *pObj = nullptr;
    if (SQ_FAILED(sq_getuserpointer(v, -1, (SQUserPointer *)&pObj)))
    {
        return nullptr;
    }

    return dynamic_cast<TEntity *>(pObj);
}

template <class T>
void ScriptEngine::pushObject(HSQUIRRELVM v, T &object)
{
    sq_newtable(v);
    sq_pushstring(v, _SC("instance"), -1);
    sq_pushuserpointer(v, &object);
    sq_newslot(v, -3, SQFalse);
}

template <typename TPack>
void ScriptEngine::addPack()
{
    auto pack = std::make_unique<TPack>();
    auto pPack = (Pack *)pack.get();
    pPack->addTo(*this);
    _packs.push_back(std::move(pack));
}

ScriptEngine::ScriptEngine(Engine &engine)
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
    registerConstants<bool>({{"NO", false},
                             {"YES", true}});
    registerConstants<const char *>({{"VERB_CLOSE", "close"},
                                     {"VERB_GIVE", "give"},
                                     {"VERB_LOOKAT", "lookat"},
                                     {"VERB_OPEN", "open"},
                                     {"VERB_PICKUP", "pickup"},
                                     {"VERB_PULL", "pull"},
                                     {"VERB_PUSH", "push"},
                                     {"VERB_TALKTO", "talkto"},
                                     {"VERB_USE", "use"},
                                     {"VERB_WALKTO", "walkto"}});
    registerConstants<int>({{"HERE", 0},
                            {"GONE", 4},
                            {"OFF", 0},
                            {"ON", 1},
                            {"FULL", 0},
                            {"EMPTY", 1},
                            {"OPEN", 1},
                            {"CLOSED", 0},
                            {"FULL", 0},
                            {"EMPTY", 1},
                            {"FADE_IN", 0},
                            {"FADE_OUT", 1},
                            {"FACE_FRONT", 0},
                            {"FACE_BACK", 1},
                            {"FACE_LEFT", 2},
                            {"FACE_RIGHT", 3},
                            {"DIR_FRONT", 0},
                            {"DIR_BACK", 1},
                            {"DIR_LEFT", 2},
                            {"DIR_RIGHT", 3},
                            {"LINEAR", 0},
                            {"EASE_IN", 1},
                            {"EASE_INOUT", 2},
                            {"EASE_OUT", 3},
                            {"SLOW_EASE_IN", 4},
                            {"SLOW_EASE_OUT", 5},
                            {"LOOPING", 6},
                            {"USE_WITH", 2},
                            {"USE_ON", 4},
                            {"USE_IN", 8},
                            {"ALIGN_LEFT",   0x10000000},
                            {"ALIGN_CENTER", 0x20000000},
                            {"ALIGN_RIGHT",  0x40000000},
                            {"ALIGN_TOP",    0x80000000},
                            {"ALIGN_BOTTOM", 0x1000000},
                            });

    addPack<_ActorPack>();
    addPack<_GeneralPack>();
    addPack<_ObjectPack>();
    addPack<_RoomPack>();
    addPack<_SoundPack>();
    addPack<_SystemPack>();

    auto pVerbExecute = std::make_unique<_DefaultVerbExecute>(v, engine);
    engine.setVerbExecute(std::move(pVerbExecute));
    auto pScriptExecute = std::make_unique<_DefaultScriptExecute>(v);
    engine.setScriptExecute(std::move(pScriptExecute));
}

ScriptEngine::~ScriptEngine()
{
    sq_pop(v, 1);
    sq_close(v);
}

Engine &ScriptEngine::getEngine()
{
    return _engine;
}

Object *ScriptEngine::getObject(HSQUIRRELVM v, SQInteger index)
{
    return ScriptEngine::getEntity<Object>(v, index);
}

Room *ScriptEngine::getRoom(HSQUIRRELVM v, SQInteger index)
{
    return ScriptEngine::getEntity<Room>(v, index);
}

Actor *ScriptEngine::getActor(HSQUIRRELVM v, SQInteger index)
{
    return ScriptEngine::getEntity<Actor>(v, index);
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
    case InterpolationMethod::SlowEaseIn:
    case InterpolationMethod::EaseIn:
        return Interpolations::easeIn;
    case InterpolationMethod::EaseInOut:
        return Interpolations::easeInOut;
    case InterpolationMethod::SlowEaseOut:
    case InterpolationMethod::EaseOut:
        return Interpolations::easeOut;
    default:
        return Interpolations::linear;
    }
}
} // namespace ng
