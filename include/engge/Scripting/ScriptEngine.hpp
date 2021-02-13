#pragma once
#include <cassert>
#include <functional>
#include <string>
#include <squirrel.h>
#include "../../../extlibs/squirrel/squirrel/sqobject.h"
#include "engge/Engine/Engine.hpp"
#include "engge/Engine/Interpolations.hpp"
#include "engge/System/Logger.hpp"
#include <sqstdaux.h>
#include <sqstdio.h>

namespace ng {
class Pack {
public:
  virtual void registerPack() const = 0;
  virtual ~Pack() = default;
};
class ScriptEngine {
public:
  using ErrorCallback = std::function<void(
      HSQUIRRELVM, const SQChar *, const SQChar *, SQInteger, SQInteger)>;
  using PrintCallback = std::function<void(HSQUIRRELVM v, const SQChar *s)>;

public:
  explicit ScriptEngine();
  ~ScriptEngine();

  void setEngine(Engine &engine);
  static Engine &getEngine();

  static HSQUIRRELVM getVm() { return m_vm; }

  static SQObjectPtr toSquirrel(const std::string &value);

  template <typename TConstant>
  void registerConstants(
      std::initializer_list<std::tuple<const SQChar *, TConstant>> list);
  static void registerGlobalFunction(SQFUNCTION f, const SQChar *functionName,
                                     SQInteger nparamscheck = 0,
                                     const SQChar *typemask = nullptr);
  static void executeScript(const std::string &name);
  static void executeNutScript(const std::string &name);

  template <class TPack> void registerPack();

  template <class T> static void pushObject(HSQUIRRELVM v, T *pObject);

  template <typename T> static void push(HSQUIRRELVM v, T value);
  template <typename First, typename... Rest>
  static void push(HSQUIRRELVM v, First firstValue, Rest... rest);

  template <typename TThis> static bool exists(TThis pThis, const char *name);

  template <typename T>
  static bool get(SQInteger index, T &result);
  template <typename T> static bool get(const char *name, T &result);
  template <typename TThis, typename T>
  static bool get(TThis pThis, const char *name, T &result);

  template <typename T> static void set(const char *name, T value);

  template <typename TThis, typename T>
  static void set(TThis pThis, const char *name, T value);

  template <typename... T> static bool call(const char *name, T... args);
  static bool call(const char *name);

  template <typename TThis, typename... T>
  static bool objCall(TThis pThis, const char *name, T... args);
  template <typename TThis> static bool objCall(TThis pThis, const char *name);
  template <typename TThis>
  static int getParameterCount(TThis pThis, const char *name);

  template <typename TResult, typename TThis, typename... T>
  static bool callFunc(TResult &result, TThis pThis, const char *name,
                       T... args);

  template <typename TResult, typename... T>
  static bool callFunc(TResult &result, const char *name, T... args);

  template <typename TThis>
  static bool rawExists(TThis pThis, const char *name);

  template <typename TThis, typename T>
  static bool rawGet(TThis pThis, const char *name, T &result);

  template <typename... T> static bool rawCall(const char *name, T... args);
  static bool rawCall(const char *name);

  template <typename TThis, typename... T>
  static bool rawCall(TThis pThis, const char *name, T... args);
  template <typename TThis> static bool rawCall(TThis pThis, const char *name);

  template <typename TResult, typename TThis, typename... T>
  static bool rawCallFunc(TResult &result, TThis pThis, const char *name,
                          T... args);

  static void registerErrorCallback(const PrintCallback &callback) {
    m_errorCallbacks.push_back(callback);
  }

  static void registerPrintCallback(const PrintCallback &callback) {
    m_printCallbacks.push_back(callback);
  }

  static void printfunc(HSQUIRRELVM v, const SQChar *s, ...);

private:
  static SQInteger aux_printerror(HSQUIRRELVM v);
  static void errorHandler(HSQUIRRELVM v, const SQChar *desc,
                           const SQChar *source, SQInteger line,
                           SQInteger column);

