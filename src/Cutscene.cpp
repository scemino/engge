#include "Camera.h"
#include "Cutscene.h"
#include "Engine.h"
#include "Locator.h"
#include "Logger.h"
#include "ResourceManager.h"
#include "ScriptEngine.h"

namespace ng
{
Cutscene::Cutscene(Engine &engine, HSQUIRRELVM v, HSQOBJECT thread, HSQOBJECT closureObj, HSQOBJECT closureCutsceneOverrideObj, HSQOBJECT envObj)
    : _engine(engine), _v(v), _thread(thread), _state(0), _closureObj(closureObj), _closureCutsceneOverrideObj(closureCutsceneOverrideObj), _envObj(envObj)
{
    _engineVm = engine.getVm();
    _hasCutsceneOverride = !sq_isnull(_closureCutsceneOverrideObj);
    _inputState = _engine.getInputState();
    trace("Cutscene with inputState {}", _inputState);
    _engine.setInputActive(false);

    _id = Locator::getResourceManager().getThreadId();

    sq_addref(_engineVm, &_thread);
    sq_addref(_engineVm, &_closureObj);
    sq_addref(_engineVm, &_closureCutsceneOverrideObj);
    sq_addref(_engineVm, &_envObj);
}

Cutscene::~Cutscene()
{
    sq_release(_engineVm, &_thread);
    sq_release(_engineVm, &_closureObj);
    sq_release(_engineVm, &_closureCutsceneOverrideObj);
    sq_release(_engineVm, &_envObj);
}

HSQUIRRELVM Cutscene::getThread() const
{
    return _thread._unVal.pThread;
}

bool Cutscene::isElapsed() { return _state == 5; }

void Cutscene::cutsceneOverride()
{
    if (_hasCutsceneOverride && _state == 1)
        _state = 2;
}

void Cutscene::operator()(const sf::Time &elapsed)
{
    switch (_state)
    {
    case 0:
        trace("startCutscene");
        startCutscene();
        break;
    case 1:
        checkEndCutscene();
        break;
    case 2:
        trace("doCutsceneOverride");
        doCutsceneOverride();
        break;
    case 3:
        trace("checkEndCutsceneOverride");
        checkEndCutsceneOverride();
        break;
    case 4:
        trace("endCutscene");
        endCutscene();
        break;
    case 5:
        return;
    }
}

void Cutscene::startCutscene()
{
    _state = 1;
    trace("start cutscene: {}", (long)_thread._unVal.pThread);
    sq_pushobject(_thread._unVal.pThread, _closureObj);
    sq_pushobject(_thread._unVal.pThread, _envObj);
    if (SQ_FAILED(sq_call(_thread._unVal.pThread, 1, SQFalse, SQTrue)))
    {
        error("Couldn't call cutscene");
    }
}

void Cutscene::checkEndCutscene()
{
    if (isStopped())
    {
        _state = 4;
        trace("end cutscene: {}", (long)_thread._unVal.pThread);
    }
}

void Cutscene::doCutsceneOverride()
{
    if (_hasCutsceneOverride)
    {
        _state = 3;
        trace("start cutsceneOverride: {}", (long)_thread._unVal.pThread);
        sq_pushobject(_thread._unVal.pThread, _closureCutsceneOverrideObj);
        sq_pushobject(_thread._unVal.pThread, _envObj);
        if (SQ_FAILED(sq_call(_thread._unVal.pThread, 1, SQFalse, SQTrue)))
        {
            error("Couldn't call cutsceneOverride");
        }
        return;
    }
    _state = 4;
}

void Cutscene::checkEndCutsceneOverride()
{
    if (isStopped())
    {
        _state = 4;
        trace("end checkEndCutsceneOverride: {}", (long)_thread._unVal.pThread);
    }
}

void Cutscene::endCutscene()
{
    _state = 5;
    trace("End cutscene with inputState {}", _inputState);
    _engine.setInputState(_inputState);
    _engine.follow(_engine.getCurrentActor());
    sq_wakeupvm(_v, SQFalse, SQFalse, SQTrue, SQFalse);
    _engine.stopThread(_id);
}
} // namespace ng