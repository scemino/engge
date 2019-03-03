#pragma once
#include "squirrel.h"
#include "Engine.h"
#include "Actor.h"
#include "Function.h"

namespace ng
{
class Cutscene : public Function
{
  private:
    Engine &_engine;
    HSQUIRRELVM _v;
    HSQOBJECT _thread;
    int _state;
    HSQOBJECT _closureObj;
    HSQOBJECT _closureCutsceneOverrideObj;
    HSQOBJECT _envObj;
    bool _inputActive;
    Actor *_currentActor;
    bool _hasCutsceneOverride;

  public:
    Cutscene(Engine &engine, HSQUIRRELVM v, HSQOBJECT thread, HSQOBJECT closureObj, HSQOBJECT closureCutsceneOverrideObj, HSQOBJECT envObj);

  public:
    bool isElapsed() override;
    void operator()() override;
    void cutsceneOverride();

  private:
    void startCutscene();
    void checkEndCutscene();
    void doCutsceneOverride();
    void checkEndCutsceneOverride();
    void endCutscene();
};
} // namespace ng