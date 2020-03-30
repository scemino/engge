#pragma once

namespace ng {
class ResourceManager {
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
  static const int START_CALLBACKID = 300000;
  static const int END_CALLBACKID = 400000;
  static const int START_THREADID = 400000;
  static const int END_THREADID = 500000;

public:
  inline int getActorId() { return START_ACTORID + _actorId++; }
  inline int getRoomId() { return START_ROOMID + _roomId++; }
  inline int getObjectId() { return START_OBJECTID + _objectId++; }
  inline int getLightId() { return START_LIGHTID + _lightId++; }
  inline int getSoundId() { return START_SOUNDID + _soundId++; }
  inline int getCallbackId() { return START_CALLBACKID + _callbackId++; }
  inline int getThreadId() { return START_THREADID + _threadId++; }

  static bool isActor(int id) { return isBetween(id, START_ACTORID, END_ACTORID); }
  static bool isRoom(int id) { return isBetween(id, START_ROOMID, END_ROOMID); }
  static bool isObject(int id) { return isBetween(id, START_OBJECTID, END_OBJECTID); }
  static bool isLight(int id) { return isBetween(id, START_LIGHTID, END_LIGHTID); }
  static bool isSound(int id) { return isBetween(id, START_SOUNDID, END_SOUNDID); }
  static bool isCallbackId(int id) { return isBetween(id, START_CALLBACKID, END_CALLBACKID); }
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
