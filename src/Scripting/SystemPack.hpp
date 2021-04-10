#pragma once
#include <squirrel.h>
#include "../../extlibs/squirrel/squirrel/sqpcheader.h"
#include "../../extlibs/squirrel/squirrel/sqvm.h"
#include "../../extlibs/squirrel/squirrel/sqstring.h"
#include "../../extlibs/squirrel/squirrel/sqtable.h"
#include "../../extlibs/squirrel/squirrel/sqarray.h"
#include "../../extlibs/squirrel/squirrel/sqfuncproto.h"
#include "../../extlibs/squirrel/squirrel/sqclosure.h"
#include <engge/Entities/Actor.hpp>
#include <engge/Engine/Camera.hpp>
#include <engge/Dialog/DialogManager.hpp>
#include <engge/Engine/Engine.hpp>
#include <engge/Engine/Function.hpp>
#include <engge/System/Logger.hpp>
#include <engge/Engine/Preferences.hpp>
#include <engge/Room/Room.hpp>
#include <engge/Audio/SoundId.hpp>
#include <engge/Audio/SoundManager.hpp>
#include <engge/Engine/Thread.hpp>
#include <Engine/AchievementManager.hpp>
#include "Util/Util.hpp"

#define SQ_SUSPEND_FLAG -666

namespace ng {
class BreakFunction : public Function {
protected:
  Engine &m_engine;
  int m_threadId;
  bool m_done{false};

public:
  explicit BreakFunction(Engine &engine, int id)
      : m_engine(engine), m_threadId(id) {
  }

  [[nodiscard]] virtual std::string getName() const {
    return "_BreakFunction ";
  }

  void operator()(const ngf::TimeSpan &) override {
    if (m_done)
      return;

    if (!isElapsed())
      return;

    m_done = true;
    auto pThread = EntityManager::getThreadFromId(m_threadId);
    if (!pThread)
      return;
    pThread->resume();
  }
};

class BreakHereFunction final : public BreakFunction {
public:
  explicit BreakHereFunction(Engine &engine, int id, int numFrames)
      : BreakFunction(engine, id), m_fc(engine.getFrameCounter()), m_numFrames(numFrames) {
  }

  bool isElapsed() override {
    return m_engine.getFrameCounter() >= (m_fc + m_numFrames);
  }

  [[nodiscard]] std::string getName() const override {
    return "_BreakHereFunction";
  }

private:
  int m_fc;
  int m_numFrames;
};

class BreakWhileAnimatingFunction final : public BreakFunction {
private:
  std::string m_name;
  Actor &m_actor;
  Animation *m_pAnimation;

public:
  BreakWhileAnimatingFunction(Engine &engine, int id, Actor &actor)
      : BreakFunction(engine, id), m_actor(actor), m_pAnimation(actor.getCostume().getAnimation()) {
    m_name = m_pAnimation->name;
  }

  [[nodiscard]] std::string getName() const override {
    return "_BreakWhileAnimatingFunction " + m_name;
  }

  bool isElapsed() override {
    auto animControl = m_actor.getCostume().getAnimControl();
    return animControl.getAnimation() != m_pAnimation || animControl.getState() != AnimState::Play;
  }
};

class BreakWhileAnimatingObjectFunction final : public BreakFunction {
private:
  Object &m_object;
  std::optional<Animation *> m_animation;

public:
  BreakWhileAnimatingObjectFunction(Engine &engine, int id, Object &object)
      : BreakFunction(engine, id), m_object(object), m_animation(object.getAnimation()) {
  }

  [[nodiscard]] std::string getName() const override {
    return "_BreakWhileAnimatingObjectFunction";
  }

  bool isElapsed() override {
    return !m_animation.has_value() || m_object.getAnimControl().getState() != AnimState::Play;
  }
};

class BreakWhileWalkingFunction final : public BreakFunction {
private:
  Actor &m_actor;

public:
  explicit BreakWhileWalkingFunction(Engine &engine, int id, Actor &actor)
      : BreakFunction(engine, id), m_actor(actor) {
  }

  [[nodiscard]] std::string getName() const override {
    return "_BreakWhileWalkingFunction";
  }

  bool isElapsed() override {
    return !m_actor.isWalking();
  }
};

class BreakWhileTalkingFunction final : public BreakFunction {
private:
  Entity &m_entity;

public:
  explicit BreakWhileTalkingFunction(Engine &engine, int id, Entity &entity)
      : BreakFunction(engine, id), m_entity(entity) {
  }

  [[nodiscard]] std::string getName() const override {
    return "_BreakWhileTalkingFunction";
  }

  bool isElapsed() override {
    return !m_entity.isTalking();
  }
};

class BreakWhileAnyActorTalkingFunction final : public BreakFunction {
public:
  explicit BreakWhileAnyActorTalkingFunction(Engine &engine, int id)
      : BreakFunction(engine, id) {
  }

  [[nodiscard]] std::string getName() const override {
    return "_BreakWhileAnyActorTalkingFunction";
  }

  bool isElapsed() override {
    for (auto &&actor : m_engine.getActors()) {
      if (actor->isTalking())
        return false;
    }
    return true;
  }
};

class BreakWhileSoundFunction final : public BreakFunction {
private:
  int m_soundId;

public:
  BreakWhileSoundFunction(Engine &engine, int id, int soundId)
      : BreakFunction(engine, id), m_soundId(soundId) {
  }

  [[nodiscard]] std::string getName() const override {
    return "_BreakWhileSoundFunction";
  }

