#pragma once
#include <string>
#include <vector>
#include "squirrel.h"
#include "ThreadBase.hpp"

namespace ng {
class Thread final : public ThreadBase {
public:
  Thread(std::string  name, bool isGlobal,
         HSQUIRRELVM v,
         HSQOBJECT thread_obj,
         HSQOBJECT env_obj,
         HSQOBJECT closureObj,
         std::vector<HSQOBJECT> args);
  ~Thread() final;

  [[nodiscard]] std::string getName() const final;
  [[nodiscard]] HSQUIRRELVM getThread() const final;
  [[nodiscard]] bool isGlobal() const final { return m_isGlobal; }

  bool call();

private:
  std::string m_name;
  HSQUIRRELVM m_v;
  HSQOBJECT m_threadObj;
  HSQOBJECT m_envObj;
  HSQOBJECT m_closureObj;
  std::vector<HSQOBJECT> m_args;
  bool m_isGlobal{false};
};
} // namespace ng
