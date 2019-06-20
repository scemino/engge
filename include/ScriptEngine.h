#pragma once
#include <functional>
#include <string>
#include "squirrel.h"
#include "Engine.h"
#include "Interpolations.h"

namespace ng
{
class Entity;
class Light;
class Room;
class ScriptEngine;
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
  void pushValue(TConstant value);
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

  template <class T>
  static void pushObject(HSQUIRRELVM v, T &object);

  static std::function<float(float)> getInterpolationMethod(InterpolationMethod index);

private:
  static SQInteger aux_printerror(HSQUIRRELVM v);
  static void errorHandler(HSQUIRRELVM v, const SQChar *desc, const SQChar *source, SQInteger line, SQInteger column);
  static void printfunc(HSQUIRRELVM v, const SQChar *s, ...);
  static void errorfunc(HSQUIRRELVM v, const SQChar *s, ...);

private:
  Engine *_pEngine{nullptr};
  HSQUIRRELVM v;
  std::vector<std::unique_ptr<Pack>> _packs;
};

} // namespace ng