  bool isElapsed() override {
    auto pSoundId = dynamic_cast<SoundId *>(EntityManager::getSoundFromId(m_soundId));
    return !pSoundId || !pSoundId->isPlaying();
  }
};

class BreakWhileRunningFunction final : public Function {
private:
  int m_currentThreadId, m_threadId;
  bool m_done;

public:
  BreakWhileRunningFunction(int currentThreadId, int threadId)
      : m_currentThreadId(currentThreadId), m_threadId(threadId), m_done(false) {
  }

  void operator()(const ngf::TimeSpan &) override {
    if (m_done)
      return;

    auto pThread = EntityManager::getThreadFromId(m_threadId);
    if (!pThread || pThread->isStopped()) {
      auto pCurrentThread = EntityManager::getThreadFromId(m_currentThreadId);
      if (pCurrentThread) {
        pCurrentThread->resume();
      }
      m_done = true;
    }
  }

  bool isElapsed() override {
    return m_done;
  }
};

class BreakWhileDialogFunction final : public BreakFunction {
public:
  BreakWhileDialogFunction(Engine &engine, int id)
      : BreakFunction(engine, id) {
  }

  [[nodiscard]] std::string getName() const override {
    return "_BreakWhileDialogFunction";
  }

  bool isElapsed() override {
    return m_engine.getDialogManager().getState() == DialogManagerState::None;
  }
};

class BreakWhileCutsceneFunction final : public BreakFunction {
public:
  BreakWhileCutsceneFunction(Engine &engine, int id)
      : BreakFunction(engine, id) {
  }

  [[nodiscard]] std::string getName() const override {
    return "_BreakWhileCutsceneFunction";
  }

  bool isElapsed() override {
    return !m_engine.inCutscene();
  }
};

class BreakWhileCameraFunction final : public BreakFunction {
public:
  BreakWhileCameraFunction(Engine &engine, int id)
      : BreakFunction(engine, id) {
  }

  [[nodiscard]] std::string getName() const override {
    return "_BreakWhileCameraFunction";
  }

  bool isElapsed() override {
    return !m_engine.getCamera().isMoving();
  }
};

class BreakWhileInputOffFunction final : public BreakFunction {
public:
  BreakWhileInputOffFunction(Engine &engine, int id)
      : BreakFunction(engine, id) {
  }

  [[nodiscard]] std::string getName() const override {
    return "_BreakWhileInputOffFunction";
  }

  bool isElapsed() override {
    return m_engine.getInputActive();
  }
};

class BreakTimeFunction final : public TimeFunction {
private:
  int m_threadId;

public:
  BreakTimeFunction(int id, const ngf::TimeSpan &time)
      : TimeFunction(time), m_threadId(id) {
  }

  void operator()(const ngf::TimeSpan &elapsed) override {
    if (m_done)
      return;
    TimeFunction::operator()(elapsed);
    if (isElapsed()) {
      m_done = true;
      auto pThread = EntityManager::getThreadFromId(m_threadId);
      if (!pThread)
        return;
      pThread->resume();
    }
  }
};

class SystemPack final : public Pack {
private:
  static Engine *g_pEngine;

private:
  void registerPack() const override {
    g_pEngine = &ScriptEngine::getEngine();
    ScriptEngine::registerGlobalFunction(activeController, "activeController");
    ScriptEngine::registerGlobalFunction(addCallback, "addCallback");
    ScriptEngine::registerGlobalFunction(addFolder, "addFolder");
    ScriptEngine::registerGlobalFunction(breakhere, "breakhere");
    ScriptEngine::registerGlobalFunction(breaktime, "breaktime");
    ScriptEngine::registerGlobalFunction(breakwhileanimating, "breakwhileanimating");
    ScriptEngine::registerGlobalFunction(breakwhilecamera, "breakwhilecamera");
    ScriptEngine::registerGlobalFunction(breakwhilecutscene, "breakwhilecutscene");
    ScriptEngine::registerGlobalFunction(breakwhiledialog, "breakwhiledialog");
    ScriptEngine::registerGlobalFunction(breakwhileinputoff, "breakwhileinputoff");
    ScriptEngine::registerGlobalFunction(breakwhilesound, "breakwhilesound");
    ScriptEngine::registerGlobalFunction(breakwhilerunning, "breakwhilerunning");
    ScriptEngine::registerGlobalFunction(breakwhiletalking, "breakwhiletalking");
    ScriptEngine::registerGlobalFunction(breakwhilewalking, "breakwhilewalking");
    ScriptEngine::registerGlobalFunction(chr, "chr");
    ScriptEngine::registerGlobalFunction(cursorPosX, "cursorPosX");
    ScriptEngine::registerGlobalFunction(cursorPosY, "cursorPosY");
    ScriptEngine::registerGlobalFunction(dumpvar, "dumpvar");
    ScriptEngine::registerGlobalFunction(dumprt, "dumprt");
    ScriptEngine::registerGlobalFunction(exCommand, "exCommand");
    ScriptEngine::registerGlobalFunction(gameTime, "gameTime");
    ScriptEngine::registerGlobalFunction(getPrivatePref, "getPrivatePref");
    ScriptEngine::registerGlobalFunction(getUserPref, "getUserPref");
    ScriptEngine::registerGlobalFunction(include, "include");
    ScriptEngine::registerGlobalFunction(inputHUD, "inputHUD");
    ScriptEngine::registerGlobalFunction(inputOff, "inputOff");
    ScriptEngine::registerGlobalFunction(inputOn, "inputOn");
    ScriptEngine::registerGlobalFunction(inputSilentOff, "inputSilentOff");
    ScriptEngine::registerGlobalFunction(inputState, "inputState");
    ScriptEngine::registerGlobalFunction(isInputOn, "isInputOn");
    ScriptEngine::registerGlobalFunction(is_string, "is_string");
    ScriptEngine::registerGlobalFunction(is_table, "is_table");
    ScriptEngine::registerGlobalFunction(ord, "ord");
    ScriptEngine::registerGlobalFunction(inputController, "inputController");
    ScriptEngine::registerGlobalFunction(inputVerbs, "inputVerbs");
    ScriptEngine::registerGlobalFunction(logEvent, "logEvent");
    ScriptEngine::registerGlobalFunction(logInfo, "logInfo");
    ScriptEngine::registerGlobalFunction(logWarning, "logWarning");
    ScriptEngine::registerGlobalFunction(microTime, "microTime");
    ScriptEngine::registerGlobalFunction(moveCursorTo, "moveCursorTo");
    ScriptEngine::registerGlobalFunction(pushSentence, "pushSentence");
    ScriptEngine::registerGlobalFunction(removeCallback, "removeCallback");
    ScriptEngine::registerGlobalFunction(setAmbientLight, "setAmbientLight");
    ScriptEngine::registerGlobalFunction(setPrivatePref, "setPrivatePref");
    ScriptEngine::registerGlobalFunction(setUserPref, "setUserPref");
    ScriptEngine::registerGlobalFunction(startglobalthread, "startglobalthread");
    ScriptEngine::registerGlobalFunction(startthread, "startthread");
    ScriptEngine::registerGlobalFunction(stopthread, "stopthread");
    ScriptEngine::registerGlobalFunction(threadid, "threadid");
    ScriptEngine::registerGlobalFunction(threadpauseable, "threadpauseable");
  }

