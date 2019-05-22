#pragma once

#include "squirrel.h"
#include "Engine.h"
#include "ScriptExecute.h"
#include "../_NGUtil.h"

namespace ng
{
class _DefaultScriptExecute : public ScriptExecute
{
public:
    explicit _DefaultScriptExecute(HSQUIRRELVM vm)
        : _vm(vm)
    {
    }

public:
    void execute(const std::string &code) override
    {
        sq_resetobject(&_result);
        _pos = 0;
        auto top = sq_gettop(_vm);
        // compile
        sq_pushroottable(_vm);
        if (SQ_FAILED(sq_compilebuffer(_vm, code.data(), code.size(), _SC("_DefaultScriptExecute"), SQTrue)))
        {
            std::cerr << "Error executing code " << code << std::endl;
            return;
        }
        sq_push(_vm, -2);
        // call
        if (SQ_FAILED(sq_call(_vm, 1, SQTrue, SQTrue)))
        {
            std::cerr << "Error calling code " << code << std::endl;
            return;
        }
        sq_getstackobj(_vm, -1, &_result);
        sq_settop(_vm, top);
    }

    bool executeCondition(const std::string &code) override
    {
        std::string c;
        c.append("return ");
        c.append(code);

        execute(c);
        if (_result._type == OT_BOOL)
        {
            std::cout << code << " returns " << sq_objtobool(&_result) << std::endl;
            return sq_objtobool(&_result);
        }

        if (_result._type == OT_INTEGER)
        {
            std::cout << code << " returns " << sq_objtointeger(&_result) << std::endl;
            return sq_objtointeger(&_result) != 0;
        }

        std::cerr << "Error getting result " << code << std::endl;
        return false;
    }

    std::string executeDollar(const std::string &code) override
    {
        std::string c;
        c.append("return ");
        c.append(code);

        execute(c);
        // get the result
        if (_result._type != OT_STRING)
        {
            std::cerr << "Error getting result " << code << std::endl;
            return "";
        }
        std::cout << code << " returns " << sq_objtostring(&_result) << std::endl;
        return sq_objtostring(&_result);
    }

    std::shared_ptr<SoundDefinition> getSoundDefinition(const std::string &name) override
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

        std::shared_ptr<SoundDefinition> pSound = std::shared_ptr<SoundDefinition>(
            static_cast<SoundDefinition *>(obj._unVal.pUserPointer));
        return pSound;
    }

private:
    static int _pos;
    HSQUIRRELVM _vm;
    HSQOBJECT _result;
};

int _DefaultScriptExecute::_pos = 0;

class _CompositeFunction : public Function
{
    bool isElapsed() override { return _functions.empty(); }

    void operator()(const sf::Time &elapsed) override
    {
        if (_functions.empty())
            return;
        if (_functions[0]->isElapsed())
        {
            _functions.erase(_functions.begin());
        }
        else
        {
            (*_functions[0])(elapsed);
        }
    }

public:
    _CompositeFunction &push_back(std::unique_ptr<Function> func)
    {
        _functions.push_back(std::move(func));
        return *this;
    }

private:
    std::vector<std::unique_ptr<Function>> _functions;
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

    void operator()(const sf::Time &elapsed) override
    {
    }

private:
    Actor &_actor;
};

class _Use : public Function
{
public:
    _Use(HSQUIRRELVM v, Actor &actor, const InventoryObject &objectSource, Object &objectTarget)
        : _vm(v), _actor(actor), _objectSource(objectSource), _objectTarget(objectTarget)
    {
    }

private:
    bool isElapsed() override { return true; }

    void operator()(const sf::Time &elapsed) override
    {
        HSQOBJECT objSource = *(HSQOBJECT *)_objectSource.getHandle();
        auto &objTarget = _objectTarget.getTable();

        auto &roomTable = _actor.getRoom()->getTable();
        sq_pushobject(_vm, objSource);
        sq_pushobject(_vm, roomTable);
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
    Object &_objectTarget;
};

class _PostWalk : public Function
{
public:
    _PostWalk(const HSQUIRRELVM &v, const HSQOBJECT &object, int verb)
        : _vm(v), _object(object), _verb(verb)
    {
    }

    bool isElapsed() override { return _done; }

    void operator()(const sf::Time &elapsed) override
    {
        _done = true;
        sq_pushobject(_vm, _object);
        sq_pushstring(_vm, _SC("objectPostWalk"), -1);
        if (SQ_SUCCEEDED(sq_get(_vm, -2)))
        {
            sq_remove(_vm, -2);
            sq_pushobject(_vm, _object);
            sq_pushinteger(_vm, _verb);
            sq_pushnull(_vm);
            sq_pushnull(_vm);
            sq_call(_vm, 4, SQTrue, SQTrue);
            SQInteger handled = 0;
            sq_getinteger(_vm, -1, &handled);
            if (handled == 1)
            {
                _handled = true;
                return;
            }
        }
        _handled = false;
    }

private:
    const HSQUIRRELVM &_vm;
    const HSQOBJECT &_object;
    int _verb;
    bool _done{false};
    bool _handled{false};
};

class _VerbExecute : public Function
{
public:
    _VerbExecute(Engine &engine, HSQUIRRELVM v, Actor &actor, Object &object, const std::string &verb)
        : _engine(engine), _vm(v), _object(object), _verb(verb)
    {
    }

private:
    bool isElapsed() override { return _done; }

