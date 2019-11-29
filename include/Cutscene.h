#pragma once
#include "squirrel.h"
#include "Function.h"
#include "ThreadBase.h"

namespace ng
{
class Actor;
class Engine;
class Room;
class Cutscene : public ThreadBase, public Function
{
private:
  Engine &_engine;
  HSQUIRRELVM _v{};
  HSQUIRRELVM _engineVm{};
  HSQOBJECT _thread{};
  int _state{0};
  HSQOBJECT _closureObj{};
  HSQOBJECT _closureCutsceneOverrideObj{};
  HSQOBJECT _envObj{};
  int _inputState{0};
  bool _hasCutsceneOverride{false};

public:
  Cutscene(Engine &engine, HSQUIRRELVM v, HSQOBJECT thread, HSQOBJECT closureObj, HSQOBJECT closureCutsceneOverrideObj, HSQOBJECT envObj);
  ~Cutscene() override;

  HSQUIRRELVM getThread() const override;
  virtual bool isGlobal() const override { return true; }

public:
  bool isElapsed() override;
  void operator()(const sf::Time &elapsed) override;
  void cutsceneOverride();

private:
  void startCutscene();
  void checkEndCutscene();
  void doCutsceneOverride();
  void checkEndCutsceneOverride();
  void endCutscene();
};
} // namespace ng