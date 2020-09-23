#pragma once

#include "../System/_Util.hpp"
#include "Engine/Engine.hpp"
#include "Engine/Sentence.hpp"
#include "Scripting/ScriptEngine.hpp"
#include "System/Logger.hpp"
#include "squirrel.h"

namespace ng::priv {

class DefaultVerbExecute final : public VerbExecute {
public:
  explicit DefaultVerbExecute(Engine &engine) : _engine(engine) {}

private:
  void execute(Actor *pActor, const Verb *pVerb, Entity *pObject1, Entity *pObject2) override {
    if (!pActor)
      pActor = _engine.getCurrentActor();

    if (!pVerb || !pObject1)
      return;

    if (!pObject1->isInventoryObject() && !pObject1->isTouchable())
      return; // Object became untouchable
    if (pObject2 && !pObject2->isInventoryObject() && !pObject2->isTouchable())
      return; // Object became untouchable

    if (pVerb->id == VerbConstants::VERB_USE && !pObject2 && useFlags(pObject1))
      return;

    if (pVerb->id == VerbConstants::VERB_GIVE && !pObject2) {
      _engine.setUseFlag(UseFlag::GiveTo, pObject1);
      return;
    }

    trace("execute {} {} {} {}",
          pActor->getKey(),
          pVerb->func,
          pObject1->getKey(),
          pObject2 ? pObject2->getKey() : "(none)");

    if (pObject1->isInventoryObject()) {
      if (!pObject2 || pObject2->isInventoryObject()) {
        callVerb(pActor, pVerb, pObject1, pObject2);
        return;
      }
    }

    if (!needsToWalkTo(pVerb->id, pObject1, pObject2)) {
      if (!pObject1->isInventoryObject()) {
        actorTurnTo(pActor, pObject1);
      }
      callVerb(pActor, pVerb, pObject1, pObject2);
      return;
    }
    if (actorPreWalkHandled(pActor, pVerb, pObject1, pObject2))
      return;
    if (objectPreWalkHandled(pVerb, pObject1, pObject2))
      return;

    pActor->setArrivedCallback([=]() {
      if (cantReach(pActor, pVerb, pObject1, pObject2))
        return;
      if (actorPostWalkHandled(pActor, pVerb, pObject1, pObject2))
        return;
      if (objectPostWalkHandled(pVerb, pObject1, pObject2))
        return;
      callVerb(pActor, pVerb, pObject1, pObject2);
    });
    if (!pObject1->isInventoryObject()) {
      actorWalkTo(pActor, pObject1);
    } else {
      actorWalkTo(pActor, pObject2);
    }
  }

  bool cantReach(Actor *pActor, const Verb *pVerb, Entity *pNoun1, Entity *pNoun2) {
    if (!pNoun1->isInventoryObject()) {
      return cantReach(pActor, pVerb, pNoun1);
    }
    if (pNoun2 && !pNoun2->isInventoryObject()) {
      return cantReach(pActor, pVerb, pNoun2);
    }
    return false;
  }

  bool cantReach(Actor *pActor, const Verb *pVerb, Entity *pNoun) {
    if (!pNoun->isTouchable()) {    // Object became untouchable as we were walking there.
      return true;
    }
    if (needsReachAnim(pVerb->id)) {
      // Did we get close enough?
      const auto dist = distance(pActor->getPosition(), getDestination(pNoun));
      int useDist = 0;
      ScriptEngine::rawGet(pNoun, "useDist", useDist);
      useDist = std::max(useDist, 4);
      if (dist > static_cast<float>(useDist)) {
        verbCantReach(pNoun);
        return true;
      }
    }
    pActor->getCostume().setFacing(getFacing(pNoun));
    return false;
  }

  void verbCantReach(Entity *pObj) const {
    if (ScriptEngine::rawExists(pObj, "verbCantReach")) {
      ScriptEngine::objCall(pObj, "verbCantReach");
      return;
    }
    auto &obj = _engine.getDefaultObject();
    ScriptEngine::objCall(obj, "verbCantReach", pObj, nullptr);
  }

