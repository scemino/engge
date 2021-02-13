#include <squirrel.h>
#include "../../extlibs/squirrel/squirrel/sqpcheader.h"
#include "../../extlibs/squirrel/squirrel/sqvm.h"
#include "../../extlibs/squirrel/squirrel/sqstring.h"
#include "../../extlibs/squirrel/squirrel/sqtable.h"
#include "../../extlibs/squirrel/squirrel/sqarray.h"
#include "../../extlibs/squirrel/squirrel/sqfuncproto.h"
#include "../../extlibs/squirrel/squirrel/sqclosure.h"
#include "../../extlibs/squirrel/squirrel/squserdata.h"
#include "../../extlibs/squirrel/squirrel/sqclass.h"
#include "engge/Engine/Callback.hpp"
#include "engge/System/Locator.hpp"
#include "engge/System/Logger.hpp"
#include "engge/Scripting/ScriptEngine.hpp"

namespace ng {
Callback::Callback(int id, ngf::TimeSpan duration, std::string method, HSQOBJECT arg)
    : TimeFunction(duration), m_id(id), m_method(std::move(method)), m_arg(arg) {
  sq_addref(ScriptEngine::getVm(), &m_arg);
}

Callback::~Callback(){
  sq_release(ScriptEngine::getVm(), &m_arg);
}

void Callback::onElapsed() {
  if (m_callbackDone)
    return;
  m_callbackDone = true;

  auto v = ScriptEngine::getVm();
  SQObjectPtr method;
  _table(v->_roottable)->Get(ScriptEngine::toSquirrel(m_method), method);

  SQInteger numArgs = 1;
  sq_pushobject(v, method);
  sq_pushroottable(v);
  if (m_arg._type != OT_NULL) {
    numArgs++;
    sq_pushobject(v, m_arg);
  }
  if (SQ_FAILED(sq_call(v, numArgs, SQFalse, SQTrue))) {
    error("failed to call callback");
  }
  sq_pop(v, 1);
}
} // namespace ng
