#pragma once
#include "squirrel.h"
#include "Lip.h"
#include "Engine.h"
#include "../_NGUtil.h"

namespace ng
{
class _ActorPack : public Pack
{
  private:
    static Engine *g_pEngine;

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
        engine.registerGlobalFunction(actorSlotSelectable, "actorSlotSelectable");
        engine.registerGlobalFunction(actorStopWalking, "actorStopWalking");
        engine.registerGlobalFunction(actorTalking, "actorTalking");
        engine.registerGlobalFunction(actorTalkColors, "actorTalkColors");
        engine.registerGlobalFunction(actorTalkOffset, "actorTalkOffset");
        engine.registerGlobalFunction(actorTurnTo, "actorTurnTo");
        engine.registerGlobalFunction(actorUsePos, "actorUsePos");
        engine.registerGlobalFunction(actorUseWalkboxes, "actorUseWalkboxes");
        engine.registerGlobalFunction(actorVolume, "actorVolume");
        engine.registerGlobalFunction(actorWalking, "actorWalking");
        engine.registerGlobalFunction(actorWalkSpeed, "actorWalkSpeed");
        engine.registerGlobalFunction(actorWalkTo, "actorWalkTo");
        engine.registerGlobalFunction(actorWalkForward, "actorWalkForward");
        engine.registerGlobalFunction(addSelectableActor, "addSelectableActor");
        engine.registerGlobalFunction(createActor, "createActor");
        engine.registerGlobalFunction(flashSelectableActor, "flashSelectableActor");
        engine.registerGlobalFunction(isActor, "isActor");
        engine.registerGlobalFunction(isActorOnScreen, "isActorOnScreen");
        engine.registerGlobalFunction(isActor, "is_actor");
        engine.registerGlobalFunction(masterActorArray, "masterActorArray");
        engine.registerGlobalFunction(mumbleLine, "mumbleLine");
        engine.registerGlobalFunction(sayLine, "sayLine");
        engine.registerGlobalFunction(selectActor, "selectActor");
        engine.registerGlobalFunction(stopTalking, "stopTalking");
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

            HSQOBJECT object;
            sq_resetobject(&object);
            if (SQ_FAILED(sq_getstackobj(v, 3, &object)))
            {
                return sq_throwerror(v, _SC("failed to get object or room"));
            }

            sq_pushobject(v, object);
            sq_pushstring(v, _SC("_type"), -1);
            SQInteger type = 0;
            if (SQ_SUCCEEDED(sq_rawget(v, -2)))
            {
                sq_remove(v, -2);
                if (SQ_FAILED(sq_getinteger(v, -1, &type)))
                {
                    return sq_throwerror(v, _SC("failed to get object or room"));
                }
            }

            if (type == Room::RoomType)
            {
                auto *pRoom = ScriptEngine::getRoom(v, 3);
                if (!pRoom)
                {
                    return sq_throwerror(v, _SC("failed to get room"));
                }
                pActor->setRoom(pRoom);
                return 0;
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
            pActor->setRoom(pRoom);
            pActor->setPosition(pos);
            return 0;
        }

        if (numArgs >= 4)
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
            SQInteger x, y, dir = 0;
            if (SQ_FAILED(sq_getinteger(v, 4, &x)))
            {
                return sq_throwerror(v, _SC("failed to get x"));
            }
            if (SQ_FAILED(sq_getinteger(v, 5, &y)))
            {
                return sq_throwerror(v, _SC("failed to get y"));
            }
            if (numArgs == 5 && SQ_FAILED(sq_getinteger(v, 6, &dir)))
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

    static SQInteger flashSelectableActor(HSQUIRRELVM v)
    {
        std::cerr << "TODO: flashSelectableActor: not implemented" << std::endl;
        return 0;
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

        auto inWalkbox = g_pEngine->getRoom().inWalkbox(actor->getPosition());
        sq_pushbool(v, inWalkbox ? SQTrue : SQFalse);
        return 1;
    }

