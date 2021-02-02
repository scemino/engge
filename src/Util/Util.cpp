#include "../../extlibs/squirrel/squirrel/sqpcheader.h"
#include "../../extlibs/squirrel/squirrel/sqvm.h"
#include "../../extlibs/squirrel/squirrel/sqstring.h"
#include "../../extlibs/squirrel/squirrel/sqtable.h"
#include "../../extlibs/squirrel/squirrel/sqarray.h"
#include "../../extlibs/squirrel/squirrel/sqfuncproto.h"
#include "../../extlibs/squirrel/squirrel/sqclosure.h"
#include "engge/Util/Util.hpp"
#include "engge/Engine/EntityManager.hpp"
#include "engge/Scripting/ScriptEngine.hpp"

namespace {
constexpr static const char *_objectKey = "_objectKey";
constexpr static const char *_roomKey = "_roomKey";
constexpr static const char *_actorKey = "_actorKey";
constexpr static const char *_idKey = "_id";

ngf::GGPackValue toGGPackValue(SQObject obj, bool checkId, const std::string &tableKey = "");

bool canSave(HSQOBJECT obj) {
  switch (sq_type(obj)) {
  case OT_STRING: return true;
  case OT_INTEGER: return true;
  case OT_BOOL:return true;
  case OT_FLOAT:return true;
  case OT_NULL:return true;
  case OT_TABLE:return true;
  case OT_ARRAY:return true;
  default:return false;
  }
}

ngf::GGPackValue toArray(HSQOBJECT obj) {
  ngf::GGPackValue array;
  SQObjectPtr refpos;
  SQObjectPtr outkey, outvar;
  SQInteger res;
  while ((res = obj._unVal.pArray->Next(refpos, outkey, outvar)) != -1) {
    if (canSave(outvar)) {
      array.push_back(toGGPackValue(outvar, true));
    }
    refpos._type = OT_INTEGER;
    refpos._unVal.nInteger = res;
  }
  return array;
}

ngf::GGPackValue toTable(HSQOBJECT table, bool checkId, const std::string &tableKey = "") {
  ngf::GGPackValue hash;
  int id;
  if (checkId && ng::ScriptEngine::get(table, _idKey, id)) {
    if (ng::EntityManager::isActor(id)) {
      auto pActor = ng::EntityManager::getActorFromId(id);
      if (pActor && pActor->getKey() != tableKey) {
        hash[_actorKey] = pActor->getKey();
        return hash;
      }
      return nullptr;
    }
    if (ng::EntityManager::isObject(id)) {
      auto pObj = ng::EntityManager::getObjectFromId(id);
      if (pObj && pObj->getKey() != tableKey) {
        auto pRoom = pObj->getRoom();
        if (pRoom && pRoom->isPseudoRoom()) {
          hash[_roomKey] = pRoom->getName();
        }
        hash[_objectKey] = pObj->getKey();
        return hash;
      }
      return nullptr;
    }
    if (ng::EntityManager::isRoom(id)) {
      auto pRoom = ng::EntityManager::getRoomFromId(id);
      if (pRoom && pRoom->getName() != tableKey) {
        hash[_roomKey] = pRoom->getName();
        return hash;
      }
      return nullptr;
    }
    return nullptr;
  }

  SQObjectPtr refpos;
  SQObjectPtr outkey, outvar;
  SQInteger res;
  while ((res = table._unVal.pTable->Next(false, refpos, outkey, outvar)) != -1) {
    std::string key = _stringval(outkey);
    if (!key.empty() && key[0] != '_' && canSave(outvar)) {
      auto value = toGGPackValue(outvar, true, key);
      if (!value.isNull()) {
        hash[key] = value;
      }
    }
    refpos._type = OT_INTEGER;
    refpos._unVal.nInteger = res;
  }
  return hash;
}

ngf::GGPackValue toGGPackValue(SQObject obj, bool checkId, const std::string &tableKey) {
  switch (sq_type(obj)) {
  case OT_STRING:return _stringval(obj);
  case OT_INTEGER:
  case OT_BOOL:return static_cast<int>(_integer(obj));
  case OT_FLOAT:return static_cast<float >(_float(obj));
  case OT_NULL:return nullptr;
  case OT_TABLE: return toTable(obj, checkId, tableKey);
  case OT_ARRAY: {
    return toArray(obj);
  }
  default:assert(false);
  }
}
}

namespace ng {
ngf::frect getGlobalBounds(const ng::Text &text) {
  return ngf::transform(text.getTransform().getTransform(), text.getLocalBounds());
}

ngf::frect getGlobalBounds(const ngf::Sprite &sprite) {
  return ngf::transform(sprite.getTransform().getTransform(), sprite.getLocalBounds());
}

ngf::GGPackValue toGGPackValue(HSQOBJECT obj) {
  return ::toGGPackValue((SQObject) obj, false);
}
}
