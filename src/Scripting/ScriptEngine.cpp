#include <stdarg.h>
#include "squirrel.h"
#include "sqstdio.h"
#include "sqstdaux.h"
#include "sqstdstring.h"
#include "sqstdmath.h"
#include "Entity.h"
#include "Logger.h"
#include "Room.h"
#include "ScriptEngine.h"
#include "SoundDefinition.h"
#include "VerbExecute.h"
#include "_SystemPack.h"
#include "_GeneralPack.h"
#include "_ObjectPack.h"
#include "_ActorPack.h"
#include "_RoomPack.h"
#include "_SoundPack.h"
#include "_DefaultVerbExecute.h"
#include "_bnutPass.h"

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

template <typename TScriptObject>
TScriptObject *ScriptEngine::getScriptObject(HSQUIRRELVM v, SQInteger index)
{
    auto type = sq_gettype(v, index);
    // is it a table?
    if (type != OT_TABLE)
    {
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

    ScriptObject *pObj = nullptr;
    if (SQ_FAILED(sq_getuserpointer(v, -1, (SQUserPointer *)&pObj)))
    {
        return nullptr;
    }
    sq_pop(v, 2);

    return dynamic_cast<TScriptObject *>(pObj);
}

Entity *ScriptEngine::getEntity(HSQUIRRELVM v, SQInteger index)
{
    return ScriptEngine::getScriptObject<Entity>(v, index);
}

Object *ScriptEngine::getObject(HSQUIRRELVM v, SQInteger index)
{
    return ScriptEngine::getScriptObject<Object>(v, index);
}

Room *ScriptEngine::getRoom(HSQUIRRELVM v, SQInteger index)
{
    return ScriptEngine::getScriptObject<Room>(v, index);
}

Actor *ScriptEngine::getActor(HSQUIRRELVM v, SQInteger index)
{
    return ScriptEngine::getScriptObject<Actor>(v, index);
}

Light *ScriptEngine::getLight(HSQUIRRELVM v, SQInteger index)
{
    return ScriptEngine::getScriptObject<Light>(v, index);
}

bool ScriptEngine::tryGetLight(HSQUIRRELVM v, SQInteger index, Light*& light)
{
    HSQOBJECT obj;
    light = nullptr;
    if(SQ_SUCCEEDED(sq_getstackobj(v, index, &obj)) && sq_isinteger(obj) && sq_objtointeger(&obj) == 0)
    {
        return false;
    }
    light = ScriptEngine::getScriptObject<Light>(v, index);
    return true;
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

enum class Platform : int
{
    Mac = 1,
    Win = 2,
    Linux = 3,
    Xbox = 4,
    IOS = 5,
    Android = 6,
    Switch = 7,
    PS4 = 8,
};

static Platform _getPlatform()
{
#ifdef __APPLE__
#ifdef TARGET_OS_MAC
    return Platform::Mac;
#elif TARGET_OS_IPHONE
    return Platform::IOS;
#endif
#elif _WIN32
    return Platform::Win;
#elif defined __linux__ && !defined __ANDROID__
    return Platform::Linux;
// TODO: XBOX
//     return Platform::Xbox;
#elif __ANDROID__
    return Platform::Android;
// TODO: SWITCH
//    return Platform::Switch;
// TODO: PS4__
// TODO: return Platform::PS4;
#endif
}

ScriptEngine::ScriptEngine()
{
    v = sq_open(1024 * 2);
    sq_setcompilererrorhandler(v, errorHandler);
    sq_newclosure(v, aux_printerror, 0);
    sq_seterrorhandler(v);
    sq_setprintfunc(v, printfunc, errorfunc); //sets the print function

    sq_pushroottable(v);
    sqstd_register_mathlib(v);
    sqstd_register_stringlib(v);
    sq_pushstring(v, _SC("PLATFORM"), -1);
    sq_pushinteger(v, (SQInteger)_getPlatform());
    sq_newslot(v, -3, SQFalse);
    registerConstants<int>({
        {"HERE", 0},
        {"GONE", 4},
        {"OFF", 0},
        {"ON", 1},
        {"FALSE", 0},
        {"TRUE", 1},
        {"MOUSE", 1},
        {"CONTROLLER", 2},
        {"DIRECTDRIVE", 3},
        {"TOUCH", 4},
        {"REMOTE", 5},
        {"FULL", 0},
        {"EMPTY", 1},
        {"OPEN", 1},
        {"CLOSED", 0},
        {"FULL", 0},
        {"EMPTY", 1},
        {"FADE_IN", 0},
        {"FADE_OUT", 1},
        {"FADE_WOBBLE", 2},
        {"FADE_WOBBLE_TO_SEPIA", 3},
        {"FACE_FRONT", 0x4},
        {"FACE_BACK", 0x8},
        {"FACE_LEFT", 0x2},
        {"FACE_RIGHT", 0x1},
        {"FACE_FLIP", 0x10},
        {"DIR_FRONT", 0x4},
        {"DIR_BACK", 0x8},
        {"DIR_LEFT", 0x2},
        {"DIR_RIGHT", 0x1},
        {"LINEAR", 0},
        {"EASE_IN", 1},
        {"EASE_INOUT", 2},
        {"EASE_OUT", 3},
        {"SLOW_EASE_IN", 4},
        {"SLOW_EASE_OUT", 5},
        {"SWING", 0X200},
        {"LOOPING", 6},
        {"USE_WITH", 2},
        {"USE_ON", 4},
        {"USE_IN", 8},
        {"ALIGN_LEFT", 0x10000000},
        {"ALIGN_CENTER", 0x20000000},
        {"ALIGN_RIGHT", 0x40000000},
        {"ALIGN_TOP", 0x80000000},
        {"ALIGN_BOTTOM", 0x1000000},
        {"REACH_HIGH", 0x8000},
        {"REACH_MED", 0x10000},
        {"REACH_LOW", 0x20000},
        {"REACH_NONE", 0x40000},
        {"EX_ALLOW_SAVEGAMES", 0x01},
        {"EX_POP_CHARACTER_SELECTION", 0x02},
        {"EX_CAMERA_TRACKING", 0x03},
        {"EX_BUTTON_HOVER_SOUND", 0x04},
        {"EX_RESTART", 0x06},
        {"EX_IDLE_TIME", 0x07},
        {"EX_AUTOSAVE", 0x08},
        {"EX_AUTOSAVE_STATE", 0x09},
        {"EX_DISABLE_SAVESYSTEM", 0x0A},
        {"EX_SHOW_OPTIONS", 0x0B},
        {"EX_OPTIONS_MUSIC", 0x0C},
        {"GRASS_BACKANDFORTH", 0x00},
        {"EFFECT_NONE", 0x00},
        {"DOOR", 0x40},
        {"DOOR_LEFT", 0x140},
        {"DOOR_RIGHT", 0x240},
        {"DOOR_BACK", 0x440},
        {"DOOR_FRONT", 0x840},
        {"FAR_LOOK", 0x8},
        {"GIVEABLE", 0x1000},
        {"TALKABLE", 0x2000},
        {"IMMEDIATE", 0x4000},
        {"FEMALE", 0x8000},
        {"MALE", 0x100000},
        {"PERSON", 0x200000},
        {"VERB_CLOSE", 6},
        {"VERB_GIVE", 9},
        {"VERB_LOOKAT", 2},
        {"VERB_OPEN", 5},
        {"VERB_PICKUP", 4},
        {"VERB_PULL", 8},
        {"VERB_PUSH", 7},
        {"VERB_TALKTO", 3},
        {"VERB_USE", 10},
        {"VERB_WALKTO", 1},
        {"VERBFLAG_INSTANT", 1},
        {"NO", 0},
        {"YES", 1},
        {"UNSELECTABLE", 0},
        {"SELECTABLE", 1},
        {"TEMP_UNSELECTABLE", 2},
        {"TEMP_SELECTABLE", 3},
        {"MAC", 1},
        {"WIN", 2},
        {"LINUX", 3},
        {"XBOX", 4},
        {"IOS", 5},
        {"ANDROID", 6},
        {"SWITCH", 7},
        {"PS4", 8},
        {"EFFECT_NONE", 0},
        {"EFFECT_SEPIA", 1},
        {"EFFECT_EGA", 2},
        {"EFFECT_VHS", 3},
        {"EFFECT_GHOST", 4},
        {"EFFECT_BLACKANDWHITE", 5},
        {"KEY_UP", 0x40000052},
        {"KEY_RIGHT", 0x4000004F},
        {"KEY_DOWN", 0x40000051},
        {"KEY_LEFT", 0x40000050},
        {"KEY_PAD1", 0x40000059},
        {"KEY_PAD2", 0x4000005A},
        {"KEY_PAD3", 0x4000005B},
        {"KEY_PAD4", 0x4000005C},
        {"KEY_PAD5", 0x4000005D},
        {"KEY_PAD6", 0x4000005E},
        {"KEY_PAD7", 0x4000005F},
        {"KEY_PAD8", 0x40000056},
        {"KEY_PAD9", 0x40000061},
    });
}

ScriptEngine::~ScriptEngine()
{
    sq_close(v);
}

void ScriptEngine::setEngine(Engine &engine)
{
    _pEngine = &engine;
    engine.setVm(v);
    auto pVerbExecute = std::make_unique<_DefaultVerbExecute>(v, engine);
    engine.setVerbExecute(std::move(pVerbExecute));
    auto pScriptExecute = std::make_unique<_DefaultScriptExecute>(v);
    engine.setScriptExecute(std::move(pScriptExecute));

    addPack<_ActorPack>();
    addPack<_GeneralPack>();
    addPack<_ObjectPack>();
    addPack<_RoomPack>();
    addPack<_SoundPack>();
    addPack<_SystemPack>();
}

Engine &ScriptEngine::getEngine()
{
    return *_pEngine;
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
    error("{} {}({},{})", desc, source, line, column);
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
        error("failed to execute {}", name);
        sq_getlasterror(v);
        aux_printerror(v);
        return;
    }
}

void ScriptEngine::executeNutScript(const std::string &name)
{
    std::vector<char> code;

    std::ifstream is(name);
    if (is.is_open())
    {
        trace("Load local file file {}", name);
        is.seekg(-1, std::ios::end);
        auto len = (size_t)is.tellg();
        is.seekg(0, std::ios::beg);
        code.resize(len + 1);
        is.read(code.data(), len);
    }
    else
    {
        auto entryName = std::regex_replace(name, std::regex("\\.nut"), ".bnut");
        _pEngine->getSettings().readEntry(entryName, code);

        // decode bnut
        int cursor = code.size() & 0xff;
        for (char &i : code)
        {
            i ^= _bnutPass[cursor];
            cursor = (cursor + 1) % 4096;
        }
    }

#if 0
    std::ofstream o;
    o.open(name);
    o.write(code.data(), code.size());
    o.close();
#endif
    auto top = sq_gettop(v);
    sq_pushroottable(v);
    if (SQ_FAILED(sq_compilebuffer(v, code.data(), code.size() - 1, _SC(name.data()), SQTrue)))
    {
        error("Error compiling {}", name);
        return;
    }
    sq_push(v, -2);
    // call
    if (SQ_FAILED(sq_call(v, 1, SQFalse, SQTrue)))
    {
        error("Error calling {}", name);
        sqstd_printcallstack(v);
        return;
    }
    sq_settop(v, top);
}

void ScriptEngine::executeBootScript()
{
    executeNutScript("Defines.nut");
    executeNutScript("Boot.nut");

    // call start
    sq_pushroottable(v);
    sq_pushstring(v, _SC("start"), -1);
    sq_get(v, -2);

    sq_pushroottable(v);
    sq_pushbool(v, SQTrue);
    if (SQ_FAILED(sq_call(v, 2, SQFalse, SQTrue)))
    {
        error("Error calling start");
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
    case InterpolationMethod::EaseOutCubic:
        return Interpolations::easeOutCubic;
    default:
        return Interpolations::linear;
    }
}
} // namespace ng
