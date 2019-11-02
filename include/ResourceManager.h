#pragma once

namespace ng
{
class ResourceManager
{
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
    
public:
    int getActorId() {return START_ACTORID+_actorId++;}
    int getRoomId() {return START_ROOMID+_roomId++;}
    int getObjectId() {return START_OBJECTID+_objectId++;}
    int getLightId() {return START_LIGHTID+_lightId++;}
    int getSoundId() {return START_SOUNDID+_soundId++;}

    static bool isActor(int id) { return isBetween(id, START_ACTORID, END_ACTORID); }
    static bool isRoom(int id) { return isBetween(id, START_ROOMID, END_ROOMID); }
    static bool isObject(int id) { return isBetween(id, START_OBJECTID, END_OBJECTID); }
    static bool isLight(int id) { return isBetween(id, START_LIGHTID, END_LIGHTID); }
    static bool isSound(int id) { return isBetween(id, START_SOUNDID, END_SOUNDID); }

private:
    static bool isBetween(int id, int min, int max) { return id>=min && id<max; }

private:
    int _actorId{0};
    int _roomId{0};
    int _objectId{0};
    int _lightId{0};
    int _soundId{0};
};
}