  static SQInteger activeController(HSQUIRRELVM v) {
    error("TODO: activeController: not implemented");
    // harcode mouse
    sq_pushinteger(v, 1);
    return 1;
  }

  static SQInteger addCallback(HSQUIRRELVM v) {
    auto count = sq_gettop(v);
    SQFloat duration;
    if (SQ_FAILED(sq_getfloat(v, 2, &duration))) {
      return sq_throwerror(v, _SC("failed to get duration"));
    }
    HSQOBJECT method;
    sq_resetobject(&method);
    if (SQ_FAILED(sq_getstackobj(v, 3, &method)) || !sq_isclosure(method)) {
      return sq_throwerror(v, _SC("failed to get method"));
    }

    std::string methodName;
    if (SQ_SUCCEEDED(sq_getclosurename(v, 3))) {
      const SQChar *tmpMethodName = nullptr;
      sq_getstring(v, -1, &tmpMethodName);
      methodName = tmpMethodName;
    }

    HSQOBJECT arg;
    sq_resetobject(&arg);
    if (count == 4 && SQ_FAILED(sq_getstackobj(v, 4, &arg))) {
      return sq_throwerror(v, _SC("failed to get argument"));
    }

    auto id = Locator<EntityManager>::get().getCallbackId();
    auto callback = std::make_unique<Callback>(id, ngf::TimeSpan::seconds(duration), methodName, arg);
    g_pEngine->addCallback(std::move(callback));

    sq_pushinteger(v, static_cast<SQInteger>(id));
    return 1;
  }

  static SQInteger addFolder(HSQUIRRELVM) {
    // do nothing
    return 0;
  }

  static SQInteger breakhere(HSQUIRRELVM v) {
    SQFloat numFrames;
    if (SQ_FAILED(sq_getfloat(v, 2, &numFrames))) {
      return sq_throwerror(v, _SC("failed to get numFrames"));
    }
    auto pThread = EntityManager::getThreadFromVm(v);
    pThread->suspend();

    g_pEngine->addFunction(std::make_unique<BreakHereFunction>(*g_pEngine, pThread->getId(), numFrames));
    return SQ_SUSPEND_FLAG;
  }

  static SQInteger breakwhileanimating(HSQUIRRELVM v) {
    auto *pActor = EntityManager::getActor(v, 2);
    if (pActor) {
      auto pAnim = pActor->getCostume().getAnimation();
      if (!pAnim)
        return 0;

      auto pThread = EntityManager::getThreadFromVm(v);
      pThread->suspend();

      g_pEngine->addFunction(std::make_unique<BreakWhileAnimatingFunction>(*g_pEngine, pThread->getId(), *pActor));
      return SQ_SUSPEND_FLAG;
    }

    auto *pObj = EntityManager::getObject(v, 2);
    if (pObj) {
      auto pThread = EntityManager::getThreadFromVm(v);
      pThread->suspend();

      g_pEngine->addFunction(std::make_unique<BreakWhileAnimatingObjectFunction>(*g_pEngine, pThread->getId(), *pObj));
      return SQ_SUSPEND_FLAG;
    }
    return sq_throwerror(v, _SC("failed to get actor or object"));
  }

  static SQInteger breakwhilecamera(HSQUIRRELVM v) {
    auto pThread = EntityManager::getThreadFromVm(v);
    pThread->suspend();

    g_pEngine->addFunction(std::make_unique<BreakWhileCameraFunction>(*g_pEngine, pThread->getId()));
    return SQ_SUSPEND_FLAG;
  }

  static SQInteger breakwhilecutscene(HSQUIRRELVM v) {
    auto pThread = EntityManager::getThreadFromVm(v);
    pThread->suspend();

    g_pEngine->addFunction(std::make_unique<BreakWhileCutsceneFunction>(*g_pEngine, pThread->getId()));
    return SQ_SUSPEND_FLAG;
  }

