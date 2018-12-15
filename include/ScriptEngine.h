#pragma once
#include <string>
#include "squirrel3/squirrel.h"
#include "NGEngine.h"

namespace ng
{
class ScriptEngine;
class Pack
{
public:
  virtual void addTo(ScriptEngine &engine) const = 0;
  virtual ~Pack() {}
};
class ScriptEngine
{
public:
  explicit ScriptEngine(NGEngine &engine);
  ~ScriptEngine();

  NGEngine &getEngine();

  template <typename TConstant>
  void pushValue(TConstant value);
  template <typename TConstant>
  void registerConstant(const SQChar *name, TConstant value);
  template <typename TConstant>
  void registerConstants(std::initializer_list<std::tuple<const SQChar*, TConstant>> list);
  void registerGlobalFunction(SQFUNCTION f, const SQChar *functionName, SQInteger nparamscheck = 0, const SQChar *typemask = nullptr);
  void executeScript(const std::string &name);

  template <class TPack>
  void addPack();

  template <typename TEntity>
  static TEntity *getEntity(HSQUIRRELVM v, SQInteger index);

  static NGObject *getObject(HSQUIRRELVM v, SQInteger index);
  static NGRoom *getRoom(HSQUIRRELVM v, SQInteger index);
  static NGActor *getActor(HSQUIRRELVM v, SQInteger index);

  template <class T>
  static void pushObject(HSQUIRRELVM v, T &object);

  static std::function<float(float)> getInterpolationMethod(InterpolationMethod index);

private:
  static SQInteger aux_printerror(HSQUIRRELVM v);
  static void errorHandler(HSQUIRRELVM v, const SQChar *desc, const SQChar *source, SQInteger line, SQInteger column);
  static void printfunc(HSQUIRRELVM v, const SQChar *s, ...);
  static void errorfunc(HSQUIRRELVM v, const SQChar *s, ...);

private:
  NGEngine &_engine;
  HSQUIRRELVM v;
  std::vector<std::unique_ptr<Pack>> _packs;
};

} // namespace ng