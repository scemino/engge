#include <engge/System/Logger.hpp>
#include <engge/Engine/EntityManager.hpp>
#include "DefaultScriptExecute.hpp"

namespace ng {
void DefaultScriptExecute::execute(const std::string &code) {
  sq_resetobject(&m_result);
  auto top = sq_gettop(m_vm);
// compile
  sq_pushroottable(m_vm);
  if (SQ_FAILED(sq_compilebuffer(m_vm, code.data(), code.size(), _SC("_DefaultScriptExecute"), SQTrue))) {
    error("Error executing code {}", code);
    return;
  }
  sq_push(m_vm, -2);
// call
  if (SQ_FAILED(sq_call(m_vm, 1, SQTrue, SQTrue))) {
    error("Error calling code {}", code);
    return;
  }
  sq_getstackobj(m_vm, -1, &m_result);
  sq_settop(m_vm, top);
}

bool DefaultScriptExecute::executeCondition(const std::string &code) {
  std::string c;
  c.append("return ");
  c.append(code);

  execute(c);
  if (m_result._type == OT_BOOL) {
    trace("{} returns {}", code, sq_objtobool(&m_result));
    return sq_objtobool(&m_result);
  }

  if (m_result._type == OT_INTEGER) {
    trace("{} return {}", code, sq_objtointeger(&m_result));
    return sq_objtointeger(&m_result) != 0;
  }

  error("Error getting result {}", code);
  return false;
}

std::string DefaultScriptExecute::executeDollar(const std::string &code) {
  std::string c;
  c.append("return ");
  c.append(code);

  execute(c);
// get the result
  if (m_result._type != OT_STRING) {
    error("Error getting result {}", code);
    return "";
  }
  trace("{} returns {}", code, sq_objtostring(&m_result));
  return sq_objtostring(&m_result);
}

SoundDefinition *DefaultScriptExecute::getSoundDefinition(const std::string &name) {
  auto top = sq_gettop(m_vm);
  sq_pushroottable(m_vm);
  sq_pushstring(m_vm, name.data(), -1);
  sq_get(m_vm, -2);

  auto sound = EntityManager::getSoundDefinition(m_vm, -1);
  sq_settop(m_vm, top);
  return sound.get();
}
}
