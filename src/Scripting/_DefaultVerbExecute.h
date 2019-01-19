#pragma once
#include "squirrel.h"
#include "Engine.h"
#include "../_NGUtil.h"

namespace ng
{
class _DefaultScriptExecute : public ScriptExecute
{
  public:
    _DefaultScriptExecute(HSQUIRRELVM vm)
        : _vm(vm)
    {
    }

  public:
    void execute(const std::string &code) override
    {
        std::string c;
        c.append("return ");
        c.append(code);
        _pos = 0;
        // compile
        sq_pushroottable(_vm);
        if (SQ_FAILED(sq_compile(_vm, program_reader, (SQUserPointer)c.data(), _SC("_DefaultScriptExecute"), SQTrue)))
        {
            std::cerr << "Error executing code " << code << std::endl;
            return;
        }
        // call
        sq_pushroottable(_vm);
        if (SQ_FAILED(sq_call(_vm, 1, SQTrue, SQTrue)))
        {
            std::cerr << "Error calling code " << code << std::endl;
            return;
        }
    }

    bool executeCondition(const std::string &code) override
    {
        execute(code);
        // get the result
        auto type = sq_gettype(_vm, -1);
        SQBool result;
        if (SQ_FAILED(sq_getbool(_vm, -1, &result)))
        {
            std::cerr << "Error getting result " << code << std::endl;
            return false;
        }
        std::cout << code << " returns " << result << std::endl;
        return result == SQTrue;
    }

    SoundDefinition *getSoundDefinition(const std::string &name) override
    {
        sq_pushroottable(_vm);
        sq_pushstring(_vm, name.data(), -1);
        sq_get(_vm, -2);
        HSQOBJECT obj;
        sq_getstackobj(_vm, -1, &obj);

        if (!sq_isuserpointer(obj))
        {
            std::cerr << "getSoundDefinition: sound should be a userpointer" << std::endl;
            return nullptr;
        }

        SoundDefinition *pSound = static_cast<SoundDefinition *>(obj._unVal.pUserPointer);
        return pSound;
    }

    static SQInteger program_reader(SQUserPointer p)
    {
        auto code = (char *)p;
        return (SQInteger)code[_pos++];
    }

  private:
    static int _pos;
    HSQUIRRELVM _vm;
};

int _DefaultScriptExecute::_pos = 0;

class _AfterFunction : public Function
{
  public:
    _AfterFunction(std::unique_ptr<Function> before, std::unique_ptr<Function> after)
        : _before(std::move(before)), _after(std::move(after))
    {
    }

    bool isElapsed() override { return _before->isElapsed() && _after->isElapsed(); }
    void operator()() override
    {
        if (!_before->isElapsed())
        {
            (*_before)();
            return;
        }
        (*_after)();
    }

  private:
    std::unique_ptr<Function> _before;
    std::unique_ptr<Function> _after;
};

class _ActorWalk : public Function
{
  public:
    _ActorWalk(Actor &actor, const Object &object)
        : _actor(actor)
    {
        auto pos = object.getPosition();
        auto usePos = object.getUsePosition();
        auto dest = sf::Vector2f(pos.x + usePos.x, pos.y - usePos.y);
        auto facing = _toFacing(object.getUseDirection());
        _actor.walkTo(dest, facing);
    }

  private:
    bool isElapsed() override { return !_actor.isWalking(); }
    void operator()() override
    {
    }

  private:
    Actor &_actor;
};

class _Use : public Function
{
  public:
    _Use(HSQUIRRELVM v, Actor &actor, const InventoryObject &objectSource, const Object &objectTarget)
        : _vm(v), _actor(actor), _objectSource(objectSource), _objectTarget(objectTarget)
    {
    }

  private:
    bool isElapsed() override { return true; }
    void operator()() override
    {
        HSQOBJECT objSource = *(HSQOBJECT *)_objectSource.getHandle();
        HSQOBJECT objTarget = *(HSQOBJECT *)_objectTarget.getTable();

        auto pTable = _actor.getRoom()->getTable();
        sq_pushobject(_vm, objSource);
        sq_pushobject(_vm, *pTable);
        sq_setdelegate(_vm, -2);

        sq_pushobject(_vm, objSource);
        sq_pushstring(_vm, _SC("verbUse"), -1);
        if (SQ_SUCCEEDED(sq_get(_vm, -2)))
        {
            sq_remove(_vm, -2);
            sq_pushobject(_vm, objSource);
            sq_pushobject(_vm, objTarget);
            sq_call(_vm, 2, SQFalse, SQTrue);
            sq_pop(_vm, 2); //pops the roottable and the function
            return;
        }
    }

  private:
    HSQUIRRELVM _vm;
    Actor &_actor;
    const InventoryObject &_objectSource;
    const Object &_objectTarget;
};

class _VerbExecute : public Function
{
  public:
    _VerbExecute(HSQUIRRELVM v, Actor &actor, const Object &object, const std::string &verb)
        : _vm(v), _actor(actor), _object(object), _verb(verb)
    {
    }

