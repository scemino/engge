#pragma once
#include <squirrel3/squirrel.h>
#include "NGLip.h"
#include "NGEngine.h"
#include "_NGUtil.h"

namespace ng
{
class _ActorPack : public Pack
{
  private:
    static NGEngine *g_pEngine;

  private:
    void addTo(ScriptEngine &engine) const override
    {
        g_pEngine = &engine.getEngine();
        engine.registerGlobalFunction(actorAlpha, "actorAlpha");
        engine.registerGlobalFunction(actorAnimationNames, "actorAnimationNames");
        engine.registerGlobalFunction(actorAt, "actorAt");
        engine.registerGlobalFunction(actorColor, "actorColor");
        engine.registerGlobalFunction(actorCostume, "actorCostume");
        engine.registerGlobalFunction(actorDistanceTo, "actorDistanceTo");
        engine.registerGlobalFunction(actorDistanceWithin, "actorDistanceWithin");
        engine.registerGlobalFunction(actorFace, "actorFace");
        engine.registerGlobalFunction(actorHidden, "actorHidden");
        engine.registerGlobalFunction(actorHideLayer, "actorHideLayer");
        engine.registerGlobalFunction(actorInTrigger, "actorInTrigger");
        engine.registerGlobalFunction(actorInWalkbox, "actorInWalkbox");
        engine.registerGlobalFunction(actorLockFacing, "actorLockFacing");
        engine.registerGlobalFunction(actorBlinkRate, "actorBlinkRate");
        engine.registerGlobalFunction(actorPlayAnimation, "actorPlayAnimation");
        engine.registerGlobalFunction(actorPosX, "actorPosX");
        engine.registerGlobalFunction(actorPosY, "actorPosY");
        engine.registerGlobalFunction(actorRenderOffset, "actorRenderOffset");
        engine.registerGlobalFunction(actorRoom, "actorRoom");
        engine.registerGlobalFunction(actorShowLayer, "actorShowLayer");
        engine.registerGlobalFunction(actorTalking, "actorTalking");
        engine.registerGlobalFunction(actorTalkColors, "actorTalkColors");
        engine.registerGlobalFunction(actorTalkOffset, "actorTalkOffset");
        engine.registerGlobalFunction(actorUsePos, "actorUsePos");
        engine.registerGlobalFunction(actorUseWalkboxes, "actorUseWalkboxes");
        engine.registerGlobalFunction(actorWalking, "actorWalking");
        engine.registerGlobalFunction(actorWalkSpeed, "actorWalkSpeed");
        engine.registerGlobalFunction(actorWalkTo, "actorWalkTo");
        engine.registerGlobalFunction(actorWalkForward, "actorWalkForward");
        engine.registerGlobalFunction(createActor, "createActor");
        engine.registerGlobalFunction(currentActor, "currentActor");
        engine.registerGlobalFunction(isActor, "isActor");
        engine.registerGlobalFunction(isActor, "is_actor");
        engine.registerGlobalFunction(masterActorArray, "masterActorArray");
        engine.registerGlobalFunction(sayLine, "sayLine");
        engine.registerGlobalFunction(selectActor, "selectActor");
        engine.registerGlobalFunction(triggerActors, "triggerActors");
        engine.registerGlobalFunction(verbUIColors, "verbUIColors");
    }

