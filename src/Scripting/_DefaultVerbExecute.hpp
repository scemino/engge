#pragma once

#include "../System/_Util.hpp"
#include "engge/Engine/Engine.hpp"
#include "engge/System/Logger.hpp"
#include "engge/Engine/Sentence.hpp"
#include "engge/Scripting/ScriptEngine.hpp"
#include <squirrel.h>

namespace ng {

class _ActorWalk : public Function {
public:
  _ActorWalk(Engine &engine, Actor &actor, Entity *pEntity, Sentence *pSentence, const Verb *pVerb)
      : _engine(engine), _actor(actor), _pEntity(pEntity), _pSentence(pSentence), _pVerb(pVerb) {
    auto pActor = dynamic_cast<Actor *>(pEntity);
    auto pos = pEntity->getPosition();
    auto usePos = pEntity->getUsePosition().value_or(glm::vec2());
    auto facing = getFacing(pEntity);
    auto destination = pActor ? pos : pos + usePos;
    _path = _actor.walkTo(destination, facing);
    int useDist = 0;
    ScriptEngine::rawGet(pEntity, "useDist", useDist);
    useDist = std::max(useDist, 4);
    _isDestination = distance(_path.back(), destination) <= static_cast<float>(useDist);
  }

private:
  static Facing getFacing(const Entity *pEntity) {
    const auto pActor = dynamic_cast<const Actor *>(pEntity);
    if (pActor) {
      return getOppositeFacing(pActor->getCostume().getFacing());
    }
    const auto pObj = dynamic_cast<const Object *>(pEntity);
    return _toFacing(pObj->getUseDirection());
  }

private:
  bool isElapsed() override {
    if (_done)
      return true;
    auto isWalking = _actor.isWalking();
    if (isWalking)
      return false;

    _done = true;
    auto pos = _actor.getPosition();
    if (_isDestination)
      return true;

    // to talk to someone, there's no need to reach him
    if (_pVerb->id == VerbConstants::VERB_TALKTO)
      return true;

    _pSentence->stop();
    if (_path.back() != pos)
      return true;

    if (ScriptEngine::rawExists(_pEntity, "verbCantReach")) {
      ScriptEngine::objCall(_pEntity, "verbCantReach");
      return true;
    }
    callDefaultObjectVerb();
    return true;
  }

  void callDefaultObjectVerb() {
    auto &obj = _engine.getDefaultObject();
    ScriptEngine::objCall(obj, "verbCantReach", _pEntity, nullptr);
  }

private:
  Engine &_engine;
  Actor &_actor;
  Entity *_pEntity;
  Sentence *_pSentence;
  const Verb *_pVerb;
  std::vector<glm::vec2> _path;
  bool _isDestination;
  bool _done{false};
};

class _PostWalk : public Function {
public:
  _PostWalk(Sentence &sentence, Entity *pObject1, Entity *pObject2, int verb)
      : _sentence(sentence), _pObject1(pObject1), _pObject2(pObject2), _verb(verb) {
  }

  bool isElapsed() override { return _done; }

  void operator()(const ngf::TimeSpan &) override {
    auto *pObj = dynamic_cast<Object *>(_pObject1);
    auto functionName = pObj ? "objectPostWalk" : "actorPostWalk";
    bool handled = false;
    if (ScriptEngine::rawExists(_pObject1, functionName)) {
      ScriptEngine::callFunc(handled, _pObject1, functionName, _verb, _pObject1, _pObject2);
    }
    if (handled) {
      _sentence.stop();
    }
    _done = true;
  }

private:
  Sentence &_sentence;
  Entity *_pObject1{nullptr};
  Entity *_pObject2{nullptr};
  int _verb{0};
  bool _done{false};
};

class _SetDefaultVerb : public Function {
public:
  explicit _SetDefaultVerb(Engine &engine) : _engine(engine) {}

  bool isElapsed() override { return _done; }

