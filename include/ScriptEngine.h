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
  void registerConstant(const SQChar *name, SQInteger value);
  void registerGlobalFunction(SQFUNCTION f, const SQChar *functionName);
  void executeScript(const std::string &name);
};
} // namespace gg