#include <algorithm>
#include <engge/Audio/SoundId.hpp>
#include <engge/Audio/SoundManager.hpp>
#include <engge/Engine/Cutscene.hpp>
#include <engge/Engine/Engine.hpp>
#include <engge/Engine/EntityManager.hpp>
#include <engge/Engine/Light.hpp>
#include <engge/Engine/ThreadBase.hpp>
#include <engge/Entities/Object.hpp>
#include <engge/Entities/Actor.hpp>
#include <engge/Room/Room.hpp>
#include <engge/System/Locator.hpp>

namespace ng {
Actor *EntityManager::getActorFromId(int id) {
  if (!EntityManager::isActor(id))
    return nullptr;

  for (auto &&actor : ng::Locator<ng::Engine>::get().getActors()) {
    if (actor->getId() == id)
      return actor.get();
  }
  return nullptr;
}

Object *EntityManager::getObjectFromId(int id) {
  if (!EntityManager::isObject(id))
    return nullptr;
  auto currentRoom = ng::Locator<ng::Engine>::get().getRoom();
  if (currentRoom) {
    for (auto &&obj : currentRoom->getObjects()) {
      if (obj->getId() == id)
        return obj.get();
    }
  }
  for (auto &&room : ng::Locator<ng::Engine>::get().getRooms()) {
    for (auto &&obj : room->getObjects()) {
      if (obj->getId() == id)
        return obj.get();
    }
  }
  return nullptr;
}

Room *EntityManager::getRoomFromId(int id) {
  if (!EntityManager::isRoom(id))
    return nullptr;
  for (auto &&room : ng::Locator<ng::Engine>::get().getRooms()) {
    if (room->getId() == id)
      return room.get();
  }
  return nullptr;
}

Sound *EntityManager::getSoundFromId(int id) {
  if (!EntityManager::isSound(id))
    return nullptr;

  for (auto sound : ng::Locator<ng::Engine>::get().getSoundManager().getSoundDefinitions()) {
    if (sound->getId() == id)
      return sound.get();
  }

  for (auto sound : ng::Locator<ng::Engine>::get().getSoundManager().getSounds()) {
    if (sound && sound->getId() == id)
      return sound.get();
  }
  return nullptr;
}

ThreadBase *EntityManager::getThreadFromId(int id) {
  if (!EntityManager::isThread(id))
    return nullptr;

  auto &threads = ng::Locator<ng::Engine>::get().getThreads();
  auto it = std::find_if(threads.begin(), threads.end(), [id](auto &t) -> bool {
    return t->getId() == id;
  });
  if (it != threads.end())
    return (*it).get();

  return nullptr;
}

ThreadBase *EntityManager::getThreadFromVm(HSQUIRRELVM v) {
  auto pCutscene = ng::Locator<ng::Engine>::get().getCutscene();
  if (pCutscene && pCutscene->getThread() == v) {
    return pCutscene;
  }

  auto &threads = ng::Locator<ng::Engine>::get().getThreads();
  auto it = std::find_if(threads.begin(), threads.end(), [v](auto &t) -> bool {
    return t->getThread() == v;
  });
  if (it != threads.end())
    return (*it).get();

  return nullptr;
}

Entity *EntityManager::getEntity(HSQUIRRELVM v, SQInteger index) {
  return EntityManager::getScriptObject<Entity>(v, index);
}

Object *EntityManager::getObject(HSQUIRRELVM v, SQInteger index) {
  return EntityManager::getScriptObject<Object>(v, index);
}

Room *EntityManager::getRoom(HSQUIRRELVM v, SQInteger index) { return EntityManager::getScriptObject<Room>(v, index); }

Actor *EntityManager::getActor(HSQUIRRELVM v, SQInteger index) {
  return EntityManager::getScriptObject<Actor>(v,
                                               index);
}

SoundId *EntityManager::getSound(HSQUIRRELVM v, SQInteger index) {
  return EntityManager::getScriptObject<SoundId>(v,
                                                 index);
}

std::shared_ptr<SoundDefinition> EntityManager::getSoundDefinition(HSQUIRRELVM v,
                                                                   SQInteger index) {
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

  sq_pushobject(v, object);
  sq_pushstring(v, _SC("_id"), -1);
  if (SQ_FAILED(sq_rawget(v, -2))) {
    return nullptr;
  }

  SQInteger id = 0;
  if (SQ_FAILED(sq_getinteger(v, -1, &id))) {
    return nullptr;
  }
  sq_pop(v, 2);

  if (EntityManager::isSound(id)) {
    for (auto sound : ng::Locator<ng::Engine>::get().getSoundManager().getSoundDefinitions()) {
      if (sound->getId() == id) {
        return sound;
      }
    }
  }

  return nullptr;
}

std::shared_ptr<SoundDefinition> EntityManager::getSoundDefinition(HSQUIRRELVM v, const std::string &name) {
  auto top = sq_gettop(v);
  sq_pushroottable(v);
  sq_pushstring(v, name.data(), -1);
  sq_get(v, -2);

  auto sound = getSoundDefinition(v, -1);
  sq_settop(v, top);
  return sound;
}

bool EntityManager::tryGetLight(HSQUIRRELVM v, SQInteger index, Light *&light) {
  HSQOBJECT obj;
  light = nullptr;
  if (SQ_SUCCEEDED(sq_getstackobj(v, index, &obj)) && sq_isinteger(obj) && sq_objtointeger(&obj) == 0) {
    return false;
  }
  light = EntityManager::getScriptObject<Light>(v, index);
  return true;
}
}