  void operator()(const ngf::TimeSpan &) override {
    if (_done)
      return;

    _done = true;
    _engine.setDefaultVerb();
  }

private:
  Engine &_engine;
  bool _done{false};
};

class _ReachAnim : public Function {
public:
  _ReachAnim(Actor &actor, Entity *obj)
      : _actor(actor), _pObject(obj) {
    auto flags = _pObject->getFlags();
    if (flags & ObjectFlagConstants::REACH_HIGH) {
      _reaching = Reaching::High;
    } else if (flags & ObjectFlagConstants::REACH_MED) {
      _reaching = Reaching::Medium;
    } else if (flags & ObjectFlagConstants::REACH_LOW) {
      _reaching = Reaching::Low;
    } else {
      _state = 2;
    }
  }

private:
  bool isElapsed() override { return _state == 3; }

  void playReachAnim() {
    _actor.getCostume().setReachState(_reaching);
    _actor.getCostume().getAnimControl().play(false);
  }

  void operator()(const ngf::TimeSpan &elapsed) override {
    switch (_state) {
    case 0:playReachAnim();
      _state = 1;
      break;
    case 1:_elapsed += elapsed;
      if (_elapsed.getTotalSeconds() > 0.330)
        _state = 2;
      break;
    case 2:_actor.getCostume().setStandState();
      _state = 3;
      break;
    }
  }

private:
  int32_t _state{0};
  Actor &_actor;
  Entity *_pObject;
  Reaching _reaching;
  ngf::TimeSpan _elapsed;
};

class _VerbExecute : public Function {
public:
  _VerbExecute(Engine &engine, Actor &actor, Entity *pObject1, Entity *pObject2, const Verb *pVerb)
      : _engine(engine), _pVerb(pVerb), _pObject1(pObject1), _pObject2(pObject2), _actor(actor) {
  }

private:
  bool isElapsed() override { return _done; }

  void operator()(const ngf::TimeSpan &) override {
    _done = true;
    auto executeVerb = true;
    if (_pVerb->id == VerbConstants::VERB_GIVE && _pObject2) {
      auto *pActor2 = dynamic_cast<Actor *>(_pObject2);
      if (!pActor2)
        pActor2 = Entity::getActor(_pObject2);
      bool selectable = true;
      ScriptEngine::rawGet(pActor2, "selectable", selectable);
      executeVerb = !selectable;
    }

    auto handled = false;
    if (executeVerb) {
      if (!ScriptEngine::rawExists(&_actor, _pVerb->func.data())) {
        handled = false;
      } else if (_pVerb->id == VerbConstants::VERB_GIVE) {
        handled = ScriptEngine::objCall(&_actor, _pVerb->func.data(), _pObject1);
      } else if (_pVerb->id == VerbConstants::VERB_TALKTO) {
        ScriptEngine::objCall(_pObject1, _pVerb->func.data());
        handled = true;
      }

      if (!handled) {
        auto count = ScriptEngine::getParameterCount(_pObject1, _pVerb->func.data());
        if (!ScriptEngine::rawExists(_pObject1, _pVerb->func.data())) {
          handled = false;
        } else if (count == 2) {
          handled = ScriptEngine::objCall(_pObject1, _pVerb->func.data(), _pObject2);
        } else {
          handled = ScriptEngine::objCall(_pObject1, _pVerb->func.data());
        }
      }
    }

    if (handled) {
      onPickup();
      return;
    }

    if (_pVerb->id == VerbConstants::VERB_GIVE) {
      auto *pActor2 = dynamic_cast<Actor *>(_pObject2);
      if (!pActor2)
        pActor2 = Entity::getActor(_pObject2);
      ScriptEngine::call("objectGive", _pObject1, &_actor, pActor2);

      auto *pObject = dynamic_cast<Object *>(_pObject1);
      _actor.giveTo(pObject, pActor2);
      return;
    }

    if (callVerbDefault(_pObject1))
      return;

    callDefaultObjectVerb();
  }

  void onPickup() {
    if (_pVerb->id != VerbConstants::VERB_PICKUP)
      return;

    ScriptEngine::call("onPickup", &_actor, _pObject1);
  }

  void callDefaultObjectVerb() {
    auto &obj = _engine.getDefaultObject();
    ScriptEngine::objCall(obj, _pVerb->func.data(), _pObject1, _pObject2);
  }

