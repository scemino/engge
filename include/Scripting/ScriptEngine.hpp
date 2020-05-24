#pragma once
#include <functional>
#include <string>
#include <assert.h>
#include "../../extlibs/squirrel/squirrel/sqobject.h"
#include "sqstdio.h"
#include "sqstdaux.h"
#include "Engine/Engine.hpp"
#include "Engine/Interpolations.hpp"
#include "System/Logger.hpp"
#include "Room/Room.hpp"

namespace ng {
class Entity;
class Light;
class Room;
class ScriptEngine;
class Sound;
class SoundId;
class Thread;
class Pack {
public:
  virtual void addTo(ScriptEngine &engine) const = 0;
  virtual ~Pack() = default;
};
class ScriptEngine {
public:
  using ErrorCallback = std::function<void(HSQUIRRELVM, const SQChar *, const SQChar *, SQInteger, SQInteger)>;
  using PrintCallback = std::function<void(HSQUIRRELVM v, const SQChar *s)>;

public:
  explicit ScriptEngine();
  ~ScriptEngine();

  void setEngine(Engine &engine);
  Engine &getEngine();

  static HSQUIRRELVM getVm() { return v; }

  static SQObjectPtr toSquirrel(const std::string &value);

  template<typename TConstant>
  void registerConstants(std::initializer_list<std::tuple<const SQChar *, TConstant>> list);
  void registerGlobalFunction(SQFUNCTION f,
                              const SQChar *functionName,
                              SQInteger nparamscheck = 0,
                              const SQChar *typemask = nullptr);
  static void executeScript(const std::string &name);
  static void executeNutScript(const std::string &name);

  template<class TPack>
  void addPack();

  template<typename TScriptObject>
  static TScriptObject *getScriptObject(HSQUIRRELVM v, SQInteger index);
  template<typename TScriptObject>
  static TScriptObject *getScriptObjectFromId(int id);

  static Entity *getEntity(HSQUIRRELVM v, SQInteger index);
  static Object *getObject(HSQUIRRELVM v, SQInteger index);
  static Room *getRoom(HSQUIRRELVM v, SQInteger index);
  static Actor *getActor(HSQUIRRELVM v, SQInteger index);
  static SoundId *getSound(HSQUIRRELVM v, SQInteger index);
  static SoundDefinition *getSoundDefinition(HSQUIRRELVM v, SQInteger index);

  static Actor *getActorFromId(int id);
  static Room *getRoomFromId(int id);
  static Object *getObjectFromId(int id);
  static Sound *getSoundFromId(int id);
  static ThreadBase *getThreadFromId(int id);
  static ThreadBase *getThreadFromVm(HSQUIRRELVM v);

  static bool tryGetLight(HSQUIRRELVM v, SQInteger index, Light *&light);

  template<class T>
  static void pushObject(HSQUIRRELVM v, T *pObject);

  template<typename T>
  static void push(HSQUIRRELVM v, T value);
  template<typename First, typename... Rest>
  static void push(HSQUIRRELVM v, First firstValue, Rest... rest);

  template<typename TThis>
  static bool exists(TThis pThis, const char *name);

  template<typename T>
  static bool get(HSQUIRRELVM v, SQInteger index, T &result);
  template<typename TThis, typename T>
  static bool get(TThis pThis, const char *name, T &result);
  template<typename TThis, typename T>
  static bool get(HSQUIRRELVM v, TThis pThis, const char *name, T &result);

  template<typename T>
  static void set(const char *name, T value);

  template<typename TThis, typename T>
  static void set(TThis pThis, const char *name, T value);
  template<typename TThis, typename T>
  static void set(HSQUIRRELVM v, TThis pThis, const char *name, T value);

  template<typename...T>
  static bool call(const char *name, T... args);
  static bool call(const char *name);

  template<typename TThis, typename...T>
  static bool objCall(TThis pThis, const char *name, T... args);
  template<typename TThis>
  static bool objCall(TThis pThis, const char *name);
  template<typename TThis>
  static int getParameterCount(TThis pThis, const char *name);

  template<typename TResult, typename TThis, typename...T>
  static bool callFunc(TResult &result, TThis pThis, const char *name, T... args);

  template<typename TThis>
  static bool rawExists(TThis pThis, const char *name);

  template<typename TThis, typename T>
  static bool rawGet(TThis pThis, const char *name, T &result);
  template<typename TThis, typename T>
  static bool rawGet(HSQUIRRELVM v, TThis pThis, const char *name, T &result);

