#pragma once
#include "engge/Engine/Engine.hpp"
#include "engge/Entities/Objects/Object.hpp"
#include "engge/Engine/Trigger.hpp"
#include <squirrel.h>
#include <string>

namespace ng {
class RoomTrigger : public Trigger {
public:
  RoomTrigger(Engine &engine, Object &object, HSQOBJECT inside, HSQOBJECT outside);
  ~RoomTrigger() override;

  HSQOBJECT &getInside() { return _inside; }
  HSQOBJECT &getOutside() { return _outside; }

  std::string getName() override;

private:
  HSQUIRRELVM createThread();
  void trigCore() override;
  void callTrigger(std::vector<HSQOBJECT> &params, const std::string &name);

private:
  Engine &_engine;
  HSQUIRRELVM _vm{};
  Object &_object;
  HSQOBJECT _inside{};
  HSQOBJECT _outside{};
  bool _isInside{false};
  SQInteger _insideParamsCount{0};
  SQInteger _outsideParamsCount{0};
  std::string _insideName;
  std::string _outsideName;
  std::string _name;
  int _id{0};
};
} // namespace ng