  static SQInteger breakwhileinputoff(HSQUIRRELVM v) {
    auto pThread = EntityManager::getThreadFromVm(v);
    pThread->suspend();

    g_pEngine->addFunction(std::make_unique<BreakWhileInputOffFunction>(*g_pEngine, pThread->getId()));
    return SQ_SUSPEND_FLAG;
  }

  static SQInteger breakwhilesound(HSQUIRRELVM v) {
    SoundId *pSound = EntityManager::getSound(v, 2);
    auto pThread = EntityManager::getThreadFromVm(v);
    pThread->suspend();

    g_pEngine->addFunction(std::make_unique<BreakWhileSoundFunction>(*g_pEngine,
                                                                     pThread->getId(),
                                                                      pSound ? pSound->getId() : 0));
    return SQ_SUSPEND_FLAG;
  }

  static SQInteger breakwhiledialog(HSQUIRRELVM v) {
    auto pThread = EntityManager::getThreadFromVm(v);
    pThread->suspend();

    g_pEngine->addFunction(std::make_unique<BreakWhileDialogFunction>(*g_pEngine, pThread->getId()));
    return SQ_SUSPEND_FLAG;
  }

  static SQInteger breakwhilewalking(HSQUIRRELVM v) {
    auto *pActor = EntityManager::getActor(v, 2);
    if (!pActor) {
      return sq_throwerror(v, _SC("failed to get actor"));
    }

    auto pThread = EntityManager::getThreadFromVm(v);
    pThread->suspend();

    g_pEngine->addFunction(std::make_unique<BreakWhileWalkingFunction>(*g_pEngine, pThread->getId(), *pActor));
    return SQ_SUSPEND_FLAG;
  }

  static SQInteger breakwhiletalking(HSQUIRRELVM v) {
    if (sq_gettop(v) == 2) {
      auto *pEntity = EntityManager::getEntity(v, 2);
      if (!pEntity) {
        return sq_throwerror(v, _SC("failed to get actor/object"));
      }
      auto pThread = EntityManager::getThreadFromVm(v);
      pThread->suspend();

      g_pEngine->addFunction(std::make_unique<BreakWhileTalkingFunction>(*g_pEngine, pThread->getId(), *pEntity));
      return SQ_SUSPEND_FLAG;
    }

    auto pThread = EntityManager::getThreadFromVm(v);
    pThread->suspend();

    g_pEngine->addFunction(std::make_unique<BreakWhileAnyActorTalkingFunction>(*g_pEngine, pThread->getId()));
    return SQ_SUSPEND_FLAG;
  }

  static SQInteger breakwhilerunning(HSQUIRRELVM v) {
    SQInteger id = 0;
    if (sq_gettype(v, 2) == OT_INTEGER) {
      sq_getinteger(v, 2, &id);
    }

    if (EntityManager::isThread(id)) {
      auto pCurrentThread = EntityManager::getThreadFromVm(v);
      if (!pCurrentThread) {
        return sq_throwerror(v, "Current thread should be created with startthread");
      }

      auto pThread = EntityManager::getThreadFromId(id);
      if (!pThread)
        return 0;

      pCurrentThread->suspend();
      g_pEngine->addFunction(std::make_unique<BreakWhileRunningFunction>(pCurrentThread->getId(), id));
      return SQ_SUSPEND_FLAG;
    }
    return breakwhilesound(v);
  }

  static SQInteger chr(HSQUIRRELVM v) {
    SQInteger number;
    if (SQ_FAILED(sq_getinteger(v, 2, &number))) {
      return sq_throwerror(v, "Failed to get number");
    }
    auto character = (char) number;
    char s[]{character, '\0'};
    sq_pushstring(v, s, -1);
    return 1;
  }

  static SQInteger cursorPosX(HSQUIRRELVM v) {
    auto roomSize = g_pEngine->getRoom()->getRoomSize();
    auto pos = g_pEngine->getMousePositionInRoom();
    auto x = Screen::Width * pos.x / roomSize.x;
    sq_pushinteger(v, static_cast<SQInteger>(x));
    return 1;
  }

  static SQInteger cursorPosY(HSQUIRRELVM v) {
    auto roomSize = g_pEngine->getRoom()->getRoomSize();

    auto pos = g_pEngine->getMousePositionInRoom();
    auto y = Screen::Height * pos.y / roomSize.y;
    sq_pushinteger(v, static_cast<SQInteger>(y));
    return 1;
  }

  static SQInteger dumpvar(HSQUIRRELVM v) {
    HSQOBJECT obj;
    if (sq_gettype(v, 2) == OT_BOOL) {
      SQBool value;
      sq_getbool(v, 2, &value);
      ScriptEngine::printfunc(v, "%s", value ? "true" : "false");
      return 0;
    }
    if (sq_gettype(v, 2) == OT_INTEGER) {
      SQInteger value;
      sq_getinteger(v, 2, &value);
      ScriptEngine::printfunc(v, "%ld", value);
      return 0;
    }
    if (sq_gettype(v, 2) == OT_FLOAT) {
      SQFloat value;
      sq_getfloat(v, 2, &value);
      ScriptEngine::printfunc(v, "%lf", value);
      return 0;
    }
    if (sq_gettype(v, 2) == OT_STRING) {
      const SQChar *value;
      sq_getstring(v, 2, &value);
      ScriptEngine::printfunc(v, "%s", value);
      return 0;
    }
    if (sq_gettype(v, 2) != OT_TABLE) {
      return sq_throwerror(v, _SC("A bool, int, float or table was expected"));
    }
    if (SQ_FAILED(sq_getstackobj(v, 2, &obj))) {
      return sq_throwerror(v, _SC("Failed to get object"));
    }
    auto value = ng::toGGPackValue(obj);
    std::ostringstream os;
    os << value;
    ScriptEngine::printfunc(v, "%s", os.str().data());
    return 0;
  }