  static void errorfunc(HSQUIRRELVM v, const SQChar *s, ...);

private:
  inline static HSQUIRRELVM m_vm{};
  std::vector<std::unique_ptr<Pack>> m_packs;
  inline static std::vector<PrintCallback> m_errorCallbacks;
  inline static std::vector<PrintCallback> m_printCallbacks;

private:
  static Engine *g_pEngine;
};

template <typename First, typename... Rest>
void ScriptEngine::push(HSQUIRRELVM v, First firstValue, Rest... rest) {
  ScriptEngine::push(v, firstValue);
  ScriptEngine::push(v, rest...);
}

template <typename... T> bool ScriptEngine::call(const char *name, T... args) {
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

template <typename TThis>
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

template <typename... T>
bool ScriptEngine::rawCall(const char *name, T... args) {
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

template <typename TThis, typename... T>
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

template <typename TThis, typename... T>
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

template <typename TThis>
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

template <typename TThis>
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

template <typename TResult, typename TThis, typename... T>
bool ScriptEngine::callFunc(TResult &result, TThis pThis, const char *name,
                            T... args) {
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
  if constexpr(n > 0) {
    ScriptEngine::push(v, std::forward<T>(args)...);
  }
  if (SQ_FAILED(sq_call(v, n + 1, SQTrue, SQTrue))) {
    sqstd_printcallstack(v);
    sq_settop(v, top);
    error("function {} call failed", name);
    return false;
  }
  ScriptEngine::get(-1, result);
  sq_settop(v, top);
  return true;
}

template <typename TResult, typename... T>
bool ScriptEngine::callFunc(TResult &result, const char *name, T... args) {
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
  if (SQ_FAILED(sq_call(v, n + 1, SQTrue, SQTrue))) {
    sqstd_printcallstack(v);
    sq_settop(v, top);
    error("function {} call failed", name);
    return false;
  }
  ScriptEngine::get(-1, result);
  sq_settop(v, top);
  return true;
}

template <typename TResult, typename TThis, typename... T>
bool ScriptEngine::rawCallFunc(TResult &result, TThis pThis, const char *name,
                               T... args) {
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
  if constexpr (n > 0) {
    ScriptEngine::push(v, std::forward<T>(args)...);
  }
  if (SQ_FAILED(sq_call(v, n + 1, SQTrue, SQTrue))) {
    sqstd_printcallstack(v);
    sq_settop(v, top);
    error("function {} call failed", name);
    return false;
  }
  ScriptEngine::get(-1, result);
  sq_settop(v, top);
  return true;
}

template <typename T> bool ScriptEngine::get(const char *name, T &result) {
  auto v = getVm();
  sq_pushroottable(v);
  HSQOBJECT rootTable;
  sq_getstackobj(v, -1, &rootTable);
  sq_pop(v, 1);
  return ScriptEngine::get(rootTable, name, result);
}

template <typename TThis, typename T>
bool ScriptEngine::get(TThis pThis, const char *name, T &result) {
  auto v = ScriptEngine::getVm();
  auto top = sq_gettop(v);
  push(v, pThis);
  sq_pushstring(v, _SC(name), -1);
  if (SQ_SUCCEEDED(sq_get(v, -2))) {
    auto status = ScriptEngine::get(-1, result);
    sq_settop(v, top);
    return status;
  }
  sq_settop(v, top);
  return false;
}

template <typename TThis, typename T>
bool ScriptEngine::rawGet(TThis pThis, const char *name,
                          T &result) {
  auto v = ScriptEngine::getVm();
  auto top = sq_gettop(v);
  push(v, pThis);
  sq_pushstring(v, _SC(name), -1);
  if (SQ_SUCCEEDED(sq_rawget(v, -2))) {
    auto status = ScriptEngine::get(-1, result);
    sq_settop(v, top);
    return status;
  }
  sq_settop(v, top);
  return false;
}

template <typename TThis>
bool ScriptEngine::exists(TThis pThis, const char *name) {
  auto v = ScriptEngine::getVm();
  auto top = sq_gettop(v);
  push(v, pThis);
  sq_pushstring(v, _SC(name), -1);
  if (SQ_SUCCEEDED(sq_get(v, -2))) {
    auto type = sq_gettype(v, -1);
    sq_settop(v, top);
    return type != OT_NULL;
  }
  sq_settop(v, top);
  return false;
}

template <typename TThis>
bool ScriptEngine::rawExists(TThis pThis, const char *name) {
  auto v = ScriptEngine::getVm();
  auto top = sq_gettop(v);
  push(v, pThis);
  sq_pushstring(v, _SC(name), -1);
  if (SQ_SUCCEEDED(sq_rawget(v, -2))) {
    auto type = sq_gettype(v, -1);
    sq_settop(v, top);
    return type != OT_NULL;
  }
  sq_settop(v, top);
  return false;
}

template <typename T> void ScriptEngine::set(const char *name, T value) {
  auto v = ScriptEngine::getVm();
  sq_pushroottable(v);
  sq_pushstring(v, _SC(name), -1);
  ScriptEngine::push(v, value);
  sq_newslot(v, -3, SQFalse);
  sq_pop(v, 1);
}

template <typename TThis, typename T>
void ScriptEngine::set(TThis pThis, const char *name, T value) {
  auto v = ScriptEngine::getVm();
  push(v, pThis);
  sq_pushstring(v, _SC(name), -1);
  ScriptEngine::push(v, value);
  sq_newslot(v, -3, SQFalse);
  sq_pop(v, 1);
}

} // namespace ng