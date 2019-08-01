#pragma once
#include "Engine.h"
#include "Trigger.h"
#include "squirrel.h"

namespace ng
{
class _RoomTriggerThread : public ThreadBase
{
  public:
    explicit _RoomTriggerThread(HSQOBJECT thread_obj) : _thread_obj(thread_obj) {}
    virtual ~_RoomTriggerThread() = default;

    virtual HSQUIRRELVM getThread() { return _thread_obj._unVal.pThread; }

  private:
    HSQOBJECT _thread_obj;
};

class _RoomTrigger : public Trigger
{
  public:
    _RoomTrigger(Engine &engine, Object &object, HSQOBJECT inside, HSQOBJECT outside)
        : _engine(engine), _object(object), _inside(inside), _outside(outside)
    {
        _vm = engine.getVm();
        sq_addref(_vm, &inside);
        sq_addref(_vm, &outside);
        sq_resetobject(&thread_obj);

        SQInteger top = sq_gettop(_vm);
        sq_newthread(_vm, 1024);
        if (SQ_FAILED(sq_getstackobj(_vm, -1, &thread_obj)))
        {
            error("Couldn't get coroutine thread from stack");
            return;
        }
        sq_addref(_vm, &thread_obj);
        std::cout << "start thread: " << thread_obj._unVal.pThread << std::endl;
        auto pUniquethread = std::make_unique<_RoomTriggerThread>(thread_obj);
        _engine.addThread(std::move(pUniquethread));

        SQInteger nfreevars;
        sq_pushobject(_vm, _inside);
        sq_getclosureinfo(_vm, -1, &_insideParamsCount, &nfreevars);
        if (SQ_SUCCEEDED(sq_getclosurename(_vm, -1)))
        {
            sq_getstring(_vm, -1, &_insideName);
        }
        sq_pushobject(_vm, _outside);
        sq_getclosureinfo(_vm, -1, &_outsideParamsCount, &nfreevars);
        if (SQ_SUCCEEDED(sq_getclosurename(_vm, -1)))
        {
            sq_getstring(_vm, -1, &_outsideName);
        }
        sq_settop(_vm, top);
    }
    ~_RoomTrigger() override
    {
        sq_release(_vm, &thread_obj);
        sq_release(_vm, &_inside);
        sq_release(_vm, &_outside);
    }

    HSQOBJECT &getInside() { return _inside; }
    HSQOBJECT &getOutside() { return _outside; }

  private:
    void trigCore() override
    {
        auto actor = _engine.getCurrentActor();
        if (!actor)
            return;

        auto inObjectHotspot = _object.getRealHotspot().contains((sf::Vector2i)actor->getPosition());
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
            if (_insideName)
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
                    sq_pushobject(_vm, _outside);
                    sq_pushobject(_vm, _object.getTable());
                }

                std::string name;
                name.append("outside");
                if (_outsideName)
                {
                    name.append(" ").append(_outsideName);
                }
                callTrigger(params, "outside");
            }
        }
    }

    void callTrigger(std::vector<HSQOBJECT> &params, const std::string &name)
    {
        for (auto param : params)
        {
            sq_pushobject(thread_obj._unVal.pThread, param);
        }

        trace("call room {} trigger", name);
        if (SQ_FAILED(sq_call(thread_obj._unVal.pThread, params.size() - 1, SQFalse, SQTrue)))
        {
            error("failed to call room {} trigger", name);
            return;
        }
    }

  private:
    Engine &_engine;
    HSQUIRRELVM _vm{};
    Object &_object;
    HSQOBJECT _inside;
    HSQOBJECT _outside;
    HSQOBJECT thread_obj;
    bool _isInside{false};
    SQInteger _insideParamsCount{0};
    SQInteger _outsideParamsCount{0};
    const SQChar *_insideName{nullptr};
    const SQChar *_outsideName{nullptr};
};
} // namespace ng
