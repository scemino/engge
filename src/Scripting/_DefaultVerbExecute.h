#pragma once

#include "../_Util.h"
#include "Engine.h"
#include "Logger.h"
#include "Sentence.h"
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
    _PostWalk(Sentence& sentence, HSQUIRRELVM v, Entity *pObject, Entity *pObject2, int verb)
        : _sentence(sentence), _vm(v), _pObject(pObject), _pObject2(pObject2), _verb(verb)
    {
    }

    bool isElapsed() override { return _done; }

    void operator()(const sf::Time &elapsed) override
    {
        Object *pObj = dynamic_cast<Object *>(_pObject);
        auto functionName = pObj ? "objectPostWalk" : "actorPostWalk";
        bool handled = false;
        ScriptEngine::callFunc(handled, _pObject, functionName, _verb, _pObject, _pObject2);
        if(handled)
        {
            _sentence.stop();
        }
        _done = true;
    }

  private:
    Sentence& _sentence;
    HSQUIRRELVM _vm{};
    Entity *_pObject{nullptr};
    Entity *_pObject2{nullptr};
    int _verb{0};
    bool _done{false};
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
    _VerbExecute(Engine &engine, HSQUIRRELVM v, Actor &actor, Entity &object, Entity* pObject2, const Verb *pVerb)
        : _engine(engine), _vm(v), _actor(actor), _object(object), _pObject2(pObject2), _pVerb(pVerb)
    {
    }

  private:
    bool isElapsed() override { return _done; }

    void operator()(const sf::Time &elapsed) override
    {
        _done = true;
        
        sq_pushobject(_vm, _object.getTable());
        sq_pushstring(_vm, _pVerb->func.data(), -1);

        if (SQ_SUCCEEDED(sq_rawget(_vm, -2)))
        {
            sq_remove(_vm, -2);
            sq_pushobject(_vm, _object.getTable());
            if(_pObject2)
            {
                sq_pushobject(_vm, _pObject2->getTable());
            }
            if (SQ_FAILED(sq_call(_vm, _pObject2 ? 2: 1, SQFalse, SQTrue)))
            {
                trace("failed to execute verb {}", _pVerb->func.data());
                sqstd_printcallstack(_vm);
                return;
            }
            sq_pop(_vm, 1);
            onPickup();
            return;
        }
        sq_pop(_vm, 1);

        if(_pVerb->id == VerbConstants::VERB_GIVE)
        {
            ScriptEngine::call("objectGive", &_object, &_actor, _pObject2);

            Object* pObject = dynamic_cast<Object*>(&_object);
            Actor* pActor2 = dynamic_cast<Actor*>(_pObject2);
            _actor.giveTo(pObject, pActor2);
            return;
        }

        if (callVerbDefault(&_object))
            return;

        if (callDefaultObjectVerb())
            return;
    }

    void onPickup()
    {
        if(_pVerb->id != VerbConstants::VERB_PICKUP) 
            return;

        ScriptEngine::call("onPickup", &_actor, &_object);
    }

    bool callDefaultObjectVerb()
    {
        auto &obj = _engine.getDefaultObject();
        auto pActor = _engine.getCurrentActor();

        sq_pushobject(_vm, obj);
        sq_pushstring(_vm, _pVerb->func.data(), -1);

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
                trace("failed to execute verb {}", _pVerb->func.data());
                sqstd_printcallstack(_vm);
                return false;
            }
            sq_pop(_vm, 2); // pops the roottable and the function
            return true;
        }

        return false;
    }

    bool callVerbDefault(Entity* pEntity)
    {
        sq_pushobject(_vm, pEntity->getTable());
        sq_pushstring(_vm, _SC("verbDefault"), -1);

        if (SQ_SUCCEEDED(sq_rawget(_vm, -2)))
        {
            sq_remove(_vm, -2);
            sq_pushobject(_vm, pEntity->getTable());
            sq_call(_vm, 1, SQFalse, SQTrue);
            sq_pop(_vm, 1);
            return true;
        }
        return false;
    }

  private:
    Engine &_engine;
    const Verb *_pVerb{nullptr};
    HSQUIRRELVM _vm{};
    Entity &_object;
    Entity* _pObject2{nullptr};
    Actor& _actor;
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
        getVerb(pObject1, pVerb);
        if (!pVerb)
            return;

        if (pVerb->id == VerbConstants::VERB_USE && !pObject2 && useFlags(pObject1))
            return;

        if(pVerb->id == VerbConstants::VERB_GIVE && !pObject2)
        {
            _engine.setUseFlag(UseFlag::GiveTo, pObject1);
            return;
        }

        if (callObjectOrActorPreWalk(pVerb->id, pObject1, pObject2))
            return;

        auto pActor = _engine.getCurrentActor();
        if (!pActor)
            return;
        Entity* pObj = pObject2 ? pObject2 : pObject1;
        auto sentence = std::make_unique<Sentence>();
        if ((pVerb->id != VerbConstants::VERB_LOOKAT || !isFarLook(obj)) && 
            !pObj->isInventoryObject())
        {
            auto walk = std::make_unique<_ActorWalk>(*pActor, pObj);
            sentence->push_back(std::move(walk));
        }
        auto postWalk = std::make_unique<_PostWalk>(*sentence, _vm, pObject1, pObject2, pVerb->id);
        sentence->push_back(std::move(postWalk));
        auto verb = std::make_unique<_VerbExecute>(_engine, _vm, *pActor, *pObject1, pObject2, pVerb);
        sentence->push_back(std::move(verb));
        auto setDefaultVerb = std::make_unique<_SetDefaultVerb>(_engine);
        sentence->push_back(std::move(setDefaultVerb));
        _engine.setSentence(std::move(sentence));
    }

    bool isFarLook(HSQOBJECT obj)
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
                case ObjectFlagConstants::USE_WITH:
                    useFlag = UseFlag::UseWith;
                    break;
                case ObjectFlagConstants::USE_ON:
                    useFlag = UseFlag::UseOn;
                    break;
                case ObjectFlagConstants::USE_IN:
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

    int32_t getFlags(HSQOBJECT obj)
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
        bool handled = false;
        ScriptEngine::callFunc(handled, pObj1, functionName, verb, pObj1, pObj2);
        return handled;
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

    void getVerb(Entity* pObj, const Verb *&pVerb)
    {
        int verb;
        if (pVerb)
        {
            verb = pVerb->id;
            pVerb = _engine.getVerb(verb);
            return;
        }
        auto defaultVerb = getDefaultVerb(pObj);
        if (!defaultVerb)
            return;
        verb = defaultVerb;
        pVerb = _engine.getVerb(verb);
    }

    SQInteger getDefaultVerb(Entity* pObj)
    {
        sq_pushobject(_vm, pObj->getTable());
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