    void operator()(const sf::Time &elapsed) override
    {
        _done = true;
        sq_pushobject(_vm, _object.getTable());
        sq_pushstring(_vm, _verb.data(), -1);

        if (SQ_SUCCEEDED(sq_get(_vm, -2)))
        {
            sq_remove(_vm, -2);
            sq_pushobject(_vm, _object.getTable());
            if (SQ_FAILED(sq_call(_vm, 1, SQFalse, SQTrue)))
            {
                std::cout << "failed to execute verb " << _verb.data() << std::endl;
                sqstd_printcallstack(_vm);
                return;
            }
            sq_pop(_vm, 2); //pops the roottable and the function
            return;
        }
        if (callDefaultObjectVerb())
            return;

        callVerbDefault(_object.getTable());
    }

    bool callDefaultObjectVerb()
    {
        auto &obj = _engine.getDefaultObject();
        auto pActor = _engine.getCurrentActor();

        sq_pushobject(_vm, obj);
        sq_pushstring(_vm, _verb.data(), -1);

        if (SQ_SUCCEEDED(sq_get(_vm, -2)))
        {
            sq_remove(_vm, -2);
            sq_pushobject(_vm, obj);
            if (pActor)
            {
                sq_pushobject(_vm, pActor->getTable());
            }
            else
            {
                sq_pushnull(_vm);
            }
            sq_pushobject(_vm, _object.getTable());
            if (SQ_FAILED(sq_call(_vm, 3, SQFalse, SQTrue)))
            {
                std::cout << "failed to execute verb " << _verb.data() << std::endl;
                sqstd_printcallstack(_vm);
                return false;
            }
            sq_pop(_vm, 2); //pops the roottable and the function
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

private:
    Engine &_engine;
    HSQUIRRELVM _vm;
    Object &_object;
    const std::string &_verb;
    bool _done{false};
};

class _DefaultVerbExecute : public VerbExecute
{
public:
    _DefaultVerbExecute(HSQUIRRELVM vm, Engine &engine)
        : _vm(vm), _engine(engine)
    {
    }

private:
    void execute(Object *pObject, const Verb *pVerb) override
    {
        auto obj = pObject->getTable();
        getVerb(obj, pVerb);
        if (!pVerb)
            return;

        if (callObjectPreWalk(obj, pVerb->id))
            return;

        auto pActor = _engine.getCurrentActor();
        if (!pActor)
            return;
        auto walk = std::make_unique<_ActorWalk>(*pActor, *pObject);
        auto verb = std::make_unique<_VerbExecute>(_engine, _vm, *pActor, *pObject, pVerb->func);
        auto postWalk = std::make_unique<_PostWalk>(_vm, obj, pVerb->id);
        auto sentence = std::make_unique<_CompositeFunction>();
        sentence->push_back(std::move(walk));
        sentence->push_back(std::move(verb));
        sentence->push_back(std::move(postWalk));
        _engine.addFunction(std::move(sentence));
    }

    void use(const InventoryObject *pObjectSource, Object *pObjectTarget) override
    {
        auto walk = std::make_unique<_ActorWalk>(*_engine.getCurrentActor(), *pObjectTarget);
        auto action = std::make_unique<_Use>(_vm, *_engine.getCurrentActor(), *pObjectSource, *pObjectTarget);
        auto sentence = std::make_unique<_CompositeFunction>();
        sentence->push_back(std::move(walk));
        sentence->push_back(std::move(action));
        _engine.addFunction(std::move(sentence));
    }

    void execute(const InventoryObject *pObject, const Verb *pVerb) override
    {
        HSQOBJECT obj = *(HSQOBJECT *)pObject->getHandle();

        getVerb(obj, pVerb);
        if (!pVerb)
            return;

        if (callObjectPreWalk(obj, pVerb->id))
            return;

        if (pVerb->id == 10 && useFlags(pObject))
            return;

        auto &roomTable = _engine.getRoom()->getTable();
        sq_pushobject(_vm, obj);
        sq_pushobject(_vm, roomTable);
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
            default:
                useFlag = UseFlag::None;
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

    bool callObjectPreWalk(HSQOBJECT obj, int verb)
    {
        sq_pushobject(_vm, obj);
        sq_pushstring(_vm, _SC("objectPreWalk"), -1);
        if (SQ_SUCCEEDED(sq_get(_vm, -2)))
        {
            sq_remove(_vm, -2);
            sq_pushobject(_vm, obj);
            sq_pushinteger(_vm, verb);
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
        int verb;
        if (pVerb)
        {
            verb = pVerb->id;
            pVerb = _engine.getVerb(verb);
            return;
        }
        auto defaultVerb = getDefaultVerb(obj);
        if (!defaultVerb)
            return;
        verb = defaultVerb;
        pVerb = _engine.getVerb(verb);
    }

    SQInteger getDefaultVerb(HSQOBJECT obj)
    {
        sq_pushobject(_vm, obj);
        sq_pushstring(_vm, _SC("defaultVerb"), -1);

        SQInteger defaultVerb = 0;
        if (SQ_SUCCEEDED(sq_get(_vm, -2)))
        {
            sq_getinteger(_vm, -1, &defaultVerb);
            std::cout << "defaultVerb: " << defaultVerb << std::endl;
        }
        return defaultVerb;
    }

private:
    HSQUIRRELVM _vm;
    Engine &_engine;
};
} // namespace ng
