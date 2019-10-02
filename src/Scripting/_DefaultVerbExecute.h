#pragma once

#include "../_Util.h"
#include "Engine.h"
#include "Logger.h"
#include "squirrel.h"

namespace ng
{
static void pushObject(HSQUIRRELVM vm, Entity *pObj)
{
    if (pObj)
    {
        sq_pushobject(vm, pObj->getTable());
        return;
    }
    sq_pushnull(vm);
}

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
    _ActorWalk(Actor &actor, const Entity *pEntity) : _actor(actor)
    {
        auto pos = pEntity->getRealPosition();
        auto usePos = pEntity->getUsePosition();
        auto dest = sf::Vector2f(pos.x + usePos.x, pos.y - usePos.y);
        auto facing = getFacing(pEntity);
        _actor.walkTo(dest, facing);
    }

  private:
    static Facing getFacing(const Entity *pEntity)
    {
        Facing facing;
        const auto pActor = dynamic_cast<const Actor *>(pEntity);
        if (pActor)
        {
            return getOppositeFacing(pActor->getCostume().getFacing());
        }
        const auto pObj = static_cast<const Object *>(pEntity);
        return _toFacing(pObj->getUseDirection());
    }

  private:
    bool isElapsed() override { return !_actor.isWalking(); }

  private:
    Actor &_actor;
};

class _PostWalk : public Function
{
  public:
    _PostWalk(HSQUIRRELVM v, Entity *pObject, Entity *pObject2, int verb)
        : _vm(v), _pObject(pObject), _pObject2(pObject2), _verb(verb)
    {
    }

    bool isElapsed() override { return _done; }

    void operator()(const sf::Time &elapsed) override
    {
        _done = true;
        Object *pObj = dynamic_cast<Object *>(_pObject);
        auto functionName = pObj ? "objectPostWalk" : "actorPostWalk";
        sq_pushobject(_vm, _pObject->getTable());
        sq_pushstring(_vm, functionName, -1);
        if (SQ_SUCCEEDED(sq_get(_vm, -2)))
        {
            sq_remove(_vm, -2);
            sq_pushobject(_vm, _pObject->getTable());
            sq_pushinteger(_vm, _verb);
            sq_pushobject(_vm, _pObject->getTable());
            pushObject(_vm, _pObject2);
            sq_call(_vm, 4, SQTrue, SQTrue);
            SQInteger handled = 0;
            sq_getinteger(_vm, -1, &handled);
            if (handled == 1)
            {
                _handled = true;
                return;
            }
        }
        sq_pop(_vm, 1);
        _handled = false;
    }

  private:
    HSQUIRRELVM _vm{};
    Entity *_pObject{nullptr};
    Entity *_pObject2{nullptr};
    int _verb{0};
    bool _done{false};
    bool _handled{false};
};

class _SetDefaultVerb : public Function
{
  public:
    explicit _SetDefaultVerb(Engine &engine) : _engine(engine) {}

    bool isElapsed() override { return _done; }

    void operator()(const sf::Time &elapsed) override
    {
        if (_done)
            return;

        _done = true;
        _engine.setDefaultVerb();
    }

  private:
    Engine &_engine;
    bool _done{false};
};

class _VerbExecute : public Function
{
  public:
    _VerbExecute(Engine &engine, HSQUIRRELVM v, Actor &actor, Entity &object, const std::string &verb)
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
                trace("failed to execute verb {}", _verb.data());
                sqstd_printcallstack(_vm);
                return;
            }
            sq_pop(_vm, 1);
            return;
        }
        sq_pop(_vm, 1);

        if (callVerbDefault(_object.getTable()))
            return;

        if (callDefaultObjectVerb())
            return;
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
                trace("failed to execute verb {}", _verb.data());
                sqstd_printcallstack(_vm);
                return false;
            }
            sq_pop(_vm, 2); // pops the roottable and the function
            return true;
        }

        return false;
    }

    bool callVerbDefault(HSQOBJECT obj)
    {
        sq_pushobject(_vm, obj);
        sq_pushstring(_vm, _SC("verbDefault"), -1);

        if (SQ_SUCCEEDED(sq_get(_vm, -2)))
        {
            sq_remove(_vm, -2);
            sq_pushobject(_vm, obj);
            sq_call(_vm, 1, SQFalse, SQTrue);
            sq_pop(_vm, 1);
            return true;
        }
        return false;
    }

  private:
    Engine &_engine;
    HSQUIRRELVM _vm;
    Entity &_object;
    const std::string &_verb;
    bool _done{false};
};

