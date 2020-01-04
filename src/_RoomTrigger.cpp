#include "_RoomTrigger.h"
#include "Locator.h"
#include "Logger.h"
#include "Object.h"
#include "ResourceManager.h"
#include "ScriptEngine.h"
#include "_Util.h"
#include "squirrel.h"

namespace ng
{
_RoomTrigger::_RoomTrigger(Engine &engine, Object &object, HSQOBJECT inside, HSQOBJECT outside)
    : _engine(engine), _object(object), _inside(inside), _outside(outside)
{
    _vm = engine.getVm();
    sq_addref(_vm, &inside);
    sq_addref(_vm, &outside);
    sq_resetobject(&thread_obj);

    auto id = Locator::getResourceManager().getThreadId();

    SQInteger top = sq_gettop(_vm);
    sq_newthread(_vm, 1024);
    if (SQ_FAILED(sq_getstackobj(_vm, -1, &thread_obj)))
    {
        error("Couldn't get coroutine thread from stack");
        return;
    }
    sq_addref(_vm, &thread_obj);

    sq_pushconsttable(thread_obj._unVal.pThread);
    sq_pushstring(thread_obj._unVal.pThread, _SC("_id"), -1);
    ScriptEngine::push(thread_obj._unVal.pThread, id);
    sq_newslot(thread_obj._unVal.pThread, -3, SQTrue);
    sq_pop(thread_obj._unVal.pThread, 1);

    trace("start thread: {}", (long)thread_obj._unVal.pThread);
    auto pUniquethread = std::make_unique<_RoomTriggerThread>(thread_obj);
    _engine.addThread(std::move(pUniquethread));

    const SQChar *insideName{nullptr};
    SQInteger nfreevars;
    sq_pushobject(_vm, _inside);
    sq_getclosureinfo(_vm, -1, &_insideParamsCount, &nfreevars);
    if (SQ_SUCCEEDED(sq_getclosurename(_vm, -1)))
    {
        sq_getstring(_vm, -1, &insideName);
        if (insideName)
            _insideName = insideName;
    }

    const SQChar *outsideName{nullptr};
    sq_pushobject(_vm, _outside);
    sq_getclosureinfo(_vm, -1, &_outsideParamsCount, &nfreevars);
    if (SQ_SUCCEEDED(sq_getclosurename(_vm, -1)))
    {
        sq_getstring(_vm, -1, &outsideName);
        if (outsideName)
            _outsideName = outsideName;
    }
    sq_settop(_vm, top);
    _name.append(_object.getName())
        .append(" [")
        .append(_insideName)
        .append(",")
        .append(_outsideName)
        .append("]");

    trace("add trigger: {}", _name);
}

_RoomTrigger::~_RoomTrigger()
{
    sq_release(_vm, &thread_obj);
    sq_release(_vm, &_inside);
    sq_release(_vm, &_outside);
}

void _RoomTrigger::trigCore()
{
    auto actor = _engine.getCurrentActor();
    if (!actor)
        return;

    auto inObjectHotspot = _object.getRealHotspot().contains((sf::Vector2i)actor->getRealPosition());
    if (!_isInside && inObjectHotspot)
    {
        _isInside = true;

        std::vector<HSQOBJECT> params;
        if (_insideParamsCount == 2)
        {
            params.push_back(_inside);
            params.push_back(_object.getTable());
            params.push_back(actor->getTable());
        }
        else
        {
            params.push_back(_inside);
            params.push_back(_object.getTable());
        }

        std::string name;
        name.append("inside");
        if (!_insideName.empty())
        {
            name.append(" ").append(_insideName);
        }
        callTrigger(params, name);
    }
    else if (_isInside && !inObjectHotspot)
    {
        _isInside = false;
        if (_outside._type != SQObjectType::OT_NULL)
        {
            std::vector<HSQOBJECT> params;
            if (_outsideParamsCount == 2)
            {
                params.push_back(_outside);
                params.push_back(_object.getTable());
                params.push_back(actor->getTable());
            }
            else
            {
                params.push_back(_outside);
                params.push_back(_object.getTable());
            }

            std::string name;
            name.append("outside");
            if (!_outsideName.empty())
            {
                name.append(" ").append(_outsideName);
            }
            callTrigger(params, "outside");
        }
    }
}

void _RoomTrigger::callTrigger(std::vector<HSQOBJECT> &params, const std::string &name)
{
    for (auto param : params)
    {
        sq_pushobject(thread_obj._unVal.pThread, param);
    }

    trace("call room {} trigger ({})", name, (long)this);
    if (SQ_FAILED(sq_call(thread_obj._unVal.pThread, params.size() - 1, SQFalse, SQTrue)))
    {
        error("failed to call room {} trigger", name);
        return;
    }
}

std::string _RoomTrigger::getName() { return _name; }
} // namespace ng