  static SQInteger dumprt(HSQUIRRELVM v) {
    HSQOBJECT rt;
    sq_resetobject(&rt);
    sq_pushroottable(v);
    if (SQ_FAILED(sq_getstackobj(v, -1, &rt))) {
      return sq_throwerror(v, _SC("Failed to get root table from stack"));
    }
    auto value = ng::toGGPackValue(rt);
    std::ostringstream os;
    os << value;
    ScriptEngine::printfunc(v, "%s", os.str().data());
    return 0;
  }

  static SQInteger exCommand(HSQUIRRELVM v) {
    SQInteger command;
    if (SQ_FAILED(sq_getinteger(v, 2, &command))) {
      return sq_throwerror(v, _SC("Failed to get command"));
    }
    switch (command) {
    case ExCommandConstants::EX_ALLOW_SAVEGAMES: {
      SQInteger enabled;
      if (SQ_FAILED(sq_getinteger(v, 3, &enabled))) {
        return sq_throwerror(v, _SC("Failed to get enabled"));
      }
      g_pEngine->allowSaveGames(enabled != 0);
      return 0;
    }
    case ExCommandConstants::EX_POP_CHARACTER_SELECTION: {
      error("TODO: exCommand EX_POP_CHARACTER_SELECTION: not implemented");
      return 0;
    }
    case ExCommandConstants::EX_CAMERA_TRACKING: {
      error("TODO: exCommand EX_CAMERA_TRACKING: not implemented");
      return 0;
    }
    case ExCommandConstants::EX_BUTTON_HOVER_SOUND: {
      auto pSound = EntityManager::getSoundDefinition(v, 3);
      if (!pSound) {
        return sq_throwerror(v, _SC("failed to get sound for EX_BUTTON_HOVER_SOUND"));
      }
      g_pEngine->getSoundManager().setSoundHover(pSound);
      return 0;
    }
    case ExCommandConstants::EX_RESTART: {
      error("TODO: exCommand EX_RESTART: not implemented");
      return 0;
    }
    case ExCommandConstants::EX_IDLE_TIME: {
      error("TODO: exCommand EX_IDLE_TIME: not implemented");
      return 0;
    }
    case ExCommandConstants::EX_AUTOSAVE: {
      if (g_pEngine->getAutoSave()) {
        g_pEngine->saveGame(1);
      }
      return 0;
    }
    case ExCommandConstants::EX_AUTOSAVE_STATE: {
      SQInteger enabled;
      if (SQ_FAILED(sq_getinteger(v, 3, &enabled))) {
        return sq_throwerror(v, _SC("Failed to get enabled"));
      }
      g_pEngine->setAutoSave(enabled != 0);
      return 0;
    }
    case ExCommandConstants::EX_DISABLE_SAVESYSTEM: {
      error("TODO: exCommand EX_DISABLE_SAVESYSTEM: not implemented");
      return 0;
    }
    case ExCommandConstants::EX_SHOW_OPTIONS: {
      g_pEngine->showOptions(true);
      return 0;
    }
    case ExCommandConstants::EX_OPTIONS_MUSIC: {
      error("TODO: exCommand EX_OPTIONS_MUSIC: not implemented");
      return 0;
    }
    case ExCommandConstants::EX_FORCE_TALKIE_TEXT: {
      SQInteger enabled;
      if (SQ_FAILED(sq_getinteger(v, 3, &enabled))) {
        return sq_throwerror(v, _SC("Failed to get enabled"));
      }
      g_pEngine->getPreferences().setTempPreference(TempPreferenceNames::ForceTalkieText, enabled != 0);
      return 0;
    }
    default:error("TODO: exCommand {}: not implemented", command);
      break;
    }
    return 0;
  }

  static SQInteger gameTime(HSQUIRRELVM v) {
    sq_pushfloat(v, g_pEngine->getTime().getTotalSeconds());
    return 1;
  }

  static SQInteger logEvent(HSQUIRRELVM v) {
    auto numArgs = sq_gettop(v);
    const SQChar *event = nullptr;
    if (SQ_SUCCEEDED(sq_getstring(v, 2, &event))) {
      info(event);
    }
    if (numArgs == 3) {
      if (SQ_SUCCEEDED(sq_getstring(v, 3, &event))) {
        info(event);
      }
    }
    return 0;
  }

  static SQInteger logInfo(HSQUIRRELVM v) {
    const SQChar *msg = nullptr;
    if (SQ_SUCCEEDED(sq_getstring(v, 2, &msg))) {
      info(msg);
    }
    return 0;
  }

  static SQInteger logWarning(HSQUIRRELVM v) {
    const SQChar *msg = nullptr;
    if (SQ_SUCCEEDED(sq_getstring(v, 2, &msg))) {
      warn(msg);
    }
    return 0;
  }

  static SQInteger microTime(HSQUIRRELVM v) {
    sq_pushfloat(v, g_pEngine->getTime().getTotalSeconds());
    return 1;
  }