    static SQInteger actorLockFacing(HSQUIRRELVM v)
    {
        SQInteger facing;
        Actor *actor = ScriptEngine::getActor(v, 2);
        if (!actor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        HSQOBJECT obj;
        sq_getstackobj(v, 3, &obj);
        if (sq_istable(obj))
        {
            std::cerr << "TODO: actorLockFacing with table" << std::endl;
            return 0;
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
        Actor *pActor = ScriptEngine::getActor(v, 2);
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
        Actor *pActor = ScriptEngine::getActor(v, 2);
        if (!pActor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        const SQChar *animation = nullptr;
        if (SQ_FAILED(sq_getstring(v, 3, &animation)))
        {
            return sq_throwerror(v, _SC("failed to get animation"));
        }
        SQInteger loop = 0;
        sq_getinteger(v, 4, &loop);
        std::cout << "Play anim " << animation << (loop == 1 ? " (loop)" : "") << std::endl;
        pActor->getCostume().setState(animation);
        auto pAnim = pActor->getCostume().getAnimation();
        if (pAnim)
        {
            pAnim->play(loop == 1);
        }
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

    static SQInteger actorSlotSelectable(HSQUIRRELVM v)
    {
        auto numArgs = sq_gettop(v) - 1;
        Actor *actor = nullptr;

        SQBool selectable;
        sq_tobool(v, numArgs + 1, &selectable);

        if (numArgs == 2)
        {
            SQInteger actorIndex = 0;
            if (SQ_SUCCEEDED(sq_getinteger(v, 2, &actorIndex)))
            {
                g_pEngine->actorSlotSelectable(actorIndex, selectable == SQTrue);
                return 0;
            }

            actor = ScriptEngine::getActor(v, 2);
            if (!actor)
            {
                return sq_throwerror(v, _SC("failed to get actor"));
            }
        }
        else
        {
            actor = g_pEngine->getCurrentActor();
        }
        if (!actor)
            return 0;

        g_pEngine->actorSlotSelectable(actor, selectable == SQTrue);
        return 0;
    }

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

    static SQInteger actorStopWalking(HSQUIRRELVM v)
    {
        auto actor = ScriptEngine::getActor(v, 2);
        if (!actor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        actor->stopWalking();
        return 0;
    }

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
        Actor *actor = ScriptEngine::getActor(v, 2);
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

    static SQInteger actorTurnTo(HSQUIRRELVM v)
    {
        std::cerr << "TODO: actorTurnTo: not implemented" << std::endl;
        return 0;
    }

    static SQInteger actorUsePos(HSQUIRRELVM v)
    {
        Actor *actor = ScriptEngine::getActor(v, 2);
        if (!actor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        Object *obj = ScriptEngine::getObject(v, 3);
        if (!obj)
        {
            return sq_throwerror(v, _SC("failed to get object"));
        }
        auto pos = obj->getUsePosition();
        actor->setUsePosition(pos);
        return 0;
    }

    static SQInteger actorUseWalkboxes(HSQUIRRELVM v)
    {
        Actor *actor = ScriptEngine::getActor(v, 2);
        if (!actor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        SQBool use;
        sq_tobool(v, 3, &use);

        actor->useWalkboxes(use);
        return 0;
    }

    static SQInteger actorVolume(HSQUIRRELVM v)
    {
        Actor *actor = ScriptEngine::getActor(v, 2);
        if (!actor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        SQFloat volume;
        if (SQ_FAILED(sq_getfloat(v, 3, &volume)))
        {
            return sq_throwerror(v, _SC("failed to get volume"));
        }
        actor->setVolume(volume);
        return 0;
    }

    static SQInteger actorWalkForward(HSQUIRRELVM v)
    {
        Actor *actor = ScriptEngine::getActor(v, 2);
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
        Actor *pActor = nullptr;
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

    static SQInteger addSelectableActor(HSQUIRRELVM v)
    {
        SQInteger slot;
        if (SQ_FAILED(sq_getinteger(v, 2, &slot)))
        {
            return sq_throwerror(v, _SC("failed to get slot"));
        }
        auto *pActor = ScriptEngine::getActor(v, 3);
        g_pEngine->addSelectableActor(slot, pActor);
        return 0;
    }

    static SQInteger createActor(HSQUIRRELVM v)
    {
        auto pActor = std::make_unique<Actor>(*g_pEngine);
        auto &table = pActor->getTable();
        sq_resetobject(&table);
        sq_getstackobj(v, 2, &table);
        sq_addref(v, &table);

        sq_pushobject(v, table);
        sq_pushstring(v, _SC("_key"), 4);
        const SQChar *key = nullptr;
        if (SQ_SUCCEEDED(sq_get(v, -2)))
        {
            if (SQ_FAILED(sq_getstring(v, -1, &key)))
            {
                return sq_throwerror(v, _SC("can't find _key entry"));
            }
            sq_pop(v, 2);
        }

        // define instance
        const SQChar *icon = nullptr;
        sq_pushobject(v, table);
        sq_pushstring(v, _SC("icon"), 4);
        if (SQ_SUCCEEDED(sq_get(v, -2)))
        {
            sq_getstring(v, -1, &icon);
            pActor->setIcon(icon);
        }

        sq_pop(v, 2);

        if (key)
        {
            pActor->setName(key);
        }
        sq_pushobject(v, table);
        sq_pushstring(v, _SC("instance"), -1);
        sq_pushuserpointer(v, pActor.get());
        sq_newslot(v, -3, SQFalse);

        g_pEngine->addActor(std::move(pActor));
        return 1;
    }

    static SQInteger isActor(HSQUIRRELVM v)
    {
        auto actor = ScriptEngine::getActor(v, 2);
        sq_pushbool(v, actor ? SQTrue : SQFalse);
        return 1;
    }

    static SQInteger isActorOnScreen(HSQUIRRELVM v)
    {
        auto actor = ScriptEngine::getActor(v, 2);
        if (!actor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        auto pos = (sf::Vector2i)actor->getPosition();
        auto camera = g_pEngine->getCameraAt();
        sf::IntRect rect(camera.x, camera.y, Screen::Width, Screen::Height);
        auto isOnScreen = rect.contains(pos);
        sq_pushbool(v, isOnScreen ? SQTrue : SQFalse);
        return 1;
    }

    static SQInteger masterActorArray(HSQUIRRELVM v)
    {
        auto &actors = g_pEngine->getActors();
        sq_newarray(v, 0);
        for (auto &actor : actors)
        {
            sq_pushobject(v, actor->getTable());
            sq_arrayappend(v, -2);
        }
        return 1;
    }

    static std::string str_toupper(std::string s)
    {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) { return std::toupper(c); } // correct
        );
        return s;
    }

    static SQInteger _sayLine(HSQUIRRELVM v)
    {
        auto type = sq_gettype(v, 2);
        auto actor = (type == OT_STRING) ? g_pEngine->getCurrentActor() : ScriptEngine::getActor(v, 2);
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

    static SQInteger mumbleLine(HSQUIRRELVM v)
    {
        return _sayLine(v);
    }

    static SQInteger sayLine(HSQUIRRELVM v)
    {
        auto &actors = g_pEngine->getActors();
        for (auto &a : actors)
        {
            if (a.get() == g_pEngine->getCurrentActor())
                continue;
            a->stopTalking();
        }

        return _sayLine(v);
    }

    static SQInteger selectActor(HSQUIRRELVM v)
    {
        auto *actor = ScriptEngine::getActor(v, 2);
        if (!actor)
        {
            return sq_throwerror(v, _SC("failed to get actor"));
        }
        g_pEngine->setCurrentActor(actor);

        sq_pushroottable(v);
        sq_pushstring(v, _SC("onActorSelected"), -1);
        if (SQ_FAILED(sq_get(v, -2)))
        {
            return sq_throwerror(v, _SC("failed to call onActorSelected function"));
        }

        sq_remove(v, -2);
        sq_pushroottable(v);
        sq_pushobject(v, actor->getTable());
        sq_pushbool(v, false);
        sq_call(v, 3, SQFalse, SQTrue);
        sq_pop(v, 2); //pops the roottable and the function

        return 0;
    }

    static SQInteger stopTalking(HSQUIRRELVM v)
    {
        Actor *actor = nullptr;
        auto numArgs = sq_gettop(v) - 1;
        if (numArgs == 1)
        {
            actor = ScriptEngine::getActor(v, 2);
            if (!actor)
            {
                return sq_throwerror(v, _SC("failed to get actor"));
            }
        }
        actor = g_pEngine->getCurrentActor();
        actor->stopTalking();
        return 0;
    }

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
                sq_pushobject(v, actor->getTable());
                sq_arrayappend(v, -2);
            }
        }
        return 1;
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
        colors.sentence = _fromRgb(sentence);
        colors.verbNormal = _fromRgb(verbNormal);
        colors.verbNormalTint = _fromRgb(verbNormalTint);
        colors.verbHighlight = _fromRgb(verbHighlight);
        colors.verbHighlightTint = _fromRgb(verbHighlightTint);
        colors.dialogNormal = _fromRgb(dialogNormal);
        colors.dialogHighlight = _fromRgb(dialogHighlight);
        colors.inventoryFrame = _fromRgb(inventoryFrame);
        colors.inventoryBackground = _fromRgb(inventoryBackground);
        g_pEngine->setVerbUiColors(actorSlot - 1, colors);
        return 0;
    }
};

Engine *_ActorPack::g_pEngine = nullptr;

} // namespace ng
