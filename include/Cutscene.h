#pragma once
#include "squirrel.h"
#include "Engine.h"
#include "Actor.h"
#include "Function.h"

namespace ng
{
class Room;
class Cutscene : public Function
{
private:
  Engine &_engine;
  HSQUIRRELVM _v;
  HSQOBJECT _thread;
  int _state{0};
  HSQOBJECT _closureObj;
  HSQOBJECT _closureCutsceneOverrideObj;
  HSQOBJECT _envObj;
  bool _inputActive{false};
  bool _inputVerbs{false};
  Actor *_currentActor{nullptr};
  bool _hasCutsceneOverride{false};

public:
  Cutscene(Engine &engine, HSQUIRRELVM v, HSQOBJECT thread, HSQOBJECT closureObj, HSQOBJECT closureCutsceneOverrideObj, HSQOBJECT envObj);

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