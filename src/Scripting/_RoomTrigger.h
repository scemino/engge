#pragma once
#include "squirrel.h"
#include "Engine.h"

namespace ng
{
class _RoomTrigger : public Trigger
{
  public:
    _RoomTrigger(Engine &engine, Object &object, HSQOBJECT inside, HSQOBJECT outside)
        : _engine(engine), _object(object), _v(engine.getVm()), _inside(inside), _outside(outside), _isInside(false)
    {
    }

  private:
    void trig() override
    {
        auto actor = _engine.getCurrentActor();
        if (!actor)
            return;

        auto inObjectHotspot = _object.getRealHotspot().contains((sf::Vector2i)actor->getPosition());
        if (!_isInside && inObjectHotspot)
        {
            _isInside = true;
            sq_pushobject(_v, _inside);
            SQInteger nparams, nfreevars;
            sq_getclosureinfo(_v, -1, &nparams, &nfreevars);

            std::vector<HSQOBJECT> params;
            if (nparams == 2)
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

            callTrigger(params, "inside");
        }
        else if (_isInside && !inObjectHotspot)
        {
            _isInside = false;
            if (_outside._type != SQObjectType::OT_NULL)
            {
                std::vector<HSQOBJECT> params;

                sq_pushobject(_v, _outside);
                SQInteger nparams, nfreevars;
                sq_getclosureinfo(_v, -1, &nparams, &nfreevars);
                if (nparams == 2)
                {
                    sq_pushobject(_v, _outside);
                    sq_pushobject(_v, _object.getTable());
                    params.push_back(actor->getTable());
                }
                else
                {
                    sq_pushobject(_v, _outside);
                    sq_pushobject(_v, _object.getTable());
                }

                callTrigger(params, "outside");
            }
        }
    }

    void callTrigger(std::vector<HSQOBJECT> &params, std::string name)
    {
        // create thread and store it on the stack
        auto thread = sq_newthread(_v, 1024);
        HSQOBJECT thread_obj;
        sq_resetobject(&thread_obj);
        if (SQ_FAILED(sq_getstackobj(_v, -1, &thread_obj)))
        {
            std::cerr << "Couldn't get coroutine thread from stack" << std::endl;
            return;
        }

        for (size_t i = 0; i < params.size(); i++)
        {
            sq_pushobject(thread, params[i]);
        }

        if (SQ_FAILED(sq_call(thread, params.size() - 1, SQFalse, SQTrue)))
        {
            std::cerr << "failed to call room " << name << " trigger" << std::endl;
            return;
        }

        // create a table for a thread
        sq_addref(_v, &thread_obj);
        sq_pushobject(_v, thread_obj);

        std::cout << "start thread: " << thread_obj._unVal.pThread << std::endl;
        _engine.addThread(thread_obj._unVal.pThread);
    }

  private:
    Engine &_engine;
    Object &_object;
    HSQUIRRELVM _v;
    HSQOBJECT _inside;
    HSQOBJECT _outside;
    bool _isInside;
};
} // namespace ng
