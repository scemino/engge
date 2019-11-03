#pragma once
#include <functional>
#include <string>
#include "sqstdio.h"
#include "sqstdaux.h"
#include "Engine.h"
#include "Interpolations.h"
#include "Logger.h"
#include "Room.h"

namespace ng
{
class Entity;
class Light;
class Room;
class ScriptEngine;
class Sound;
class Pack
{
public:
  virtual void addTo(ScriptEngine &engine) const = 0;
  virtual ~Pack() = default;
};
class ScriptEngine
{
public:
  explicit ScriptEngine();
  ~ScriptEngine();

  void setEngine(Engine &engine);
  Engine &getEngine();

  template <typename TConstant>
  void registerConstants(std::initializer_list<std::tuple<const SQChar*, TConstant>> list);
  void registerGlobalFunction(SQFUNCTION f, const SQChar *functionName, SQInteger nparamscheck = 0, const SQChar *typemask = nullptr);
  void executeScript(const std::string &name);
  void executeNutScript(const std::string& name);
  void executeBootScript();

  template <class TPack>
  void addPack();

  template <typename TScriptObject>
  static TScriptObject *getScriptObject(HSQUIRRELVM v, SQInteger index);

  static Entity *getEntity(HSQUIRRELVM v, SQInteger index);
  static Object *getObject(HSQUIRRELVM v, SQInteger index);
  static Room *getRoom(HSQUIRRELVM v, SQInteger index);
  static Actor *getActor(HSQUIRRELVM v, SQInteger index);
  static Light *getLight(HSQUIRRELVM v, SQInteger index);
  static SoundId *getSound(HSQUIRRELVM v, SQInteger index);
  static Sound *getSoundFromId(int id);
  static SoundDefinition *getSoundDefinition(HSQUIRRELVM v, SQInteger index);
  static bool tryGetLight(HSQUIRRELVM v, SQInteger index, Light*& light);

  template <class T>
  static void pushObject(HSQUIRRELVM v, T &object);

  static std::function<float(float)> getInterpolationMethod(InterpolationMethod index);

  template <typename T>
  static void push(HSQUIRRELVM v, T value);
  template <typename First, typename... Rest>
  static void push(HSQUIRRELVM v, First firstValue, Rest... rest);

  template <typename T>
  static bool get(HSQUIRRELVM v, size_t index, T& result);

  template<typename TThis, typename T>
  static bool get(TThis pThis, const char* name, T& result);

  template<typename TThis, typename T>
  static bool get(HSQUIRRELVM v, TThis pThis, const char* name, T& result);

  template<typename TThis, typename T>
  static void set(TThis pThis, const char* name, T value);

  template<typename TThis, typename T>
  static void set(HSQUIRRELVM v, TThis pThis, const char* name, T value);

  template<typename...T>
  static void call(const char* name, T... args);
  static void call(const char* name);

  template<typename TThis, typename...T>
  static void call(TThis pThis, const char* name, T... args);
  template<typename TThis>
  static void call(TThis pThis, const char* name);

