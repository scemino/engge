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
  ScriptEngine(GGEngine &engine);
  ~ScriptEngine();

  void registerBoolConstant(const SQChar *name, bool value);
  void registerConstant(const SQChar *name, SQInteger value);
  void registerGlobalFunction(SQFUNCTION f, const SQChar *fname);
  void executeScript(const std::string &name);

private:
  void _register_object();
};
} // namespace gg