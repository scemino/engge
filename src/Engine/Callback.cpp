#include "squirrel.h"
#include "../../extlibs/squirrel/squirrel/sqpcheader.h"
#include "../../extlibs/squirrel/squirrel/sqvm.h"
#include "../../extlibs/squirrel/squirrel/sqstring.h"
#include "../../extlibs/squirrel/squirrel/sqtable.h"
#include "../../extlibs/squirrel/squirrel/sqarray.h"
#include "../../extlibs/squirrel/squirrel/sqfuncproto.h"
#include "../../extlibs/squirrel/squirrel/sqclosure.h"
#include "../../extlibs/squirrel/squirrel/squserdata.h"
#include "../../extlibs/squirrel/squirrel/sqclass.h"
#include "Engine/Callback.hpp"
#include "System/Locator.hpp"
#include "System/Logger.hpp"
#include "Scripting/ScriptEngine.hpp"

namespace ng {
Callback::Callback(int id, sf::Time duration, std::string method)
    : TimeFunction(duration), _id(id), _method(std::move(method)) {
}

Callback::Callback(int id, sf::Time duration, std::string method, std::vector<HSQOBJECT> args)
    : TimeFunction(duration), _id(id), _method(std::move(method)), _args(std::move(args)) {
}

void Callback::onElapsed() {
  if (_callbackDone)
    return;
  _callbackDone = true;

  auto v = ScriptEngine::getVm();
  SQObjectPtr method;
  _table(v->_roottable)->Get(ScriptEngine::toSquirrel(_method), method);

  sq_pushobject(v, method);
  sq_pushroottable(v);
  for (auto &arg : _args) {
    sq_pushobject(v, arg);
  }
  if (SQ_FAILED(sq_call(v, 1 + _args.size(), SQFalse, SQTrue))) {
    error("failed to call callback");
  }
  sq_pop(v, 1);
}
} // namespace ng