  void callVerb(Actor *pActor, const Verb *pVerb, Entity *pNoun1, Entity *pNoun2) {
    _engine.setDefaultVerb();
    _engine.getCurrentActor()->stopWalking();
    if (!pNoun1->isInventoryObject() && !pNoun1->isTouchable())
      return; // Object became untouchable
    if (pNoun2 && !pNoun2->isInventoryObject() && !pNoun2->isTouchable())
      return; // Object became untouchable

    std::optional<Reaching> reach;
    if (needsReachAnim(pVerb->id)) {
      if (!pNoun1->isInventoryObject()) {
        reach = getReach(pNoun1);
      } else if (pNoun2 && !pNoun2->isInventoryObject()) {
        reach = getReach(pNoun2);
      }
    }

    playReachAnim(pActor, reach,
                  [=]() { callVerbCore(pActor, pVerb, pNoun1, pNoun2); });
  }

  static std::optional<Reaching> getReach(Entity *pObj) {
    const auto flags = pObj->getFlags();
    if (flags & ObjectFlagConstants::REACH_LOW) {
      return Reaching::Low;
    }
    if (flags & ObjectFlagConstants::REACH_HIGH) {
      return Reaching::High;
    }
    if (flags & ObjectFlagConstants::REACH_MED) {
      return Reaching::Medium;
    }
    return std::nullopt;
  }

  void callVerbCore(Actor *pActor, const Verb *pVerb, Entity *pNoun1, Entity *pNoun2) {
    if (pVerb->id == VerbConstants::VERB_GIVE) {
      auto handled = false;
      if (ScriptEngine::exists(pNoun2, pVerb->func.data())) {
        ScriptEngine::callFunc(handled, pNoun2, pVerb->func.data(), pNoun1);
        if (handled)
          return;
      } else if (ScriptEngine::exists(pNoun1, pVerb->func.data())) {
        ScriptEngine::objCall(pNoun1, pVerb->func.data(), pNoun2);
        return;
      }
      ScriptEngine::call("objectGive", pNoun1, pActor, pNoun2);
      pActor->giveTo(dynamic_cast<Object *>(pNoun1),
                     dynamic_cast<Actor *>(pNoun2));
      return;
    }

    if (ScriptEngine::exists(pNoun1, pVerb->func.data())) {
      bool handled;
      auto count = ScriptEngine::getParameterCount(pNoun1, pVerb->func.data());
      if (count == 2) {
        handled = ScriptEngine::objCall(pNoun1, pVerb->func.data(), pNoun2);
      } else {
        handled = ScriptEngine::objCall(pNoun1, pVerb->func.data());
      }
      if (handled)
        return;
    }

    if (callVerbDefault(pNoun1))
      return;

    callDefaultObjectVerb(pVerb, pActor, pNoun1, pNoun2);
  }

  void playReachAnim(Actor *pActor, std::optional<Reaching> reach, const std::function<void()> &endReachCallback) {
    if (!reach) {
      endReachCallback();
      return;
    }
    pActor->getCostume().setReachState(reach.value());
    _engine.addSystemCallback(sf::seconds(0.2), [=]() {
      pActor->getCostume().setStandState();
      endReachCallback();
    });
  }

  void callDefaultObjectVerb(const Verb *pVerb, Actor *pActor, Entity *pNoun1, Entity *pNoun2) {
    auto &obj = _engine.getDefaultObject();
    if (pVerb->id == VerbConstants::VERB_TALKTO) {
      ScriptEngine::objCall(obj, pVerb->func.data(), pNoun1, pActor);
      return;
    }
    ScriptEngine::objCall(obj, pVerb->func.data(), pNoun1, pNoun2);
  }

  static bool callVerbDefault(Entity *pEntity) {
    return ScriptEngine::objCall(pEntity, "verbDefault");
  }

  static bool actorPreWalkHandled(Actor *pActor, const Verb *pVerb, Entity *pNoun1, Entity *pNoun2) {
    auto handled = false;
    if (ScriptEngine::rawExists(pActor, "actorPreWalk")) {
      ScriptEngine::callFunc(handled, pActor, "actorPreWalk", pVerb->id, pNoun1,
                             pNoun2);
      return handled;
    }
    ScriptEngine::callFunc(handled, "actorPreWalk", pVerb->id, pNoun1, pNoun2);
    return false;
  }

