#pragma once
#include <engge/Engine/Trigger.hpp>
#include <squirrel.h>
#include <string>
#include <vector>

namespace ng {
class Engine;
class Object;

class RoomTrigger : public Trigger {
public:
  RoomTrigger(Engine &engine, Object &object, HSQOBJECT inside, HSQOBJECT outside);
  ~RoomTrigger() override;

  HSQOBJECT &getInside() { return m_inside; }
  HSQOBJECT &getOutside() { return m_outside; }

  std::string getName() override;

private:
  HSQUIRRELVM createThread();
  void trigCore() override;
  void callTrigger(std::vector<HSQOBJECT> &params, const std::string &name);

private:
  Engine &m_engine;
  HSQUIRRELVM m_vm{};
  Object &m_object;
  HSQOBJECT m_inside{};
  HSQOBJECT m_outside{};
  bool m_isInside{false};
  SQInteger m_insideParamsCount{0};
  SQInteger m_outsideParamsCount{0};
  std::string m_insideName;
  std::string m_outsideName;
  std::string m_name;
  int m_id{0};
};
} // namespace ng
