#pragma once
#include <squirrel.h>
#include "engge/Scripting/ScriptExecute.hpp"

namespace ng {
class DefaultScriptExecute final : public ScriptExecute {
public:
  explicit DefaultScriptExecute(HSQUIRRELVM vm) : m_vm(vm) {}

public:
  void execute(const std::string &code) override;
  bool executeCondition(const std::string &code) override;
  std::string executeDollar(const std::string &code) override;
  SoundDefinition *getSoundDefinition(const std::string &name) override;

private:
  HSQUIRRELVM m_vm{};
  HSQOBJECT m_result{};
};
}
