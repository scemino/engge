#pragma once
#include "engge/Engine/ThreadBase.hpp"
#include <squirrel.h>
#include <string>

namespace ng {
class RoomTriggerThread final : public ThreadBase {
public:
  RoomTriggerThread(HSQUIRRELVM vm, std::string name, HSQOBJECT thread_obj);
  ~RoomTriggerThread() override;

  [[nodiscard]] std::string getName() const override;
  [[nodiscard]] HSQUIRRELVM getThread() const override;

private:
  HSQUIRRELVM m_vm;
  std::string m_name;
  HSQOBJECT m_thread_obj;
};
}