  private:
    bool isElapsed() override { return true; }
    void operator()() override
    {
        sq_pushobject(_vm, *_object.getTable());
        sq_pushstring(_vm, _verb.data(), -1);

        if (SQ_SUCCEEDED(sq_get(_vm, -2)))
        {
            sq_remove(_vm, -2);
            sq_pushobject(_vm, *_object.getTable());
            sq_call(_vm, 1, SQFalse, SQTrue);
            sq_pop(_vm, 2); //pops the roottable and the function
            return;
        }
        callVerbDefault(*_object.getTable());
    }

    void callVerbDefault(HSQOBJECT obj)
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

  private:
    HSQUIRRELVM _vm;
    Actor &_actor;
    const Object &_object;
    const std::string &_verb;
};

class _DefaultVerbExecute : public VerbExecute
{
  public:
    _DefaultVerbExecute(HSQUIRRELVM vm, Engine &engine)
        : _vm(vm), _engine(engine)
    {
    }

  private:
    void execute(const Object *pObject, const Verb *pVerb) override
    {
        auto obj = *pObject->getTable();
        getVerb(obj, pVerb);
        if (!pVerb)
            return;

        if (callObjectPreWalk(obj, pVerb->id))
            return;

        auto walk = std::make_unique<_ActorWalk>(*_engine.getCurrentActor(), *pObject);
        auto verb = std::make_unique<_VerbExecute>(_vm, *_engine.getCurrentActor(), *pObject, pVerb->func);
        auto after = std::make_unique<_AfterFunction>(std::move(walk), std::move(verb));
        _engine.addFunction(std::move(after));
    }

    void use(const InventoryObject *pObjectSource, const Object *pObjectTarget) override
    {
        auto walk = std::make_unique<_ActorWalk>(*_engine.getCurrentActor(), *pObjectTarget);
        auto action = std::make_unique<_Use>(_vm, *_engine.getCurrentActor(), *pObjectSource, *pObjectTarget);
        auto after = std::make_unique<_AfterFunction>(std::move(walk), std::move(action));
        _engine.addFunction(std::move(after));
    }

    void execute(const InventoryObject *pObject, const Verb *pVerb) override
    {
        HSQOBJECT obj = *(HSQOBJECT *)pObject->getHandle();

        getVerb(obj, pVerb);
        if (!pVerb)
            return;

        if (callObjectPreWalk(obj, pVerb->id))
            return;

        if (pVerb->id == "use" && useFlags(pObject))
            return;

        auto pTable = _engine.getRoom().getTable();
        sq_pushobject(_vm, obj);
        sq_pushobject(_vm, *pTable);
        sq_setdelegate(_vm, -2);

        auto func = pVerb->func;
        if (callVerb(obj, func))
            return;

        callVerbDefault(obj);
    }

    bool useFlags(const InventoryObject *pObject)
    {
        HSQOBJECT obj = *(HSQOBJECT *)pObject->getHandle();
        SQInteger flags = 0;
        sq_pushobject(_vm, obj);
        sq_pushstring(_vm, _SC("flags"), -1);
        if (SQ_SUCCEEDED(sq_get(_vm, -2)))
        {
            sq_getinteger(_vm, -1, &flags);
            UseFlag useFlag;
            switch (flags)
            {
            case 2:
                useFlag = UseFlag::UseWith;
                break;
            case 4:
                useFlag = UseFlag::UseOn;
                break;
            case 8:
                useFlag = UseFlag::UseIn;
                break;
            }
            _engine.setUseFlag(useFlag, pObject);
            return true;
        }
        return false;
    }

    bool callVerb(HSQOBJECT obj, const std::string &verb)
    {
        sq_pushobject(_vm, obj);
        sq_pushstring(_vm, verb.data(), -1);
        if (SQ_SUCCEEDED(sq_get(_vm, -2)))
        {
            sq_remove(_vm, -2);
            sq_pushobject(_vm, obj);
            sq_call(_vm, 1, SQFalse, SQTrue);
            sq_pop(_vm, 2); //pops the roottable and the function
            return true;
        }
        return false;
    }

    bool callObjectPreWalk(HSQOBJECT obj, const std::string &verb)
    {
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
                return true;
        }
        return false;
    }

    void callVerbDefault(HSQOBJECT obj)
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

    void getVerb(HSQOBJECT obj, const Verb *&pVerb)
    {
        std::string verb;
        if (pVerb)
        {
            verb = pVerb->id;
            pVerb = _engine.getVerb(verb);
            return;
        }
        const SQChar *defaultVerb = getDefaultVerb(obj);
        if (!defaultVerb)
            return;
        verb = defaultVerb;
        pVerb = _engine.getVerb(verb);
    }

    const SQChar *getDefaultVerb(HSQOBJECT obj)
    {
        sq_pushobject(_vm, obj);
        sq_pushstring(_vm, _SC("defaultVerb"), -1);

        const SQChar *defaultVerb = nullptr;
        if (SQ_SUCCEEDED(sq_get(_vm, -2)))
        {
            sq_getstring(_vm, -1, &defaultVerb);
            std::cout << "defaultVerb: " << defaultVerb << std::endl;
        }
        return defaultVerb;
    }

  private:
    HSQUIRRELVM _vm;
    Engine &_engine;
}; // namespace ng
} // namespace ng