class _DefaultVerbExecute : public VerbExecute
{
  public:
    _DefaultVerbExecute(HSQUIRRELVM vm, Engine &engine) : _vm(vm), _engine(engine) {}

  private:
    void execute(const Verb *pVerb, Entity *pObject1, Entity *pObject2) override
    {
        auto obj = pObject1->getTable();
        getVerb(obj, pVerb);
        if (!pVerb)
            return;

        if (pVerb->id == VerbConstants::VERB_USE && useFlags(pObject1))
            return;

        if (callObjectOrActorPreWalk(pVerb->id, pObject1, pObject2))
            return;

        auto pActor = _engine.getCurrentActor();
        if (!pActor)
            return;
        auto sentence = std::make_unique<_CompositeFunction>();
        if ((pVerb->id != VerbConstants::VERB_LOOKAT || !isFarLook(obj)) && !isInInventory(pObject1))
        {
            auto walk = std::make_unique<_ActorWalk>(*pActor, pObject1);
            sentence->push_back(std::move(walk));
        }
        auto verb = std::make_unique<_VerbExecute>(_engine, _vm, *pActor, *pObject1, pVerb->func);
        sentence->push_back(std::move(verb));
        auto postWalk = std::make_unique<_PostWalk>(_vm, pObject1, pObject2, pVerb->id);
        sentence->push_back(std::move(postWalk));
        auto setDefaultVerb = std::make_unique<_SetDefaultVerb>(_engine);
        sentence->push_back(std::move(setDefaultVerb));
        _engine.addFunction(std::move(sentence));
    }

    bool isInInventory(Entity *pEntity)
    {
        auto pObj = dynamic_cast<Object *>(pEntity);
        if (!pObj) return false;
        return pObj->getOwner() != nullptr;
    }

    bool isFarLook(const HSQOBJECT &obj)
    {
        auto flags = getFlags(obj);
        return ((flags & 0x8) == 0x8);
    }

    bool useFlags(Entity *pObject)
    {
        HSQOBJECT obj = pObject->getTable();
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
                    return false;
            }
            _engine.setUseFlag(useFlag, pObject);
            return true;
        }
        return false;
    }

    int32_t getFlags(const HSQOBJECT &obj)
    {
        SQInteger flags = 0;
        sq_pushobject(_vm, obj);
        sq_pushstring(_vm, _SC("flags"), -1);
        if (SQ_SUCCEEDED(sq_get(_vm, -2)))
        {
            sq_getinteger(_vm, -1, &flags);
        }
        sq_pop(_vm, 2);
        return flags;
    }

    bool callObjectOrActorPreWalk(int verb, Entity *pObj1, Entity *pObj2)
    {
        Object *pObj = dynamic_cast<Object *>(pObj1);
        auto functionName = pObj ? "objectPreWalk" : "actorPreWalk";
        sq_pushobject(_vm, pObj1->getTable());
        sq_pushstring(_vm, functionName, -1);
        if (SQ_SUCCEEDED(sq_get(_vm, -2)))
        {
            sq_remove(_vm, -2);
            sq_pushobject(_vm, pObj1->getTable());
            sq_pushinteger(_vm, verb); // verb
            pushObject(pObj1);
            pushObject(pObj2);
            sq_call(_vm, 4, SQTrue, SQTrue);
            SQInteger handled = 0;
            sq_getinteger(_vm, -1, &handled);
            if (handled == 1)
                return true;
        }
        sq_pop(_vm, 1);
        return false;
    }

    void pushObject(Entity *pObj) { ng::pushObject(_vm, pObj); }

    void callVerbDefault(HSQOBJECT obj)
    {
        sq_pushobject(_vm, obj);
        sq_pushstring(_vm, _SC("verbDefault"), -1);

        if (SQ_SUCCEEDED(sq_get(_vm, -2)))
        {
            sq_remove(_vm, -2);
            sq_pushobject(_vm, obj);
            sq_call(_vm, 1, SQFalse, SQTrue);
            sq_pop(_vm, 2); // pops the roottable and the function
        }
    }

    void getVerb(const HSQOBJECT &obj, const Verb *&pVerb)
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

        if (SQ_SUCCEEDED(sq_get(_vm, -2)))
        {
            SQInteger defaultVerb = 0;
            sq_getinteger(_vm, -1, &defaultVerb);
            trace("defaultVerb: {}", defaultVerb);
            sq_pop(_vm, 2);
            return defaultVerb;
        }
        sq_pop(_vm, 1);

        return 2;
    }

  private:
    HSQUIRRELVM _vm;
    Engine &_engine;
};
} // namespace ng