  static SQInteger moveCursorTo(HSQUIRRELVM v) {
    SQInteger x;
    if (SQ_FAILED(sq_getinteger(v, 2, &x))) {
      return sq_throwerror(v, _SC("Failed to get x"));
    }
    SQInteger y;
    if (SQ_FAILED(sq_getinteger(v, 3, &y))) {
      return sq_throwerror(v, _SC("Failed to get y"));
    }
    SQFloat t;
    if (SQ_FAILED(sq_getfloat(v, 4, &t))) {
      return sq_throwerror(v, _SC("Failed to get time"));
    }

    // WIP need to be check
    // auto pos = g_pEngine->getApplication()->getRenderTarget()->mapCoordsToPixel(glm::vec2(x, y) - g_pEngine->getCamera().getAt());
    // TODO: ngf::Mouse::setPosition(pos, g_pEngine->getApplication()->get);
    error("moveCursorTo not implemented");
    return 0;
  }

  static SQInteger pushSentence(HSQUIRRELVM v) {
    auto numArgs = sq_gettop(v);
    SQInteger id;
    if (SQ_FAILED(sq_getinteger(v, 2, &id))) {
      return sq_throwerror(v, _SC("Failed to get verb id"));
    }

    if (id == VerbConstants::VERB_DIALOG) {
      SQInteger choice;
      if (SQ_FAILED(sq_getinteger(v, 3, &choice))) {
        return sq_throwerror(v, _SC("Failed to get choice"));
      }
      g_pEngine->getDialogManager().choose(choice);
      return 0;
    }

    Entity *pObj1{nullptr};
    Entity *pObj2{nullptr};
    if (numArgs > 2) {
      pObj1 = EntityManager::getEntity(v, 3);
      if (!pObj1) {
        return sq_throwerror(v, _SC("Failed to get obj1"));
      }
    }
    if (numArgs > 3) {
      pObj2 = EntityManager::getEntity(v, 4);
      if (!pObj2) {
        return sq_throwerror(v, _SC("Failed to get obj2"));
      }
    }
    g_pEngine->pushSentence(static_cast<int>(id), pObj1, pObj2);
    return 0;
  }

  static SQInteger stopthread(HSQUIRRELVM v) {
    auto type = sq_gettype(v, 2);
    if (type == OT_NULL) {
      sq_pushinteger(v, 0);
      return 1;
    }

    SQInteger id;
    if (SQ_FAILED(sq_getinteger(v, 2, &id))) {
      trace("Failed to stopthread: got {} instead", type);
      sq_pushinteger(v, 0);
      return 1;
    }

    auto pThread = EntityManager::getThreadFromId(id);
    if (!pThread) {
      sq_pushinteger(v, 0);
      return 1;
    }

    trace("stopthread {}", id);
    pThread->stop();

    sq_pushinteger(v, 0);
    return 1;
  }

  static SQInteger startglobalthread(HSQUIRRELVM v) {
    return startthread(v, true);
  }

  static SQInteger startthread(HSQUIRRELVM v) {
    return startthread(v, false);
  }

  static SQInteger startthread(HSQUIRRELVM v, bool global) {
    SQInteger size = sq_gettop(v);

    HSQOBJECT env_obj;
    sq_resetobject(&env_obj);
    if (SQ_FAILED(sq_getstackobj(v, 1, &env_obj))) {
      return sq_throwerror(v, _SC("Couldn't get environment from stack"));
    }

    auto vm = ScriptEngine::getVm();
    // create thread and store it on the stack
    sq_newthread(vm, 1024);
    HSQOBJECT thread_obj;
    sq_resetobject(&thread_obj);
    if (SQ_FAILED(sq_getstackobj(vm, -1, &thread_obj))) {
      return sq_throwerror(v, _SC("Couldn't get coroutine thread from stack"));
    }

    std::vector<HSQOBJECT> args;
    for (auto i = 0; i < size - 2; i++) {
      HSQOBJECT arg;
      sq_resetobject(&arg);
      if (SQ_FAILED(sq_getstackobj(v, 3 + i, &arg))) {
        return sq_throwerror(v, _SC("Couldn't get coroutine args from stack"));
      }
      args.push_back(arg);
    }

    // get the closure
    HSQOBJECT closureObj;
    sq_resetobject(&closureObj);
    if (SQ_FAILED(sq_getstackobj(v, 2, &closureObj))) {
      return sq_throwerror(v, _SC("Couldn't get coroutine thread from stack"));
    }

    const SQChar *name = nullptr;
    if (SQ_SUCCEEDED(sq_getclosurename(v, 2))) {
      sq_getstring(v, -1, &name);
    }

    std::string threadName = name ? name : "anonymous";
    std::string pSource = _stringval(_closure(closureObj)->_function->_sourcename);
    auto line = _closure(closureObj)->_function->_lineinfos->_line;
    threadName += ' ' + pSource + '(' + std::to_string(line) + ')';
    auto pUniquethread = std::make_unique<Thread>(threadName, global, vm, thread_obj, env_obj, closureObj, args);
    sq_pop(vm, 1);
    auto pThread = pUniquethread.get();
    trace("start thread ({}): {}", threadName, pThread->getId());
    if (name) {
      sq_pop(v, 1); // pop name
    }
    sq_pop(v, 1); // pop closure
    g_pEngine->addThread(std::move(pUniquethread));

    // call the closure in the thread
    if (!pThread->call()) {
      return sq_throwerror(v, _SC("call failed"));
    }

    sq_pushinteger(v, pThread->getId());
    return 1;
  }

