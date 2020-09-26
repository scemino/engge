#pragma once

#include "squirrel.h"

namespace ng {
class Actor;
class Entity;
class Light;
class Object;
class Room;
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
  inline int getActorId() { return START_ACTORID + _actorId++; }
  inline int getRoomId() { return START_ROOMID + _roomId++; }
  inline int getObjectId() { return START_OBJECTID + _objectId++; }
  inline int getLightId() { return START_LIGHTID + _lightId++; }
  inline int getSoundId() { return START_SOUNDID + _soundId++; }
  inline int getCallbackId() { return START_CALLBACKID + _callbackId++; }
  inline void setCallbackId(int id) { _callbackId = id - START_CALLBACKID; }
  inline int getThreadId() { return START_THREADID + _threadId++; }

  static Actor *getActorFromId(int id);
  static Room *getRoomFromId(int id);
  static Object *getObjectFromId(int id);
  static Sound *getSoundFromId(int id);
  static ThreadBase *getThreadFromId(int id);
  static ThreadBase *getThreadFromVm(HSQUIRRELVM v);

  template <typename TScriptObject>
  static TScriptObject *getScriptObject(HSQUIRRELVM v, SQInteger index);
  template <typename TScriptObject>
  static TScriptObject *getScriptObject(HSQUIRRELVM v, HSQOBJECT obj);
  template <typename TScriptObject>
  static TScriptObject *getScriptObjectFromId(int id);

  static Entity *getEntity(HSQUIRRELVM v, SQInteger index);
  static Object *getObject(HSQUIRRELVM v, SQInteger index);
  static Room *getRoom(HSQUIRRELVM v, SQInteger index);
  static Actor *getActor(HSQUIRRELVM v, SQInteger index);
  static SoundId *getSound(HSQUIRRELVM v, SQInteger index);
  static SoundDefinition *getSoundDefinition(HSQUIRRELVM v, SQInteger index);

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
  int _actorId{0};
  int _roomId{0};
  int _objectId{0};
  int _lightId{0};
  int _soundId{0};
  int _callbackId{0};
  int _threadId{0};
};
}
