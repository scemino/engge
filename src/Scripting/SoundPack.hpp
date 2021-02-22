#pragma once
#include <random>
#include <squirrel.h>
#include "engge/Engine/Engine.hpp"
#include "engge/Audio/SoundTrigger.hpp"

namespace ng {
class SoundPack final : public Pack {
private:
  static Engine *g_pEngine;

private:
  void registerPack() const override {
    g_pEngine = &ScriptEngine::getEngine();
    ScriptEngine::registerGlobalFunction(actorSound, "actorSound");
    ScriptEngine::registerGlobalFunction(defineSound, "defineSound");
    ScriptEngine::registerGlobalFunction(fadeOutSound, "fadeOutSound");
    ScriptEngine::registerGlobalFunction(isSoundPlaying, "isSoundPlaying");
    ScriptEngine::registerGlobalFunction(loadSound, "loadSound");
    ScriptEngine::registerGlobalFunction(loopSound, "loopSound");
    ScriptEngine::registerGlobalFunction(loopObjectSound, "loopObjectSound");
    ScriptEngine::registerGlobalFunction(loopMusic, "loopMusic");
    ScriptEngine::registerGlobalFunction(masterSoundVolume, "masterSoundVolume");
    ScriptEngine::registerGlobalFunction(playMusic, "playMusic");
    ScriptEngine::registerGlobalFunction(playSound, "playSound");
    ScriptEngine::registerGlobalFunction(playSoundVolume, "playSoundVolume");
    ScriptEngine::registerGlobalFunction(playObjectSound, "playObjectSound");
    ScriptEngine::registerGlobalFunction(soundVolume, "soundVolume");
    ScriptEngine::registerGlobalFunction(soundMixVolume, "soundMixVolume");
    ScriptEngine::registerGlobalFunction(musicMixVolume, "musicMixVolume");
    ScriptEngine::registerGlobalFunction(talkieMixVolume, "talkieMixVolume");
    ScriptEngine::registerGlobalFunction(stopAllSounds, "stopAllSounds");
    ScriptEngine::registerGlobalFunction(stopSound, "stopSound");
  }

  static bool _getArray(HSQUIRRELVM v,
                        SQInteger index,
                        SQInteger size,
                        std::vector<std::shared_ptr<SoundDefinition>> &array) {
    for (auto i = 0; i < static_cast<int>(size); i++) {
      auto pSound = EntityManager::getSoundDefinition(v, index + i);
      if (!pSound)
        return false;
      array.push_back(pSound);
    }
    return true;
  }

  static bool _getArray(HSQUIRRELVM v, SQInteger index, std::vector<std::shared_ptr<SoundDefinition>> &array) {
    HSQOBJECT paramObj;
    sq_resetobject(&paramObj);
    sq_getstackobj(v, index, &paramObj);
    if (!sq_isarray(paramObj))
      return false;

    sq_push(v, 3);
    sq_pushnull(v); //null iterator
    while (SQ_SUCCEEDED(sq_next(v, -2))) {
      auto pSound = EntityManager::getSoundDefinition(v, -1);
      if (!pSound)
        return false;
      array.emplace_back(pSound);
      sq_pop(v, 2);
    }
    sq_pop(v, 1); //pops the null iterator
    return true;
  }

  static SQInteger actorSound(HSQUIRRELVM v) {
    auto pEntity = EntityManager::getEntity(v, 2);
    if (!pEntity) {
      return sq_throwerror(v, _SC("failed to get actor or object"));
    }
    SQInteger triggerNumber;
    if (SQ_FAILED(sq_getinteger(v, 3, &triggerNumber))) {
      return sq_throwerror(v, _SC("failed to get triggerNumber"));
    }

    auto numSounds = sq_gettop(v) - 3;
    if (numSounds == 0) {
      return 0;
    }

    SQInteger tmp;
    if (numSounds == 1 && SQ_SUCCEEDED(sq_getinteger(v, 4, &tmp)) && !tmp) {
      pEntity->removeTrigger(triggerNumber);
      return 0;
    }

    std::vector<std::shared_ptr<SoundDefinition>> sounds;
    if (numSounds >= 1 || !_getArray(v, 4, sounds)) {
      if (!_getArray(v, 4, numSounds, sounds)) {
        return sq_throwerror(v, _SC("failed to get sounds"));
      }
    }

    auto *pSound = pEntity->createSoundTrigger(*g_pEngine, sounds);
    pEntity->setTrigger(triggerNumber, pSound);
    return 0;
  }

