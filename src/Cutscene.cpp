#include "Actor.h"
#include "Cutscene.h"
#include "Engine.h"

namespace ng
{
Cutscene::Cutscene(Engine &engine, HSQUIRRELVM v, HSQOBJECT thread, HSQOBJECT closureObj, HSQOBJECT closureCutsceneOverrideObj, HSQOBJECT envObj)
    : _engine(engine), _v(v), _thread(thread), _state(0), _closureObj(closureObj), _closureCutsceneOverrideObj(closureCutsceneOverrideObj), _envObj(envObj)
{
    _engineVm = engine.getVm();
    _hasCutsceneOverride = !sq_isnull(_closureCutsceneOverrideObj);
    _inputActive = _engine.getInputActive();
    _engine.setInputActive(false);
    _currentActor = _engine.getCurrentActor();
    _inputVerbs = _engine.getInputVerbs();
    _engine.setInputVerbs(false);

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

HSQUIRRELVM Cutscene::getThread()
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
        std::cout << "startCutscene" << std::endl;
        startCutscene();
        break;
    case 1:
        checkEndCutscene();
        break;
    case 2:
        std::cout << "doCutsceneOverride" << std::endl;
        doCutsceneOverride();
        break;
    case 3:
        std::cout << "checkEndCutsceneOverride" << std::endl;
        checkEndCutsceneOverride();
        break;
    case 4:
        std::cout << "endCutscene" << std::endl;
        endCutscene();
        break;
    case 5:
        return;
    }
}

void Cutscene::startCutscene()
{
    _state = 1;
    std::cout << "start cutscene: " << _thread._unVal.pThread << std::endl;
    sq_pushobject(_thread._unVal.pThread, _closureObj);
    sq_pushobject(_thread._unVal.pThread, _envObj);
    if (SQ_FAILED(sq_call(_thread._unVal.pThread, 1, SQFalse, SQTrue)))
    {
        std::cerr << "Couldn't call cutscene" << std::endl;
    }
}

void Cutscene::checkEndCutscene()
{
    auto s = sq_getvmstate(_thread._unVal.pThread);
    if (s == SQ_VMSTATE_IDLE)
    {
        _state = 4;
        std::cout << "end cutscene: " << _thread._unVal.pThread << std::endl;
    }
}

void Cutscene::doCutsceneOverride()
{
    if (_hasCutsceneOverride)
    {
        _state = 3;
        std::cout << "start cutsceneOverride: " << _thread._unVal.pThread << std::endl;
        sq_pushobject(_thread._unVal.pThread, _closureCutsceneOverrideObj);
        sq_pushobject(_thread._unVal.pThread, _envObj);
        if (SQ_FAILED(sq_call(_thread._unVal.pThread, 1, SQFalse, SQTrue)))
        {
            std::cerr << "Couldn't call cutsceneOverride" << std::endl;
        }
        return;
    }
    _state = 4;
}

void Cutscene::checkEndCutsceneOverride()
{
    auto s = sq_getvmstate(_thread._unVal.pThread);
    if (s == SQ_VMSTATE_IDLE)
    {
        _state = 4;
        std::cout << "end checkEndCutsceneOverride: " << _thread._unVal.pThread << std::endl;
    }
}

void Cutscene::endCutscene()
{
    _state = 5;
    _engine.setInputActive(_inputActive);
    _engine.setInputVerbs(_inputVerbs);
    if (_currentActor)
    {
        _engine.setCurrentActor(_currentActor);
        _engine.setRoom(_currentActor->getRoom());
    }
    sq_wakeupvm(_v, SQFalse, SQFalse, SQTrue, SQFalse);
    _engine.stopThread(_thread._unVal.pThread);
}
} // namespace ng