  static SQInteger breaktime(HSQUIRRELVM v) {
    SQFloat time = 0;
    if (SQ_FAILED(sq_getfloat(v, 2, &time))) {
      return sq_throwerror(v, _SC("failed to get time"));
    }

    auto pThread = EntityManager::getThreadFromVm(v);
    if (!pThread) {
      return sq_throwerror(v, _SC("failed to get thread"));
    }

    pThread->suspend();

    g_pEngine->addFunction(std::make_unique<BreakTimeFunction>(pThread->getId(), ngf::TimeSpan::seconds(time)));
    return SQ_SUSPEND_FLAG;
  }

  static SQInteger setPrivatePref(HSQUIRRELVM v) {
    const SQChar *key;
    if (SQ_FAILED(sq_getstring(v, 2, &key))) {
      return sq_throwerror(v, _SC("failed to get key"));
    }
    auto &achievements = ng::Locator<ng::AchievementManager>::get();
    auto type = sq_gettype(v, 3);
    if (type == SQObjectType::OT_STRING) {
      const SQChar *str = nullptr;
      sq_getstring(v, 3, &str);
      std::string strValue = str;
      achievements.setPrivatePreference(key, strValue);
      return 0;
    }
    if (type == SQObjectType::OT_INTEGER) {
      SQInteger integer;
      sq_getinteger(v, 3, &integer);
      achievements.setPrivatePreference(key, integer);
      return 0;
    }
    if (type == SQObjectType::OT_BOOL) {
      SQBool b;
      sq_getbool(v, 3, &b);
      achievements.setPrivatePreference(key, static_cast<int>(b));
      return 0;
    }
    if (type == SQObjectType::OT_FLOAT) {
      SQFloat fl;
      sq_getfloat(v, 3, &fl);
      achievements.setPrivatePreference(key, fl);
      return 0;
    }
    return 0;
  }

  static HSQOBJECT toSquirrel(HSQUIRRELVM v, const ngf::GGPackValue &value) {
    HSQOBJECT obj;
    switch (value.type()) {
    case ngf::detail::GGPackValueType::String:obj._type = OT_STRING;
      obj._unVal.pString = SQString::Create(v->_sharedstate, value.getString().c_str(), -1);
      return obj;
    case ngf::detail::GGPackValueType::Integer:obj._type = OT_INTEGER;
      obj._unVal.nInteger = value.getInt();
      return obj;
    case ngf::detail::GGPackValueType::Double:obj._type = OT_FLOAT;
      obj._unVal.fFloat = value.getDouble();
      return obj;
    case ngf::detail::GGPackValueType::Null:obj._type = OT_NULL;
      return obj;
    default:throw std::runtime_error("Cannot convert this type to squirrel");
    }
  }

  static SQInteger getPrivatePref(HSQUIRRELVM v) {
    const SQChar *key;
    if (SQ_FAILED(sq_getstring(v, 2, &key))) {
      return sq_throwerror(v, _SC("failed to get key"));
    }

    auto &achievements = ng::Locator<ng::AchievementManager>::get();
    auto value = achievements.getPrivatePreference(key);
    // key found in preferences ?
    if (!value.isNull()) {
      sq_pushobject(v, toSquirrel(v, value));
      return 1;
    }
    // we have a default value ?
    if (sq_gettop(v) == 3) {
      sq_push(v, 3);
      return 1;
    }

    // return null
    sq_pushnull(v);
    return 1;
  }

  static SQInteger getUserPref(HSQUIRRELVM v) {
    return _getPref(v,
                    [](auto name, auto value) { return g_pEngine->getPreferences().getUserPreference(name, value); });
  }

  static SQInteger _getPref(HSQUIRRELVM v,
                            const std::function<ngf::GGPackValue(const std::string &name,
                                                                 ngf::GGPackValue value)> &func) {
    const SQChar *key;
    if (SQ_FAILED(sq_getstring(v, 2, &key))) {
      return sq_throwerror(v, _SC("failed to get key"));
    }
    auto numArgs = sq_gettop(v) - 1;
    ngf::GGPackValue defaultValue;
    if (numArgs > 1) {
      auto type = sq_gettype(v, 3);
      if (type == SQObjectType::OT_STRING) {
        const SQChar *str = nullptr;
        sq_getstring(v, 3, &str);
        defaultValue = str;
      } else if (type == SQObjectType::OT_INTEGER) {
        SQInteger integer;
        sq_getinteger(v, 3, &integer);
        defaultValue = integer;
      } else if (type == SQObjectType::OT_BOOL) {
        SQBool b;
        sq_getbool(v, 3, &b);
        defaultValue = b ? 1 : 0;
      } else if (type == SQObjectType::OT_FLOAT) {
        SQFloat fl;
        sq_getfloat(v, 3, &fl);
        defaultValue = fl;
      }
    }

    auto value = func(key, defaultValue);
    if (value.isString()) {
      sq_pushstring(v, value.getString().data(), -1);
    } else if (value.isInteger()) {
      sq_pushinteger(v, value.getInt());
    } else if (value.isDouble()) {
      sq_pushfloat(v, value.getDouble());
    } else {
      sq_pushnull(v);
    }

    return 1;
  }

  static SQInteger removeCallback(HSQUIRRELVM v) {
    SQInteger id = 0;
    if (SQ_FAILED(sq_getinteger(v, 2, &id))) {
      return sq_throwerror(v, _SC("failed to get callback"));
    }
    g_pEngine->removeCallback(id);
    return 0;
  }

  static SQInteger setAmbientLight(HSQUIRRELVM v) {
    SQInteger c = 0;
    if (SQ_FAILED(sq_getinteger(v, 2, &c))) {
      return sq_throwerror(v, _SC("failed to get color"));
    }
    auto color = fromRgb(c);
    g_pEngine->getRoom()->setAmbientLight(color);
    return 0;
  }

