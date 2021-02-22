#pragma once
#include <squirrel.h>
#include <engge/Room/Room.hpp>
#include <engge/Engine/Engine.hpp>
#include <engge/Engine/Light.hpp>
#include <engge/Entities/Object.hpp>
#include <engge/System/Locator.hpp>

namespace ng {
class Actor;
class Entity;
class Sound;
class SoundDefinition;
class SoundId;
class ThreadBase;

class EntityManager {
private:
  static const int START_ACTORID = 1000;
  static const int END_ACTORID = 2000;
  static const int START_ROOMID = 2000;
  static const int END_ROOMID = 3000;
  static const int START_OBJECTID = 3000;
  static const int END_OBJECTID = 100000;
  static const int START_LIGHTID = 100000;
  static const int END_LIGHTID = 200000;
  static const int START_SOUNDID = 200000;
  static const int END_SOUNDID = 300000;
  static const int START_THREADID = 300000;
  static const int END_THREADID = 8000000;
  static const int START_CALLBACKID = 8000000;
  static const int END_CALLBACKID = 10000000;

public:
  inline int getActorId() { return START_ACTORID + m_actorId++; }
  inline int getRoomId() { return START_ROOMID + m_roomId++; }
  inline int getObjectId() { return START_OBJECTID + m_objectId++; }
  inline int getLightId() { return START_LIGHTID + m_lightId++; }
  inline int getSoundId() { return START_SOUNDID + m_soundId++; }
  inline int getCallbackId() { return START_CALLBACKID + m_callbackId++; }
  inline void setCallbackId(int id) { m_callbackId = id - START_CALLBACKID; }
  inline int getThreadId() { return START_THREADID + m_threadId++; }

  static Actor *getActorFromId(int id);
  static Room *getRoomFromId(int id);
  static Object *getObjectFromId(int id);
  static Sound *getSoundFromId(int id);
  static ThreadBase *getThreadFromId(int id);
  static ThreadBase *getThreadFromVm(HSQUIRRELVM v);

  template<typename TScriptObject>
  static TScriptObject *getScriptObject(HSQUIRRELVM v, SQInteger index);
  template<typename TScriptObject>
  static TScriptObject *getScriptObject(HSQUIRRELVM v, HSQOBJECT obj);
  template<typename TScriptObject>
  static TScriptObject *getScriptObjectFromId(int id);

  static Entity *getEntity(HSQUIRRELVM v, SQInteger index);
  static Object *getObject(HSQUIRRELVM v, SQInteger index);
  static Room *getRoom(HSQUIRRELVM v, SQInteger index);
  static Actor *getActor(HSQUIRRELVM v, SQInteger index);
  static SoundId *getSound(HSQUIRRELVM v, SQInteger index);
  static std::shared_ptr<SoundDefinition> getSoundDefinition(HSQUIRRELVM v, SQInteger index);
  static std::shared_ptr<SoundDefinition> getSoundDefinition(HSQUIRRELVM v, const std::string &name);

  static bool tryGetLight(HSQUIRRELVM v, SQInteger index, Light *&light);

  static bool isActor(int id) { return isBetween(id, START_ACTORID, END_ACTORID); }
  static bool isRoom(int id) { return isBetween(id, START_ROOMID, END_ROOMID); }
  static bool isObject(int id) { return isBetween(id, START_OBJECTID, END_OBJECTID); }
  static bool isLight(int id) { return isBetween(id, START_LIGHTID, END_LIGHTID); }
  static bool isSound(int id) { return isBetween(id, START_SOUNDID, END_SOUNDID); }
  static bool isThread(int id) { return isBetween(id, START_THREADID, END_THREADID); }

private:
  static inline bool isBetween(int id, int min, int max) { return id >= min && id < max; }

private:
  int m_actorId{0};
  int m_roomId{0};
  int m_objectId{0};
  int m_lightId{0};
  int m_soundId{0};
  int m_callbackId{0};
  int m_threadId{0};
};

template<typename TScriptObject>
TScriptObject *EntityManager::getScriptObject(HSQUIRRELVM v, HSQOBJECT obj) {
  sq_pushobject(v, obj);
  sq_pushstring(v, _SC("_id"), -1);
  if (SQ_FAILED(sq_rawget(v, -2))) {
    return nullptr;
  }

  SQInteger id = 0;
  if (SQ_FAILED(sq_getinteger(v, -1, &id))) {
    return nullptr;
  }
  sq_pop(v, 2);

  return getScriptObjectFromId<TScriptObject>(id);
}

template<typename TScriptObject>
TScriptObject *EntityManager::getScriptObject(HSQUIRRELVM v, SQInteger index) {
  auto type = sq_gettype(v, index);
  // is it a table?
  if (type != OT_TABLE) {
    return nullptr;
  }

  HSQOBJECT object;
  sq_resetobject(&object);
  if (SQ_FAILED(sq_getstackobj(v, index, &object))) {
    return nullptr;
  }
  return getScriptObject<TScriptObject>(v, object);
}

template<typename TScriptObject>
TScriptObject *EntityManager::getScriptObjectFromId(int id) {
  if (EntityManager::isActor(id)) {
    return dynamic_cast<TScriptObject *>(getActorFromId(id));
  }

  if (EntityManager::isRoom(id)) {
    return dynamic_cast<TScriptObject *>(getRoomFromId(id));
  }

  if (EntityManager::isLight(id)) {
    for (auto &&room : ng::Locator<ng::Engine>::get().getRooms()) {
      for (auto &light : room->getLights()) {
        if (light.getId() == id)
          return dynamic_cast<TScriptObject *>(&light);
      }
    }
    return nullptr;
  }

  if (EntityManager::isObject(id)) {
    return dynamic_cast<TScriptObject *>(getObjectFromId(id));
  }

  if (EntityManager::isSound(id)) {
    return dynamic_cast<TScriptObject *>(getSoundFromId(id));
  }

  return nullptr;
}

}