  static SQInteger loopMusic(HSQUIRRELVM v) {
    auto pSound = EntityManager::getSoundDefinition(v, 2);
    if (!pSound) {
      return sq_throwerror(v, _SC("failed to get music"));
    }
    auto music = g_pEngine->getSoundManager().playMusic(pSound, -1);
    ScriptEngine::pushObject(v, music.get());
    return 1;
  }

  static SQInteger masterSoundVolume(HSQUIRRELVM v) {
    SQFloat volume = 0;
    if (SQ_FAILED(sq_getfloat(v, 2, &volume))) {
      return sq_throwerror(v, _SC("failed to get volume"));
    }
    g_pEngine->getSoundManager().setMasterVolume(volume);
    return 0;
  }

  static SQInteger defineSound(HSQUIRRELVM v) {
    const SQChar *filename;
    if (SQ_FAILED(sq_getstring(v, 2, &filename))) {
      return sq_throwerror(v, _SC("failed to get filename"));
    }
    auto sound = g_pEngine->getSoundManager().defineSound(filename);
    ScriptEngine::pushObject(v, sound.get());
    return 1;
  }

  static SQInteger loopObjectSound(HSQUIRRELVM v) {
    auto pSound = EntityManager::getSoundDefinition(v, 2);
    if (!pSound) {
      return sq_throwerror(v, _SC("failed to get sound"));
    }
    auto pEntity = EntityManager::getEntity(v, 3);
    if (!pEntity) {
      return sq_throwerror(v, _SC("failed to get actor or object"));
    }
    SQInteger loopTimes = -1;
    sq_getinteger(v, 4, &loopTimes);
    SQFloat fadeInTime = 0;
    sq_getfloat(v, 5, &fadeInTime);
    auto pSoundId =
        g_pEngine->getSoundManager().playSound(pSound, loopTimes, ngf::TimeSpan::seconds(fadeInTime), pEntity->getId());
    if (!pSoundId) {
      sq_pushnull(v);
      return 1;
    }
    ScriptEngine::pushObject(v, pSoundId.get());
    return 1;
  }

  static SQInteger loadSound(HSQUIRRELVM v) {
    auto pSound = EntityManager::getSoundDefinition(v, 2);
    if (!pSound) {
      return sq_throwerror(v, _SC("failed to get sound"));
    }
    pSound->load();
    return 0;
  }

  static SQInteger loopSound(HSQUIRRELVM v) {
    auto pSound = EntityManager::getSoundDefinition(v, 2);
    if (!pSound) {
      return sq_throwerror(v, _SC("failed to get sound"));
    }
    SQInteger loopTimes = -1;
    sq_getinteger(v, 3, &loopTimes);
    SQFloat fadeInTime = 0;
    sq_getfloat(v, 4, &fadeInTime);
    auto pSoundId = g_pEngine->getSoundManager().playSound(pSound, loopTimes, ngf::TimeSpan::seconds(fadeInTime));
    ScriptEngine::pushObject(v, pSoundId.get());
    return 1;
  }

  static SQInteger fadeOutSound(HSQUIRRELVM v) {
    auto pSound = EntityManager::getSound(v, 2);
    float t;
    if (SQ_FAILED(sq_getfloat(v, 3, &t))) {
      return sq_throwerror(v, _SC("failed to get fadeOut time"));
    }
    auto time = ngf::TimeSpan::seconds(t);
    if (pSound) {
      pSound->stop(time);
      return 0;
    }

    auto pSoundDefinition = EntityManager::getSoundDefinition(v, 2);
    if (pSoundDefinition == nullptr) {
      error("no sound to fadeOutSound");
      return 0;
    }
    auto size = g_pEngine->getSoundManager().getSize();
    for (size_t i = 1; i <= size; i++) {
      auto pSound2 = g_pEngine->getSoundManager().getSound(i);
      if (pSound2 && pSound2->getSoundDefinition() == pSoundDefinition) {
        pSound2->stop(time);
      }
    }
    return 0;
  }

  static SQInteger isSoundPlaying(HSQUIRRELVM v) {
    auto pSound = EntityManager::getSound(v, 2);
    if (pSound) {
      sq_pushinteger(v, pSound->isPlaying() ? 1 : 0);
      return 1;
    }
    auto pSoundDef = EntityManager::getSoundDefinition(v, 2);
    if (!pSoundDef) {
      sq_pushinteger(v, 0);
      return 1;
    }

    for (size_t i = 1; i <= g_pEngine->getSoundManager().getSize(); i++) {
      auto sound = g_pEngine->getSoundManager().getSound(i);
      if (sound && pSoundDef == sound->getSoundDefinition() && sound->isPlaying()) {
        sq_pushinteger(v, 1);
        return 1;
      }
    }
    sq_pushinteger(v, 0);
    return 1;
  }