  static SQInteger setUserPref(HSQUIRRELVM v) {
    _setPref(v,
             [](auto key, auto value) { return g_pEngine->getPreferences().setUserPreference(key, value); },
             [](auto key) { return g_pEngine->getPreferences().removeUserPreference(key); });
    return 0;
  }

  static SQInteger _setPref(HSQUIRRELVM v,
                            const std::function<void(const std::string &, ngf::GGPackValue)> &setPref,
                            const std::function<void(const std::string &)> &removePref) {
    const SQChar *key;
    if (SQ_FAILED(sq_getstring(v, 2, &key))) {
      return sq_throwerror(v, _SC("failed to get key"));
    }
    auto type = sq_gettype(v, 3);
    if (type == SQObjectType::OT_STRING) {
      const SQChar *str = nullptr;
      sq_getstring(v, 3, &str);
      std::string strValue = str;
      setPref(key, Preferences::toGGPackValue(strValue));
      return 0;
    }
    if (type == SQObjectType::OT_INTEGER) {
      SQInteger integer;
      sq_getinteger(v, 3, &integer);
      setPref(key, Preferences::toGGPackValue(static_cast<int>(integer)));
      return 0;
    }
    if (type == SQObjectType::OT_BOOL) {
      SQBool b;
      sq_getbool(v, 3, &b);
      setPref(key, Preferences::toGGPackValue(b != 0));
      return 0;
    }
    if (type == SQObjectType::OT_FLOAT) {
      SQFloat fl;
      sq_getfloat(v, 3, &fl);
      setPref(key, Preferences::toGGPackValue(fl));
      return 0;
    }

    removePref(key);

    return 0;
  }

  static SQInteger include(HSQUIRRELVM v) {
    const SQChar *filename = nullptr;
    if (SQ_FAILED(sq_getstring(v, 2, &filename))) {
      return sq_throwerror(v, "failed to get filename");
    }
    trace("include {}", filename);
    ng::ScriptEngine::executeNutScript(filename);
    return 0;
  }

  static SQInteger inputHUD(HSQUIRRELVM v) {
    SQInteger on;
    if (SQ_FAILED(sq_getinteger(v, 2, &on))) {
      return sq_throwerror(v, _SC("failed to get on"));
    }
    g_pEngine->setInputHUD(on);
    return 0;
  }

  static SQInteger inputOff(HSQUIRRELVM) {
    g_pEngine->setInputActive(false);
    return 0;
  }

  static SQInteger inputOn(HSQUIRRELVM) {
    g_pEngine->setInputActive(true);
    return 0;
  }

  static SQInteger inputSilentOff(HSQUIRRELVM) {
    g_pEngine->inputSilentOff();
    return 0;
  }

  static SQInteger isInputOn(HSQUIRRELVM v) {
    bool isActive = g_pEngine->getInputActive();
    sq_pushinteger(v, isActive ? 1 : 0);
    return 1;
  }

  static SQInteger inputState(HSQUIRRELVM v) {
    auto numArgs = sq_gettop(v);
    if (numArgs == 1) {
      auto state = g_pEngine->getInputState();
      sq_pushinteger(v, state);
      return 1;
    } else if (numArgs == 2) {
      SQInteger state;
      if (SQ_FAILED(sq_getinteger(v, 2, &state))) {
        return sq_throwerror(v, _SC("failed to get state"));
      }
      g_pEngine->setInputState(state);
      return 0;
    }
    error("TODO: inputState: not implemented");
    return 0;
  }

  static SQInteger inputController(HSQUIRRELVM) {
    error("TODO: inputController: not implemented");
    return 0;
  }

  static SQInteger inputVerbs(HSQUIRRELVM v) {
    SQInteger on;
    if (SQ_FAILED(sq_getinteger(v, 2, &on))) {
      return sq_throwerror(v, _SC("failed to get isActive"));
    }
    g_pEngine->setInputVerbs(on);
    return 1;
  }

  static SQInteger is_table(HSQUIRRELVM v) {
    sq_pushbool(v, sq_gettype(v, 2) == OT_TABLE ? SQTrue : SQFalse);
    return 1;
  }

  static SQInteger ord(HSQUIRRELVM v) {
    const SQChar *letter;
    if (SQ_FAILED(sq_getstring(v, 2, &letter))) {
      return sq_throwerror(v, "Failed to get letter");
    }
    sq_pushinteger(v, (SQInteger) letter[0]);
    return 1;
  }

  static SQInteger is_string(HSQUIRRELVM v) {
    sq_pushbool(v, sq_gettype(v, 2) == OT_STRING ? SQTrue : SQFalse);
    return 1;
  }

  static SQInteger threadid(HSQUIRRELVM v) {
    auto pThread = EntityManager::getThreadFromVm(v);
    sq_pushinteger(v, pThread ? pThread->getId() : 0);
    return 1;
  }

  static SQInteger threadpauseable(HSQUIRRELVM v) {
    SQInteger threadId = 0;
    if (SQ_FAILED(sq_getinteger(v, 2, &threadId))) {
      return sq_throwerror(v, _SC("failed to get threadId"));
    }
    auto pThread = EntityManager::getThreadFromId(threadId);
    if (!pThread) {
      return 0;
    }
    SQInteger pauseable = 0;
    if (SQ_FAILED(sq_getinteger(v, 3, &pauseable))) {
      return sq_throwerror(v, _SC("failed to get pauseable"));
    }
    pThread->setPauseable(pauseable != 0);
    return 0;
  }
};

Engine *SystemPack::g_pEngine = nullptr;

} // namespace ng