  template<typename TResult, typename TThis, typename...T>
  static void callFunc(TResult& result, TThis pThis, const char* name, T... args);

private:
  static SQInteger aux_printerror(HSQUIRRELVM v);
  static void errorHandler(HSQUIRRELVM v, const SQChar *desc, const SQChar *source, SQInteger line, SQInteger column);
  static void printfunc(HSQUIRRELVM v, const SQChar *s, ...);
  static void errorfunc(HSQUIRRELVM v, const SQChar *s, ...);

private:
  Engine *_pEngine{nullptr};
  HSQUIRRELVM v;
  std::vector<std::unique_ptr<Pack>> _packs;

private:
  static Engine *g_pEngine;
};

template <typename First, typename... Rest>
void ScriptEngine::push(HSQUIRRELVM v, First firstValue, Rest... rest)
{
	ScriptEngine::push(v, firstValue);
	ScriptEngine::push(v, rest...);
}

template<typename...T>
void ScriptEngine::call(const char* name, T...args)
{
    constexpr std::size_t n = sizeof...(T);
    auto v = g_pEngine->getVm();
    sq_pushroottable(v);
    sq_pushstring(v, _SC(name), -1);
    if (SQ_FAILED(sq_rawget(v, -2)))
    {
        sq_pop(v, 1);
        trace("can't find {} function", name);
        return;
    }
    sq_remove(v, -2);

    sq_pushroottable(v);
    ScriptEngine::push(v, std::forward<T>(args)...);
    if (SQ_FAILED(sq_call(v, n + 1, SQFalse, SQTrue)))
    {
        sqstd_printcallstack(v);
        sq_pop(v, 1);
        error("function {} call failed", name);
        return;
    }
    sq_pop(v, 1);
}

template<typename TThis, typename...T>
void ScriptEngine::call(TThis pThis, const char* name, T... args)
{
    constexpr std::size_t n = sizeof...(T);
    auto v = g_pEngine->getVm();
    ScriptEngine::push(v, pThis);
    sq_pushstring(v, _SC(name), -1);
    if (SQ_FAILED(sq_rawget(v, -2)))
    {
        sq_pop(v, 1);
        trace("can't find {} function", name);
        return;
    }
    sq_remove(v, -2);

    ScriptEngine::push(v, pThis);
    ScriptEngine::push(v, std::forward<T>(args)...);
    if (SQ_FAILED(sq_call(v, n + 1, SQFalse, SQTrue)))
    {
        sqstd_printcallstack(v);
        sq_pop(v, 1);
        error("function {} call failed", name);
        return;
    }
    sq_pop(v, 1);
}

template<typename TThis>
void ScriptEngine::call(TThis pThis, const char* name)
{
    auto v = g_pEngine->getVm();
    ScriptEngine::push(v, pThis);
    sq_pushstring(v, _SC(name), -1);
    if (SQ_FAILED(sq_rawget(v, -2)))
    {
        sq_pop(v, 1);
        trace("can't find {} function", name);
        return;
    }
    sq_remove(v, -2);

    ScriptEngine::push(v, pThis);
    if (SQ_FAILED(sq_call(v, 1, SQFalse, SQTrue)))
    {
        sqstd_printcallstack(v);
        sq_pop(v, 1);
        error("function {} call failed", name);
        return;
    }
    sq_pop(v, 1);
}

template<typename TResult, typename TThis, typename...T>
void ScriptEngine::callFunc(TResult& result, TThis pThis, const char* name, T... args)
{
    constexpr std::size_t n = sizeof...(T);
    auto v = g_pEngine->getVm();
    ScriptEngine::push(v, pThis);
    sq_pushstring(v, _SC(name), -1);
    if (SQ_FAILED(sq_rawget(v, -2)))
    {
        sq_pop(v, 1);
        trace("can't find {} function", name);
        return;
    }
    sq_remove(v, -2);

    ScriptEngine::push(v, pThis);
    ScriptEngine::push(v, std::forward<T>(args)...);
    if (SQ_FAILED(sq_call(v, n + 1, SQFalse, SQTrue)))
    {
        sqstd_printcallstack(v);
        sq_pop(v, 1);
        error("function {} call failed", name);
        return;
    }
    ScriptEngine::get(v, -1, result);
    sq_pop(v, 1);
}

template<typename TThis, typename T>
bool ScriptEngine::get(TThis pThis, const char* name, T& result)
{
  return ScriptEngine::get(g_pEngine->getVm(), pThis, name, result);
}

template<typename TThis, typename T>
bool ScriptEngine::get(HSQUIRRELVM v, TThis pThis, const char* name, T& result)
{
  push(v, pThis);
  sq_pushstring(v, _SC(name), -1);
  if (SQ_SUCCEEDED(sq_rawget(v, -2)))
  {
    return ScriptEngine::get(v, -1, result);
  }
  return false;
}

template<typename TThis, typename T>
void ScriptEngine::set(TThis pThis, const char* name, T value)
{
  ScriptEngine::set(g_pEngine->getVm(), pThis, name, value);
}

template<typename TThis, typename T>
void ScriptEngine::set(HSQUIRRELVM v, TThis pThis, const char* name, T value)
{
  push(v, pThis);
  sq_pushstring(v, _SC(name), -1);
  ScriptEngine::push(v, value);
  sq_newslot(v, -3, SQFalse);
  sq_pop(v,1);
}

} // namespace ng