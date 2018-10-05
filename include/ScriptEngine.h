#pragma once
#include <string>
#include "squirrel3/squirrel.h"
#include "GGEngine.h"

namespace gg
{
class ScriptEngine
{
private:
  HSQUIRRELVM v;

public:
  explicit ScriptEngine(GGEngine &engine);
  ~ScriptEngine();

  void registerBoolConstant(const SQChar *name, bool value);
  void registerStringConstant(const SQChar *name, const SQChar *value);
  void registerConstant(const SQChar *name, SQInteger value);
  void registerGlobalFunction(SQFUNCTION f, const SQChar *functionName, SQInteger nparamscheck = 0, const SQChar *typemask = nullptr);
  void executeScript(const std::string &name);

private:
  static SQInteger aux_printerror(HSQUIRRELVM v);
  static void errorHandler(HSQUIRRELVM v, const SQChar *desc, const SQChar *source, SQInteger line, SQInteger column);
  static void printfunc(HSQUIRRELVM v, const SQChar *s, ...);
  static void errorfunc(HSQUIRRELVM v, const SQChar *s, ...);
};
} // namespace gg