  static bool callVerbDefault(Entity *pEntity) {
    return ScriptEngine::objCall(pEntity, "verbDefault");
  }

private:
  Engine &_engine;
  const Verb *_pVerb{nullptr};
  Entity *_pObject1{nullptr};
  Entity *_pObject2{nullptr};
  Actor &_actor;
  bool _done{false};
};

class _DefaultVerbExecute : public VerbExecute {
public:
  explicit _DefaultVerbExecute(Engine &engine) : _engine(engine) {}

private:
  void execute(const Verb *pVerb, Entity *pObject1, Entity *pObject2) override {
    getVerb(pObject1, pVerb);
    if (!pVerb)
      return;

    // TODO: do it earlier
    if (pVerb->id == VerbConstants::VERB_TALKTO) {
      pObject2 = _engine.getCurrentActor();
    }

    if (pVerb->id == VerbConstants::VERB_USE && !pObject2 && useFlags(pObject1))
      return;

    if (pVerb->id == VerbConstants::VERB_GIVE && !pObject2) {
      _engine.setUseFlag(UseFlag::GiveTo, pObject1);
      return;
    }

    if (callObjectOrActorPreWalk(pVerb->id, pObject1, pObject2))
      return;

    auto pActor = _engine.getCurrentActor();
    if (!pActor)
      return;

    auto sentence = std::make_unique<Sentence>();
    auto pObjToWalkTo =
        (pVerb->id == VerbConstants::VERB_USE || pVerb->id == VerbConstants::VERB_GIVE) ? pObject2 : pObject1;
    if (needsToWalkTo(pVerb->id, pObjToWalkTo)) {
      auto walk = std::make_unique<_ActorWalk>(_engine, *pActor, pObjToWalkTo, sentence.get(), pVerb);
      sentence->push_back(std::move(walk));
      auto postWalk = std::make_unique<_PostWalk>(*sentence, pObject1, pObject2, pVerb->id);
      sentence->push_back(std::move(postWalk));
    }

    if (needsReachAnim(pVerb->id)) {
      if (ScriptEngine::exists(pObject1, pVerb->func.data())) {
        auto reach = std::make_unique<_ReachAnim>(*pActor, pObject1);
        sentence->push_back(std::move(reach));
      }
    }

    auto verb = std::make_unique<_VerbExecute>(_engine, *pActor, pObject1, pObject2, pVerb);
    sentence->push_back(std::move(verb));
    auto setDefaultVerb = std::make_unique<_SetDefaultVerb>(_engine);
    sentence->push_back(std::move(setDefaultVerb));
    _engine.setSentence(std::move(sentence));
  }

  static bool needsToWalkTo(int verbId, Entity *pObj) {
    if (verbId == VerbConstants::VERB_LOOKAT && isFarLook(pObj))
      return false;
    return pObj && !pObj->isInventoryObject();
  }

  static bool needsReachAnim(int verbId) {
    return verbId == VerbConstants::VERB_PICKUP || verbId == VerbConstants::VERB_OPEN
        || verbId == VerbConstants::VERB_CLOSE || verbId == VerbConstants::VERB_PUSH
        || verbId == VerbConstants::VERB_PULL
        || verbId == VerbConstants::VERB_USE;
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

  bool callObjectOrActorPreWalk(int verb, Entity *pObj1, Entity *pObj2) {
    auto handled = false;
    auto pActor = _engine.getCurrentActor();
    if (ScriptEngine::rawExists(pActor, "actorPreWalk")) {
      ScriptEngine::callFunc(handled, pActor, "actorPreWalk", verb, pObj1, pObj2);
      if (handled)
        return true;
    }

    auto *pObj = dynamic_cast<Object *>(pObj1);
    auto functionName = pObj ? "objectPreWalk" : "actorPreWalk";
    if (ScriptEngine::rawExists(pObj1, functionName)) {
      ScriptEngine::callFunc(handled, pObj1, functionName, verb, pObj1, pObj2);
    }
    return handled;
  }

  void getVerb(Entity *pObj, const Verb *&pVerb) {
    int verb;
    if (pVerb) {
      verb = pVerb->id;
      pVerb = _engine.getHud().getVerb(verb);
      return;
    }
    auto defaultVerb = pObj->getDefaultVerb(VerbConstants::VERB_LOOKAT);
    if (!defaultVerb)
      return;
    verb = defaultVerb;
    pVerb = _engine.getHud().getVerb(verb);
  }

private:
  Engine &_engine;
};
} // namespace ng