  static SQInteger playObjectSound(HSQUIRRELVM v) {
    auto pSound = EntityManager::getSoundDefinition(v, 2);
    if (!pSound) {
      return sq_throwerror(v, _SC("failed to get sound"));
    }
    auto pEntity = EntityManager::getEntity(v, 3);
    if (!pEntity) {
      return sq_throwerror(v, _SC("failed to get actor or object"));
    }
    SQInteger loopTimes = 1;
    sq_getinteger(v, 4, &loopTimes);
    SQFloat fadeInTime = 0;
    sq_getfloat(v, 5, &fadeInTime);
    auto soundId =
        g_pEngine->getSoundManager().playSound(pSound, loopTimes, ngf::TimeSpan::seconds(fadeInTime), pEntity->getId());
    ScriptEngine::pushObject(v, soundId.get());
    return 1;
  }

  static SQInteger playMusic(HSQUIRRELVM v) {
    auto pSound = EntityManager::getSoundDefinition(v, 2);
    if (!pSound) {
      return sq_throwerror(v, _SC("failed to get music"));
    }
    auto soundId = g_pEngine->getSoundManager().playMusic(pSound);
    ScriptEngine::pushObject(v, soundId.get());

    return 1;
  }

  static SQInteger playSound(HSQUIRRELVM v) {
    auto pSound = EntityManager::getSoundDefinition(v, 2);
    if (!pSound) {
      return sq_throwerror(v, _SC("failed to get sound"));
    }
    auto soundId = g_pEngine->getSoundManager().playSound(pSound);
    ScriptEngine::pushObject(v, soundId.get());

    return 1;
  }

  static SQInteger playSoundVolume(HSQUIRRELVM v) {
    auto pSound = EntityManager::getSoundDefinition(v, 2);
    if (!pSound) {
      return sq_throwerror(v, _SC("failed to get sound"));
    }
    SQFloat volume = 0;
    if (SQ_FAILED(sq_getfloat(v, 3, &volume))) {
      return sq_throwerror(v, _SC("failed to get volume"));
    }
    auto soundId = g_pEngine->getSoundManager().playSound(pSound);
    if (soundId) {
      soundId->getSoundHandle()->get().setVolume(volume);
    }
    ScriptEngine::pushObject(v, soundId.get());
    return 1;
  }

  static SQInteger soundMixVolume(HSQUIRRELVM v) {
    SQFloat volume = 0;
    if (SQ_FAILED(sq_getfloat(v, 2, &volume))) {
      return sq_throwerror(v, _SC("failed to get volume"));
    }
    g_pEngine->getSoundManager().setSoundVolume(volume);
    return 0;
  }

  static SQInteger musicMixVolume(HSQUIRRELVM v) {
    SQFloat volume = 0;
    if (SQ_FAILED(sq_getfloat(v, 2, &volume))) {
      return sq_throwerror(v, _SC("failed to get volume"));
    }
    g_pEngine->getSoundManager().setMusicVolume(volume);
    return 0;
  }

  static SQInteger talkieMixVolume(HSQUIRRELVM v) {
    SQFloat volume = 0;
    if (SQ_FAILED(sq_getfloat(v, 2, &volume))) {
      return sq_throwerror(v, _SC("failed to get volume"));
    }
    g_pEngine->getSoundManager().setTalkVolume(volume);
    return 0;
  }

  static SQInteger soundVolume(HSQUIRRELVM v) {
    SQFloat volume = 0;
    if (SQ_FAILED(sq_getfloat(v, 3, &volume))) {
      return sq_throwerror(v, _SC("failed to get volume"));
    }
    auto pSound = EntityManager::getSound(v, 2);
    if (pSound) {
      pSound->getSoundHandle()->get().setVolume(volume);
      return 0;
    }
    auto pSoundDef = EntityManager::getSoundDefinition(v, 2);
    if (pSoundDef) {
      g_pEngine->getSoundManager().setVolume(pSoundDef.get(), volume);
    }
    return 0;
  }

  static SQInteger stopAllSounds(HSQUIRRELVM) {
    g_pEngine->getSoundManager().stopAllSounds();
    return 0;
  }

  static SQInteger stopSound(HSQUIRRELVM v) {
    auto pSound = EntityManager::getSound(v, 2);
    if (pSound) {
      pSound->stop();
      return 0;
    }
    auto pSoundDef = EntityManager::getSoundDefinition(v, 2);
    if (pSoundDef) {
      g_pEngine->getSoundManager().stopSound(pSoundDef);
    }
    return 0;
  }
};

Engine *SoundPack::g_pEngine = nullptr;

} // namespace ng
