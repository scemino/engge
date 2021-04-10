#pragma once
#include <squirrel.h>
#include "engge/Scripting/ScriptExecute.hpp"

namespace ng {
class DefaultScriptExecute final : public ScriptExecute {
public:
  explicit DefaultScriptExecute(HSQUIRRELVM vm) : m_vm(vm) {}

public:
  void execute(const std::string &code) override {
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

  bool executeCondition(const std::string &code) override {
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

  std::string executeDollar(const std::string &code) override {
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

  SoundDefinition *getSoundDefinition(const std::string &name) override {
    auto top = sq_gettop(m_vm);
    sq_pushroottable(m_vm);
    sq_pushstring(m_vm, name.data(), -1);
    sq_get(m_vm, -2);

    auto sound = EntityManager::getSoundDefinition(m_vm, -1);
    sq_settop(m_vm, top);
    return sound.get();
  }

private:
  HSQUIRRELVM m_vm{};
  HSQOBJECT m_result{};
};
}
