#pragma once
#include "squirrel3/squirrel.h"
#include "Engine.h"

namespace ng
{
class _RoomTrigger : public Trigger
{
  public:
    _RoomTrigger(Engine &engine, const Object &object, HSQUIRRELVM v, HSQOBJECT inside, HSQOBJECT outside)
        : _engine(engine), _object(object), _v(v), _inside(inside), _outside(outside), _isInside(false)
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
            SQUnsignedInteger nparams, nfreevars;
            sq_getclosureinfo(_v, -1, &nparams, &nfreevars);
            sq_pushroottable(_v);
            if (nparams == 2)
            {
                sq_pushstring(_v, actor->getName().data(), -1);
                sq_get(_v, -2);
                HSQOBJECT actorObject;
                sq_resetobject(&actorObject);
                sq_getstackobj(_v, -1, &actorObject);
                sq_pushobject(_v, _inside);
                sq_pushroottable(_v);
                sq_pushobject(_v, actorObject);
            }
            if (SQ_FAILED(sq_call(_v, nparams, SQFalse, SQTrue)))
            {
                sq_throwerror(_v, "failed to call room inside trigger");
            }
        }
        else if (_isInside && !inObjectHotspot)
        {
            _isInside = false;
            if (_outside._type != SQObjectType::OT_NULL)
            {
                sq_pushobject(_v, _outside);
                SQUnsignedInteger nparams, nfreevars;
                sq_getclosureinfo(_v, -1, &nparams, &nfreevars);
                sq_pushroottable(_v);
                if (nparams == 2)
                {
                    sq_pushstring(_v, actor->getName().data(), -1);
                    sq_get(_v, -2);
                    HSQOBJECT actorObject;
                    sq_resetobject(&actorObject);
                    sq_getstackobj(_v, -1, &actorObject);
                    sq_pushobject(_v, _outside);
                    sq_pushroottable(_v);
                    sq_pushobject(_v, actorObject);
                }
                if (SQ_FAILED(sq_call(_v, nparams, SQFalse, SQTrue)))
                {
                    sq_throwerror(_v, "failed to call room outside trigger");
                }
            }
        }
    }

  private:
    Engine &_engine;
    const Object &_object;
    HSQUIRRELVM _v;
    HSQOBJECT _inside;
    HSQOBJECT _outside;
    bool _isInside;
};
} // namespace ng