  template<typename...T>
  static bool rawCall(const char *name, T... args);
  static bool rawCall(const char *name);

  template<typename TThis, typename...T>
  static bool rawCall(TThis pThis, const char *name, T... args);
  template<typename TThis>
  static bool rawCall(TThis pThis, const char *name);
  template<typename TThis>
  static int rawGetParameterCount(TThis pThis, const char *name);

  template<typename TResult, typename TThis, typename...T>
  static bool rawCallFunc(TResult &result, TThis pThis, const char *name, T... args);

  static void registerErrorCallback(PrintCallback callback) {
    _errorCallbacks.push_back(callback);
  }

  static void registerPrintCallback(PrintCallback callback) {
    _printCallbacks.push_back(callback);
  }

  static void printfunc(HSQUIRRELVM v, const SQChar *s, ...);

private:
  static SQInteger aux_printerror(HSQUIRRELVM v);
  static void errorHandler(HSQUIRRELVM v, const SQChar *desc, const SQChar *source, SQInteger line, SQInteger column);

  static void errorfunc(HSQUIRRELVM v, const SQChar *s, ...);

private:
  static HSQUIRRELVM v;
  std::vector<std::unique_ptr<Pack>> _packs;
  inline static std::vector<PrintCallback> _errorCallbacks;
  inline static std::vector<PrintCallback> _printCallbacks;

private:
  static Engine *g_pEngine;
};

template<typename First, typename... Rest>
void ScriptEngine::push(HSQUIRRELVM v, First firstValue, Rest... rest) {
  ScriptEngine::push(v, firstValue);
  ScriptEngine::push(v, rest...);
}

template<typename...T>
bool ScriptEngine::call(const char *name, T...args) {
  constexpr std::size_t n = sizeof...(T);
  auto v = ScriptEngine::getVm();
  auto top = sq_gettop(v);
  sq_pushroottable(v);
  sq_pushstring(v, _SC(name), -1);
  if (SQ_FAILED(sq_get(v, -2))) {
    sq_settop(v, top);
    trace("can't find {} function", name);
    return false;
  }
  sq_remove(v, -2);

  sq_pushroottable(v);
  ScriptEngine::push(v, std::forward<T>(args)...);
  if (SQ_FAILED(sq_call(v, n + 1, SQFalse, SQTrue))) {
    sqstd_printcallstack(v);
    sq_settop(v, top);
    error("function {} call failed", name);
    return false;
  }
  sq_settop(v, top);
  return true;
}

template<typename TThis>
int ScriptEngine::getParameterCount(TThis pThis, const char *name) {
  auto v = ScriptEngine::getVm();
  auto top = sq_gettop(v);
  ScriptEngine::push(v, pThis);
  sq_pushstring(v, name, -1);
  if (SQ_FAILED(sq_get(v, -2))) {
    sq_settop(v, top);
    trace("can't find {} function", name);
    return 0;
  }

  SQInteger nparams, nfreevars;
  sq_getclosureinfo(v, -1, &nparams, &nfreevars);
  trace("{} function found with {} parameters", name, nparams);
  sq_settop(v, top);

  return nparams;
}

template<typename...T>
bool ScriptEngine::rawCall(const char *name, T...args) {
  constexpr std::size_t n = sizeof...(T);
  auto v = ScriptEngine::getVm();
  auto top = sq_gettop(v);
  sq_pushroottable(v);
  sq_pushstring(v, _SC(name), -1);
  if (SQ_FAILED(sq_rawget(v, -2))) {
    sq_settop(v, top);
    trace("can't find {} function", name);
    return false;
  }
  sq_remove(v, -2);

  sq_pushroottable(v);
  ScriptEngine::push(v, std::forward<T>(args)...);
  if (SQ_FAILED(sq_call(v, n + 1, SQFalse, SQTrue))) {
    sqstd_printcallstack(v);
    sq_settop(v, top);
    error("function {} call failed", name);
    return false;
  }
  sq_settop(v, top);
  return true;
}

template<typename TThis>
int ScriptEngine::rawGetParameterCount(TThis pThis, const char *name) {
  auto v = ScriptEngine::getVm();
  auto top = sq_gettop(v);
  ScriptEngine::push(v, pThis);
  sq_pushstring(v, name, -1);
  if (SQ_FAILED(sq_rawget(v, -2))) {
    sq_settop(v, top);
    trace("can't find {} function", name);
    return 0;
  }

  SQInteger nparams, nfreevars;
  sq_getclosureinfo(v, -1, &nparams, &nfreevars);
  trace("{} function found with {} parameters", name, nparams);
  sq_settop(v, top);

  return nparams;
}

template<typename TThis, typename...T>
bool ScriptEngine::objCall(TThis pThis, const char *name, T... args) {
  constexpr std::size_t n = sizeof...(T);
  auto v = ScriptEngine::getVm();
  auto top = sq_gettop(v);
  ScriptEngine::push(v, pThis);
  sq_pushstring(v, _SC(name), -1);
  if (SQ_FAILED(sq_get(v, -2))) {
    sq_settop(v, top);
    trace("can't find {} function", name);
    return false;
  }
  sq_remove(v, -2);

  ScriptEngine::push(v, pThis);
  ScriptEngine::push(v, std::forward<T>(args)...);
  if (SQ_FAILED(sq_call(v, n + 1, SQFalse, SQTrue))) {
    sqstd_printcallstack(v);
    sq_settop(v, top);
    error("function {} call failed", name);
    return false;
  }
  sq_settop(v, top);
  return true;
}

template<typename TThis, typename...T>
bool ScriptEngine::rawCall(TThis pThis, const char *name, T... args) {
  constexpr std::size_t n = sizeof...(T);
  auto v = ScriptEngine::getVm();
  auto top = sq_gettop(v);
  ScriptEngine::push(v, pThis);
  sq_pushstring(v, _SC(name), -1);
  if (SQ_FAILED(sq_rawget(v, -2))) {
    sq_settop(v, top);
    trace("can't find {} function", name);
    return false;
  }
  sq_remove(v, -2);

  ScriptEngine::push(v, pThis);
  ScriptEngine::push(v, std::forward<T>(args)...);
  if (SQ_FAILED(sq_call(v, n + 1, SQFalse, SQTrue))) {
    sqstd_printcallstack(v);
    sq_settop(v, top);
    error("function {} call failed", name);
    return false;
  }
  sq_settop(v, top);
  return true;
}

template<typename TThis>
bool ScriptEngine::objCall(TThis pThis, const char *name) {
  auto v = ScriptEngine::getVm();
  auto top = sq_gettop(v);
  ScriptEngine::push(v, pThis);
  sq_pushstring(v, _SC(name), -1);
  if (SQ_FAILED(sq_get(v, -2))) {
    sq_settop(v, top);
    trace("can't find {} function", name);
    return false;
  }
  sq_remove(v, -2);

  ScriptEngine::push(v, pThis);
  if (SQ_FAILED(sq_call(v, 1, SQFalse, SQTrue))) {
    sqstd_printcallstack(v);
    sq_settop(v, top);
    error("function {} call failed", name);
    return false;
  }
  sq_settop(v, top);
  return true;
}

template<typename TThis>
bool ScriptEngine::rawCall(TThis pThis, const char *name) {
  auto v = ScriptEngine::getVm();
  auto top = sq_gettop(v);
  ScriptEngine::push(v, pThis);
  sq_pushstring(v, _SC(name), -1);
  if (SQ_FAILED(sq_rawget(v, -2))) {
    sq_settop(v, top);
    trace("can't find {} function", name);
    return false;
  }
  sq_remove(v, -2);

  ScriptEngine::push(v, pThis);
  if (SQ_FAILED(sq_call(v, 1, SQFalse, SQTrue))) {
    sqstd_printcallstack(v);
    sq_settop(v, top);
    error("function {} call failed", name);
    return false;
  }
  sq_settop(v, top);
  return true;
}

template<typename TResult, typename TThis, typename...T>
bool ScriptEngine::callFunc(TResult &result, TThis pThis, const char *name, T... args) {
  constexpr std::size_t n = sizeof...(T);
  auto v = ScriptEngine::getVm();
  auto top = sq_gettop(v);
  ScriptEngine::push(v, pThis);
  sq_pushstring(v, _SC(name), -1);
  if (SQ_FAILED(sq_get(v, -2))) {
    sq_settop(v, top);
    trace("can't find {} function", name);
    return false;
  }
  sq_remove(v, -2);

  ScriptEngine::push(v, pThis);
  ScriptEngine::push(v, std::forward<T>(args)...);
  if (SQ_FAILED(sq_call(v, n + 1, SQTrue, SQTrue))) {
    sqstd_printcallstack(v);
    sq_settop(v, top);
    error("function {} call failed", name);
    return false;
  }
  ScriptEngine::get(v, -1, result);
  sq_settop(v, top);
  return true;
}

template<typename TResult, typename TThis, typename...T>
bool ScriptEngine::rawCallFunc(TResult &result, TThis pThis, const char *name, T... args) {
  constexpr std::size_t n = sizeof...(T);
  auto v = ScriptEngine::getVm();
  auto top = sq_gettop(v);
  ScriptEngine::push(v, pThis);
  sq_pushstring(v, _SC(name), -1);
  if (SQ_FAILED(sq_rawget(v, -2))) {
    sq_settop(v, top);
    trace("can't find {} function", name);
    return false;
  }

  ScriptEngine::push(v, pThis);
  ScriptEngine::push(v, std::forward<T>(args)...);
  if (SQ_FAILED(sq_call(v, n + 1, SQTrue, SQTrue))) {
    sqstd_printcallstack(v);
    sq_settop(v, top);
    error("function {} call failed", name);
    return false;
  }
  ScriptEngine::get(v, -1, result);
  sq_settop(v, top);
  return true;
}

template<typename TThis, typename T>
bool ScriptEngine::get(TThis pThis, const char *name, T &result) {
  return ScriptEngine::get(ScriptEngine::getVm(), pThis, name, result);
}

template<typename TThis, typename T>
bool ScriptEngine::rawGet(TThis pThis, const char *name, T &result) {
  return ScriptEngine::rawGet(ScriptEngine::getVm(), pThis, name, result);
}

template<typename TThis, typename T>
bool ScriptEngine::get(HSQUIRRELVM v, TThis pThis, const char *name, T &result) {
  auto top = sq_gettop(v);
  push(v, pThis);
  sq_pushstring(v, _SC(name), -1);
  if (SQ_SUCCEEDED(sq_get(v, -2))) {
    auto status = ScriptEngine::get(v, -1, result);
    sq_settop(v, top);
    return status;
  }
  sq_settop(v, top);
  return false;
}

template<typename TThis, typename T>
bool ScriptEngine::rawGet(HSQUIRRELVM v, TThis pThis, const char *name, T &result) {
  auto top = sq_gettop(v);
  push(v, pThis);
  sq_pushstring(v, _SC(name), -1);
  if (SQ_SUCCEEDED(sq_rawget(v, -2))) {
    auto status = ScriptEngine::get(v, -1, result);
    sq_settop(v, top);
    return status;
  }
  sq_settop(v, top);
  return false;
}

template<typename TThis>
bool ScriptEngine::exists(TThis pThis, const char *name) {
  auto v = ScriptEngine::getVm();
  auto top = sq_gettop(v);
  push(v, pThis);
  sq_pushstring(v, _SC(name), -1);
  if (SQ_SUCCEEDED(sq_get(v, -2))) {
    sq_settop(v, top);
    return true;
  }
  sq_settop(v, top);
  return false;
}

template<typename TThis>
bool ScriptEngine::rawExists(TThis pThis, const char *name) {
  auto v = ScriptEngine::getVm();
  auto top = sq_gettop(v);
  push(v, pThis);
  sq_pushstring(v, _SC(name), -1);
  if (SQ_SUCCEEDED(sq_rawget(v, -2))) {
    sq_settop(v, top);
    return true;
  }
  sq_settop(v, top);
  return false;
}

template<typename T>
void ScriptEngine::set(const char *name, T value) {
  auto v = ScriptEngine::getVm();
  sq_pushroottable(v);
  sq_pushstring(v, _SC(name), -1);
  ScriptEngine::push(v, value);
  sq_newslot(v, -3, SQFalse);
  sq_pop(v, 1);
}

template<typename TThis, typename T>
void ScriptEngine::set(TThis pThis, const char *name, T value) {
  ScriptEngine::set(ScriptEngine::getVm(), pThis, name, value);
}

template<typename TThis, typename T>
void ScriptEngine::set(HSQUIRRELVM v, TThis pThis, const char *name, T value) {
  push(v, pThis);
  sq_pushstring(v, _SC(name), -1);
  ScriptEngine::push(v, value);
  sq_newslot(v, -3, SQFalse);
  sq_pop(v, 1);
}

} // namespace ng