    static SQInteger actorAlpha(HSQUIRRELVM v)
    {
        auto actor = ScriptEngine::getActor(v, 2);
        if (!actor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        SQFloat transparency;
        if (SQ_FAILED(sq_getfloat(v, 3, &transparency)))
        {
            return sq_throwerror(v, _SC("failed to get transparency"));
        }
        auto alpha = static_cast<sf::Uint8>(transparency * 255);
        actor->setColor(sf::Color(static_cast<sf::Uint32>(actor->getColor().toInteger() << 8 | alpha)));
        return 0;
    }

    static SQInteger actorAnimationNames(HSQUIRRELVM v)
    {
        const SQChar *head = nullptr;
        const SQChar *stand = nullptr;
        const SQChar *walk = nullptr;
        const SQChar *reach = nullptr;
        auto *pActor = ScriptEngine::getActor(v, 2);
        if (!pActor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        sq_getstring(v, 3, &head);
        sq_getstring(v, 4, &stand);
        sq_getstring(v, 5, &walk);
        sq_getstring(v, 6, &reach);
        pActor->getCostume().setAnimationNames(head ? head : "", stand ? stand : "", walk ? walk : "", reach ? reach : "");
        return 0;
    }

    static SQInteger actorAt(HSQUIRRELVM v)
    {
        auto numArgs = sq_gettop(v) - 1;
        if (numArgs == 2)
        {
            auto *pActor = ScriptEngine::getActor(v, 2);
            if (!pActor)
            {
                return sq_throwerror(v, _SC("failed to get actor"));
            }
            auto *pObj = ScriptEngine::getObject(v, 3);
            if (!pObj)
            {
                return sq_throwerror(v, _SC("failed to get object"));
            }
            auto usePos = pObj->getUsePosition();
            auto pos = pObj->getPosition();
            pos.x += usePos.x;
            pos.y -= usePos.y;
            auto pRoom = pObj->getRoom();
            pActor->setPosition(pos);
            pActor->setRoom(pRoom);
            return 0;
        }

        if (numArgs == 5)
        {
            auto *pActor = ScriptEngine::getActor(v, 2);
            if (!pActor)
            {
                return sq_throwerror(v, _SC("failed to get actor"));
            }
            auto *pRoom = ScriptEngine::getRoom(v, 3);
            if (!pRoom)
            {
                return sq_throwerror(v, _SC("failed to get roomTable"));
            }
            SQInteger x, y, dir;
            if (SQ_FAILED(sq_getinteger(v, 4, &x)))
            {
                return sq_throwerror(v, _SC("failed to get x"));
            }
            if (SQ_FAILED(sq_getinteger(v, 5, &y)))
            {
                return sq_throwerror(v, _SC("failed to get y"));
            }
            if (SQ_FAILED(sq_getinteger(v, 6, &dir)))
            {
                return sq_throwerror(v, _SC("failed to get direction"));
            }
            pActor->setPosition((sf::Vector2f)sf::Vector2i(x, y));
            pActor->getCostume().setFacing((Facing)dir);
            pActor->setRoom(pRoom);
            return 0;
        }

        return sq_throwerror(v, _SC("invalid number of arguments"));
    }

    static SQInteger actorColor(HSQUIRRELVM v)
    {
        auto actor = ScriptEngine::getActor(v, 2);
        if (!actor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        SQInteger color;
        if (SQ_FAILED(sq_getinteger(v, 3, &color)))
        {
            return sq_throwerror(v, _SC("failed to get fps"));
        }
        auto alpha = actor->getColor().toInteger() & 0x000000FF;
        actor->setColor(sf::Color(static_cast<sf::Uint32>(color << 8 | alpha)));
        return 0;
    }

    static SQInteger actorCostume(HSQUIRRELVM v)
    {
        const SQChar *name;
        auto actor = ScriptEngine::getActor(v, 2);
        if (!actor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        if (SQ_FAILED(sq_getstring(v, 3, &name)))
        {
            return sq_throwerror(v, _SC("failed to get name"));
        }
        const SQChar *pSheet = nullptr;
        sq_getstring(v, 4, &pSheet);
        actor->setCostume(name, pSheet ? pSheet : "");
        return 0;
    }

    static SQInteger actorDistanceTo(HSQUIRRELVM v)
    {
        auto actor = ScriptEngine::getActor(v, 2);
        if (!actor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        auto object = ScriptEngine::getObject(v, 3);
        if (!object)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        auto pos = actor->getPosition() - object->getPosition();
        auto dist = sqrt(pos.x * pos.x + pos.y * pos.y);
        sq_pushinteger(v, dist);
        return 1;
    }

    static SQInteger actorDistanceWithin(HSQUIRRELVM v)
    {
        auto actor = ScriptEngine::getActor(v, 2);
        if (!actor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        auto object = ScriptEngine::getObject(v, 3);
        if (!object)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        SQInteger distance;
        if (SQ_FAILED(sq_getinteger(v, 4, &distance)))
        {
            return sq_throwerror(v, _SC("failed to get distance"));
        }
        auto pos = actor->getPosition() - object->getPosition();
        auto dist = sqrt(pos.x * pos.x + pos.y * pos.y);
        sq_pushbool(v, dist < distance);
        return 1;
    }

    static SQInteger actorFace(HSQUIRRELVM v)
    {
        auto actor = ScriptEngine::getActor(v, 2);
        if (!actor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        SQInteger dir;
        if (SQ_FAILED(sq_getinteger(v, 3, &dir)))
        {
            return sq_throwerror(v, _SC("failed to get direction"));
        }
        actor->getCostume().setFacing((Facing)dir);
        return 0;
    }

    static SQInteger actorHidden(HSQUIRRELVM v)
    {
        auto *pActor = ScriptEngine::getActor(v, 2);
        if (!pActor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        pActor->setVisible(false);
        return 0;
    }

    static SQInteger actorShowHideLayer(HSQUIRRELVM v, bool isVisible)
    {
        auto *pActor = ScriptEngine::getActor(v, 2);
        if (!pActor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        const SQChar *layerName;
        if (SQ_FAILED(sq_getstring(v, 3, &layerName)))
        {
            return sq_throwerror(v, _SC("failed to get layerName"));
        }
        pActor->getCostume().setLayerVisible(layerName, isVisible);
        return 0;
    }

    static SQInteger actorHideLayer(HSQUIRRELVM v)
    {
        return actorShowHideLayer(v, false);
    }

    static SQInteger actorInTrigger(HSQUIRRELVM v)
    {
        auto *actor = ScriptEngine::getActor(v, 2);
        if (!actor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        auto *object = ScriptEngine::getObject(v, 3);
        if (!object)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        bool isInside = object->getRealHotspot().contains((sf::Vector2i)(actor->getPosition()));
        sq_pushbool(v, isInside);
        return 1;
    }

    static SQInteger actorInWalkbox(HSQUIRRELVM v)
    {
        auto *actor = ScriptEngine::getActor(v, 2);
        if (!actor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }

        const auto &walkboxes = g_pEngine->getRoom().getWalkboxes();
        auto inWalkbox = std::any_of(std::begin(walkboxes), std::end(walkboxes), [actor](const std::unique_ptr<Walkbox> &w) {
            return w->contains(actor->getPosition());
        });

        sq_pushbool(v, inWalkbox ? SQTrue : SQFalse);
        return 1;
    }

    static SQInteger actorLockFacing(HSQUIRRELVM v)
    {
        SQInteger facing;
        NGActor *actor = ScriptEngine::getActor(v, 2);
        if (!actor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        if (SQ_FAILED(sq_getinteger(v, 3, &facing)))
        {
            return sq_throwerror(v, _SC("failed to get facing"));
        }
        actor->getCostume().lockFacing((Facing)facing);
        return 0;
    }

    static SQInteger actorBlinkRate(HSQUIRRELVM v)
    {
        NGActor *pActor = ScriptEngine::getActor(v, 2);
        if (!pActor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        SQFloat min = 0;
        if (SQ_FAILED(sq_getfloat(v, 3, &min)))
        {
            return sq_throwerror(v, _SC("failed to get min"));
        }
        SQFloat max = 0;
        if (SQ_FAILED(sq_getfloat(v, 4, &max)))
        {
            return sq_throwerror(v, _SC("failed to get max"));
        }
        // TODO: blink rate
        return 0;
    }

    static SQInteger actorPlayAnimation(HSQUIRRELVM v)
    {
        NGActor *pActor = ScriptEngine::getActor(v, 2);
        if (!pActor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        const SQChar *animation = nullptr;
        if (SQ_FAILED(sq_getstring(v, 3, &animation)))
        {
            return sq_throwerror(v, _SC("failed to get animation"));
        }
        SQBool loop = false;
        sq_getbool(v, 4, &loop);
        std::cout << "Play anim " << animation << (loop ? " (loop)" : "") << std::endl;
        pActor->getCostume().setState(animation);
        pActor->getCostume().getAnimation()->play(loop);
        return 0;
    }

    static SQInteger actorPosX(HSQUIRRELVM v)
    {
        auto *pActor = ScriptEngine::getActor(v, 2);
        if (!pActor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        sq_pushinteger(v, pActor->getPosition().x);
        return 1;
    }

    static SQInteger actorPosY(HSQUIRRELVM v)
    {
        auto *pActor = ScriptEngine::getActor(v, 2);
        if (!pActor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        sq_pushinteger(v, pActor->getPosition().y);
        return 1;
    }

    static SQInteger actorRenderOffset(HSQUIRRELVM v)
    {
        auto *pActor = ScriptEngine::getActor(v, 2);
        if (!pActor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        SQInteger x, y;
        if (SQ_FAILED(sq_getinteger(v, 3, &x)))
        {
            return sq_throwerror(v, _SC("failed to get x"));
        }
        if (SQ_FAILED(sq_getinteger(v, 4, &y)))
        {
            return sq_throwerror(v, _SC("failed to get y"));
        }
        pActor->setRenderOffset(sf::Vector2i(x, y));
        return 0;
    }

    static SQInteger actorRoom(HSQUIRRELVM v)
    {
        auto *pActor = ScriptEngine::getActor(v, 2);
        // TODO: sq_pushobject(v, *pRoomTable);
        sq_pushnull(v);
        return 1;
    }

    static SQInteger actorShowLayer(HSQUIRRELVM v)
    {
        return actorShowHideLayer(v, true);
    }

    // TODO: static SQInteger _actorSlotSelectable(HSQUIRRELVM v)
    static SQInteger actorTalking(HSQUIRRELVM v)
    {
        // TODO: with no actor specified
        auto actor = ScriptEngine::getActor(v, 2);
        if (!actor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        sq_pushbool(v, actor->isTalking());
        return 1;
    }

    // TODO: static SQInteger _actorStopWalking(HSQUIRRELVM v)

    static SQInteger actorTalkColors(HSQUIRRELVM v)
    {
        auto actor = ScriptEngine::getActor(v, 2);
        if (!actor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        SQInteger color;
        if (SQ_FAILED(sq_getinteger(v, 3, &color)))
        {
            return sq_throwerror(v, _SC("failed to get fps"));
        }
        actor->setTalkColor(sf::Color(static_cast<sf::Uint32>(color << 8 | 0xff)));
        return 0;
    }

    static SQInteger actorTalkOffset(HSQUIRRELVM v)
    {
        NGActor *actor = ScriptEngine::getActor(v, 2);
        if (!actor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        SQInteger x;
        if (SQ_FAILED(sq_getinteger(v, 3, &x)))
        {
            return sq_throwerror(v, _SC("failed to get x"));
        }
        SQInteger y;
        if (SQ_FAILED(sq_getinteger(v, 4, &y)))
        {
            return sq_throwerror(v, _SC("failed to get y"));
        }
        actor->setTalkOffset(sf::Vector2i(x, y));
        return 0;
    }
    // TODO: static SQInteger _actorTurnTo(HSQUIRRELVM v)

    static SQInteger actorUsePos(HSQUIRRELVM v)
    {
        NGActor *actor = ScriptEngine::getActor(v, 2);
        if (!actor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        NGObject *obj = ScriptEngine::getObject(v, 3);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        auto pos = obj->getUsePosition();
        // TODO: actor->setUsePosition(pos);
        return 0;
    }

    static SQInteger actorUseWalkboxes(HSQUIRRELVM v)
    {
        NGActor *actor = ScriptEngine::getActor(v, 2);
        if (!actor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        SQBool use;
        if (SQ_FAILED(sq_getbool(v, 3, &use)))
        {
            return sq_throwerror(v, _SC("failed to get useWalkboxes"));
        }
        actor->useWalkboxes(use);
        return 0;
    }

    static SQInteger actorWalkForward(HSQUIRRELVM v)
    {
        NGActor *actor = ScriptEngine::getActor(v, 2);
        if (!actor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        SQInteger distance;
        if (SQ_FAILED(sq_getinteger(v, 3, &distance)))
        {
            return sq_throwerror(v, _SC("failed to get distance"));
        }
        actor->walkTo(actor->getPosition() + sf::Vector2f(distance, 0));
        return 0;
    }

    static SQInteger actorWalking(HSQUIRRELVM v)
    {
        auto numArgs = sq_gettop(v) - 1;
        NGActor *pActor = nullptr;
        if (numArgs == 0)
        {
            pActor = g_pEngine->getCurrentActor();
        }
        else if (numArgs == 1)
        {
            pActor = ScriptEngine::getActor(v, 2);
        }
        if (!pActor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        sq_pushroottable(v);
        sq_pushbool(v, pActor->isWalking() ? SQTrue : SQFalse);
        return 1;
    }

    static SQInteger actorWalkSpeed(HSQUIRRELVM v)
    {
        auto pActor = ScriptEngine::getActor(v, 2);
        if (!pActor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        SQInteger x, y;
        if (SQ_FAILED(sq_getinteger(v, 3, &x)))
        {
            return sq_throwerror(v, _SC("failed to get x"));
        }
        if (SQ_FAILED(sq_getinteger(v, 4, &y)))
        {
            return sq_throwerror(v, _SC("failed to get y"));
        }
        pActor->setWalkSpeed(sf::Vector2i(x, y));
        return 0;
    }

    static SQInteger actorWalkTo(HSQUIRRELVM v)
    {
        auto *pActor = ScriptEngine::getActor(v, 2);
        if (!pActor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        auto *pObject = ScriptEngine::getObject(v, 3);
        if (!pObject)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }

        auto pos = pObject->getPosition();
        auto usePos = pObject->getUsePosition();

        pActor->walkTo(sf::Vector2f(pos.x + usePos.x, pos.y - usePos.y), _toFacing(pObject->getUseDirection()));

        return 0;
    }

    // TODO: static SQInteger addSelectableActor(HSQUIRRELVM v)

    static SQInteger createActor(HSQUIRRELVM v)
    {
        HSQOBJECT table;
        sq_resetobject(&table);
        sq_getstackobj(v, 2, &table);

        sq_pushobject(v, table);
        sq_pushstring(v, _SC("_key"), 4);
        if (SQ_FAILED(sq_get(v, -2)))
        {
            return sq_throwerror(v, _SC("can't find _key entry"));
        }
        const SQChar *key;
        if (SQ_FAILED(sq_getstring(v, -1, &key)))
        {
            return sq_throwerror(v, _SC("can't find _key entry"));
        }
        sq_pop(v, 2);

        // define instance
        auto pActor = std::make_unique<NGActor>(*g_pEngine);
        pActor->setName(key);
        // pActor->setTable(table);
        sq_pushobject(v, table);
        sq_pushstring(v, _SC("instance"), -1);
        sq_pushuserpointer(v, pActor.get());
        sq_newslot(v, -3, SQFalse);

        g_pEngine->addActor(std::move(pActor));
        return 1;
    }

    static SQInteger currentActor(HSQUIRRELVM v)
    {
        auto actor = g_pEngine->getCurrentActor();
        if (!actor)
        {
            sq_pushnull(v);
            return 1;
        }
        ScriptEngine::pushObject(v, *actor);
        return 1;
    }

    static SQInteger isActor(HSQUIRRELVM v)
    {
        auto actor = ScriptEngine::getActor(v, 2);
        sq_pushbool(v, actor ? SQTrue : SQFalse);
        return 1;
    }

    // TODO: static SQInteger isActorOnScreen(HSQUIRRELVM v)
    static SQInteger masterActorArray(HSQUIRRELVM v)
    {
        auto &actors = g_pEngine->getActors();
        sq_newarray(v, 0);
        for (auto &actor : actors)
        {
            // TODO:
            // sq_pushobject(v, actor->getTable());
            sq_arrayappend(v, -2);
        }
        return 1;
    }
    // TODO: static SQInteger mumbleLine(HSQUIRRELVM v)

    static std::string str_toupper(std::string s)
    {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) { return std::toupper(c); } // correct
        );
        return s;
    }

    static SQInteger sayLine(HSQUIRRELVM v)
    {
        NGActor *actor = ScriptEngine::getActor(v, 2);
        if (!actor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }

        auto numIds = sq_gettop(v) - 4;
        for (int i = 0; i < numIds; i++)
        {
            const SQChar *idText;
            if (SQ_FAILED(sq_getstring(v, 3 + i, &idText)))
            {
                return sq_throwerror(v, _SC("failed to get text"));
            }

            std::string s(idText);
            s = s.substr(1);
            auto id = std::strtol(s.c_str(), nullptr, 10);
            actor->say(id);
        }
        return 0;
    }

    static SQInteger selectActor(HSQUIRRELVM v)
    {
        auto *actor = ScriptEngine::getActor(v, 2);
        if (!actor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        g_pEngine->setCurrentActor(actor);
        return 0;
    }

    // TODO: static SQInteger stopTalking(HSQUIRRELVM v)

    static SQInteger triggerActors(HSQUIRRELVM v)
    {
        auto *object = ScriptEngine::getObject(v, 2);
        if (!object)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        sq_newarray(v, 0);
        for (const auto &actor : g_pEngine->getActors())
        {
            if (object->getRealHotspot().contains((sf::Vector2i)actor->getPosition()))
            {
                // TODO:
                // sq_pushobject(v, actor->getTable());
                sq_arrayappend(v, -2);
            }
        }
        return 1;
    }

    static sf::Color fromRgbInt(SQInteger color)
    {
        sf::Color c((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
        return c;
    }

    static SQInteger readFieldInt(HSQUIRRELVM v, const SQChar *name, SQInteger &field)
    {
        sq_pushstring(v, name, -1);
        if (SQ_FAILED(sq_get(v, -2)))
        {
            sq_pop(v, 2);
            return SQ_ERROR;
        }

        field = 0;
        if (SQ_FAILED(sq_getinteger(v, -1, &field)))
        {
            sq_pop(v, 2);
            return SQ_ERROR;
        }
        sq_pop(v, 1);
        return SQ_OK;
    }

    static SQInteger verbUIColors(HSQUIRRELVM v)
    {
        SQInteger actorSlot;
        if (SQ_FAILED(sq_getinteger(v, 2, &actorSlot)))
        {
            return sq_throwerror(v, _SC("failed to get actor slot"));
        }

        HSQOBJECT table;
        if (SQ_FAILED(sq_getstackobj(v, 3, &table)))
        {
            return sq_throwerror(v, _SC("failed to get verb definitionTable"));
        }
        if (!sq_istable(table))
        {
            const SQChar *tmp;
            sq_getstring(v, 3, &tmp);
            return sq_throwerror(v, _SC("failed to get verb definitionTable"));
        }

        sq_pushobject(v, table);

        // sentence
        SQInteger sentence = 0;
        if (SQ_FAILED(readFieldInt(v, _SC("sentence"), sentence)))
        {
            return sq_throwerror(v, _SC("failed to get sentence"));
        }

        SQInteger verbNormal = 0;
        if (SQ_FAILED(readFieldInt(v, _SC("verbNormal"), verbNormal)))
        {
            return sq_throwerror(v, _SC("failed to get verbNormal"));
        }

        SQInteger verbNormalTint = 0;
        if (SQ_FAILED(readFieldInt(v, _SC("verbNormalTint"), verbNormalTint)))
        {
            return sq_throwerror(v, _SC("failed to get verbNormal"));
        }

        SQInteger verbHighlight = 0;
        if (SQ_FAILED(readFieldInt(v, _SC("verbHighlight"), verbHighlight)))
        {
            return sq_throwerror(v, _SC("failed to get verbHighlight"));
        }

        SQInteger verbHighlightTint = 0;
        if (SQ_FAILED(readFieldInt(v, _SC("verbHighlightTint"), verbHighlightTint)))
        {
            return sq_throwerror(v, _SC("failed to get verbHighlightTint"));
        }

        // TODO:
        SQInteger dialogNormal = verbNormal;
        // if (SQ_FAILED(readFieldInt(v, _SC("dialogNormal"), dialogNormal)))
        // {
        //     dialogNormal = verbNormal;
        // }

        SQInteger dialogHighlight = verbHighlight;
        // if (SQ_FAILED(readFieldInt(v, _SC("dialogHighlight"), dialogHighlight)))
        // {
        //     dialogHighlight = verbHighlight;
        // }

        SQInteger inventoryFrame = 0;
        if (SQ_FAILED(readFieldInt(v, _SC("inventoryFrame"), inventoryFrame)))
        {
            return sq_throwerror(v, _SC("failed to get inventoryFrame"));
        }

        SQInteger inventoryBackground = 0;
        if (SQ_FAILED(readFieldInt(v, _SC("inventoryBackground"), inventoryBackground)))
        {
            return sq_throwerror(v, _SC("failed to get inventoryBackground"));
        }

        sq_pop(v, 2);

        VerbUiColors colors;
        colors.sentence = fromRgbInt(sentence);
        colors.verbNormal = fromRgbInt(verbNormal);
        colors.verbNormalTint = fromRgbInt(verbNormalTint);
        colors.verbHighlight = fromRgbInt(verbHighlight);
        colors.verbHighlightTint = fromRgbInt(verbHighlightTint);
        colors.dialogNormal = fromRgbInt(dialogNormal);
        colors.dialogHighlight = fromRgbInt(dialogHighlight);
        colors.inventoryFrame = fromRgbInt(inventoryFrame);
        colors.inventoryBackground = fromRgbInt(inventoryBackground);
        g_pEngine->setVerbUiColors(actorSlot - 1, colors);
        return 0;
    }
};

NGEngine *_ActorPack::g_pEngine = nullptr;

} // namespace ng