  static bool objectPreWalkHandled(const Verb *pVerb, Entity *pNoun1, Entity *pNoun2) {
    auto handled = false;
    if (ScriptEngine::rawExists(pNoun1, "objectPreWalk")) {
      ScriptEngine::callFunc(handled, pNoun1, "objectPreWalk", pVerb->id,
                             pNoun1, pNoun2);
      return handled;
    }
    ScriptEngine::callFunc(handled, "objectPreWalk", pVerb->id, pNoun1, pNoun2);
    return false;
  }

  static bool actorPostWalkHandled(Actor *pActor, const Verb *pVerb, Entity *pNoun1, Entity *pNoun2) {
    auto handled = false;
    if (ScriptEngine::rawExists(pActor, "actorPostWalk")) {
      ScriptEngine::callFunc(handled, pActor, "actorPostWalk", pVerb->id,
                             pNoun1, pNoun2);
      return handled;
    }
    ScriptEngine::callFunc(handled, "actorPostWalk", pVerb->id, pNoun1, pNoun2);
    return handled;
  }

  static bool objectPostWalkHandled(const Verb *pVerb, Entity *pNoun1, Entity *pNoun2) {
    auto handled = false;
    if (ScriptEngine::rawExists(pNoun1, "objectPostWalk")) {
      ScriptEngine::callFunc(handled, pNoun1, "objectPostWalk", pVerb->id,
                             pNoun1, pNoun2);
      return handled;
    }
    ScriptEngine::callFunc(handled, "objectPostWalk", pVerb->id, pNoun1,
                           pNoun2);
    return handled;
  }

  static sf::Vector2f getDestination(Entity *pObject) {
    auto pos = pObject->getPosition();
    if (dynamic_cast<Object *>(pObject)) {
      const auto usePos = pObject->getUsePosition().value_or(sf::Vector2f());
      pos += usePos;
    }
    return pos;
  }

  static void actorWalkTo(Actor *pActor, Entity *pObject) {
    const auto pos = getDestination(pObject);
    pActor->walkTo(pos);
  }

  static void actorTurnTo(const Actor *pActor, Entity *pObject) {
    pActor->getCostume().setFacing(getFacing(pObject));
  }

  static Facing getFacing(const Entity *pEntity) {
    const auto pActor = dynamic_cast<const Actor *>(pEntity);
    if (pActor) {
      return getOppositeFacing(pActor->getCostume().getFacing());
    }
    const auto pObj = dynamic_cast<const Object *>(pEntity);
    return _toFacing(pObj->getUseDirection());
  }

  static bool needsToWalkTo(int verbId, Entity *pNoun1, Entity *pNoun2) {
    if (verbId == VerbConstants::VERB_LOOKAT && isFarLook(pNoun1))
      return false;
    return (pNoun1 && !pNoun1->isInventoryObject()) ||
        (pNoun2 && !pNoun2->isInventoryObject());
  }

  static bool needsReachAnim(int verbId) {
    return verbId == VerbConstants::VERB_PICKUP ||
        verbId == VerbConstants::VERB_OPEN ||
        verbId == VerbConstants::VERB_CLOSE ||
        verbId == VerbConstants::VERB_PUSH ||
        verbId == VerbConstants::VERB_PULL ||
        verbId == VerbConstants::VERB_USE;
  }

  static bool isFarLook(const Entity *pEntity) {
    auto flags = pEntity->getFlags();
    return flags & 8u;
  }

  bool useFlags(Entity *pObject) {
    auto flags = pObject->getFlags();
    UseFlag useFlag;
    if (flags & ObjectFlagConstants::USE_WITH) {
      useFlag = UseFlag::UseWith;
    } else if (flags & ObjectFlagConstants::USE_ON) {
      useFlag = UseFlag::UseOn;
    } else if (flags & ObjectFlagConstants::USE_IN) {
      useFlag = UseFlag::UseIn;
    } else {
      return false;
    }
    _engine.setUseFlag(useFlag, pObject);
    return true;
  }

private:
  Engine &_engine;
};
} // namespace ng
