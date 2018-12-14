#pragma once
#include "squirrel3/squirrel.h"
#include "GGEngine.h"

namespace gg
{
class _DefaultVerbExecute : public VerbExecute
{
  public:
    _DefaultVerbExecute(HSQUIRRELVM vm, GGEngine &engine)
        : _vm(vm), _engine(engine)
    {
    }

  public:
    void execute(GGObject *pObject, const Verb *pVerb) override
    {
        auto pRoom = pObject->getRoom();
        sq_pushroottable(_vm);
        sq_pushstring(_vm, pRoom->getId().data(), -1);
        if (SQ_SUCCEEDED(sq_get(_vm, -2)))
        {
            HSQOBJECT room;
            sq_getstackobj(_vm, -1, &room);

            sq_pushobject(_vm, room);
            sq_pushstring(_vm, pObject->getId().data(), -1);

            if (SQ_SUCCEEDED(sq_get(_vm, -2)))
            {
                HSQOBJECT obj;
                sq_getstackobj(_vm, -1, &obj);

                std::string verb;
                if (pVerb)
                {
                    verb = pVerb->id;
                }
                else
                {
                    sq_pushobject(_vm, obj);
                    sq_pushstring(_vm, _SC("defaultVerb"), -1);

                    const SQChar *defaultVerb = nullptr;
                    if (SQ_SUCCEEDED(sq_get(_vm, -2)))
                    {
                        sq_getstring(_vm, -1, &defaultVerb);
                        verb = defaultVerb;
                        std::cout << "defaultVerb: " << defaultVerb << std::endl;
                    }
                    if (!defaultVerb)
                        return;
                }

                // first check for objectPreWalk
                sq_pushobject(_vm, obj);
                sq_pushstring(_vm, _SC("objectPreWalk"), -1);
                if (SQ_SUCCEEDED(sq_get(_vm, -2)))
                {
                    sq_remove(_vm, -2);
                    sq_pushobject(_vm, obj);
                    sq_pushstring(_vm, verb.data(), -1);
                    sq_pushnull(_vm);
                    sq_pushnull(_vm);
                    sq_call(_vm, 4, SQTrue, SQTrue);
                    SQInteger handled = 0;
                    sq_getinteger(_vm, -1, &handled);
                    if (handled == 1)
                        return;
                }

                pVerb = _engine.getVerb(verb);
                if (!pVerb)
                    return;
                auto func = pVerb->func;

                sq_pop(_vm, 2); //pops the roottable and the function

                sq_pushobject(_vm, obj);
                sq_pushstring(_vm, func.data(), -1);

                if (SQ_SUCCEEDED(sq_get(_vm, -2)))
                {
                    sq_remove(_vm, -2);
                    sq_pushobject(_vm, obj);
                    sq_call(_vm, 1, SQFalse, SQTrue);
                    sq_pop(_vm, 2); //pops the roottable and the function
                }
                else if (pVerb->id == "walkto")
                {
                    _engine.getCurrentActor()->walkTo(pObject->getPosition());
                }
                else
                {
                    sq_pushobject(_vm, obj);
                    sq_pushstring(_vm, _SC("verbDefault"), -1);

                    if (SQ_SUCCEEDED(sq_get(_vm, -2)))
                    {
                        sq_remove(_vm, -2);
                        sq_pushobject(_vm, obj);
                        sq_call(_vm, 1, SQFalse, SQTrue);
                        sq_pop(_vm, 2); //pops the roottable and the function
                    }
                }
            }
        }
    }

  private:
    HSQUIRRELVM _vm;
    GGEngine &_engine;
}; // namespace gg
} // namespace gg
