#pragma once
#include <string>
#include "squirrel3/squirrel.h"
#include "GGEngine.h"

namespace gg
{
class ScriptEngine;
class Pack
{
public:
  virtual void addTo(ScriptEngine &engine) const = 0;
};
class ScriptEngine
{
public:
  explicit ScriptEngine(GGEngine &engine);
  ~ScriptEngine();

  GGEngine &getEngine();

  template <typename TConstant>
  void registerConstant(const SQChar *name, TConstant value);
  void registerGlobalFunction(SQFUNCTION f, const SQChar *functionName, SQInteger nparamscheck = 0, const SQChar *typemask = nullptr);
  void executeScript(const std::string &name);

  template <class TPack>
  void addPack();

  template <typename TEntity>
  static TEntity *getEntity(HSQUIRRELVM v, SQInteger index);

  static GGObject *getObject(HSQUIRRELVM v, SQInteger index);
  static GGRoom *getRoom(HSQUIRRELVM v, SQInteger index);
  static GGActor *getActor(HSQUIRRELVM v, SQInteger index);

  template <class T>
  static void pushObject(HSQUIRRELVM v, T &object);

  static std::function<float(float)> getInterpolationMethod(InterpolationMethod index);

private:
  static SQInteger aux_printerror(HSQUIRRELVM v);
  static void errorHandler(HSQUIRRELVM v, const SQChar *desc, const SQChar *source, SQInteger line, SQInteger column);
  static void printfunc(HSQUIRRELVM v, const SQChar *s, ...);
  static void errorfunc(HSQUIRRELVM v, const SQChar *s, ...);

private:
  GGEngine &_engine;
  HSQUIRRELVM v;
  std::vector<std::unique_ptr<Pack>> _packs;
};

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

} // namespace gg