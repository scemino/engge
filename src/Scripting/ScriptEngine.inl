#include "Entities/Objects/Object.hpp"
#include "../../extlibs/squirrel/squirrel/sqobject.h"

namespace ng {
template<>
bool ScriptEngine::get(HSQUIRRELVM v, SQInteger index, bool &result) {
  SQInteger integer = 0;
  auto status = SQ_SUCCEEDED(sq_getinteger(v, index, &integer));
  result = integer != 0;
  return status;
}

template<>
bool ScriptEngine::get(HSQUIRRELVM v, SQInteger index, int &result) {
  SQInteger integer = 0;
  auto status = SQ_SUCCEEDED(sq_getinteger(v, index, &integer));
  result = integer;
  return status;
}

template<>
bool ScriptEngine::get(HSQUIRRELVM v, SQInteger index, const char *&result) {
  const SQChar *text = nullptr;
  auto status = SQ_SUCCEEDED(sq_getstring(v, index, &text));
  result = text;
  return status;
}

template<>
bool ScriptEngine::get(HSQUIRRELVM v, SQInteger index, float &result) {
  SQFloat value = 0;
  auto status = SQ_SUCCEEDED(sq_getfloat(v, index, &value));
  result = value;
  return status;
}

template<>
void ScriptEngine::push<bool>(HSQUIRRELVM v, bool value) {
  sq_pushbool(v, value ? SQTrue : SQFalse);
}

template<>
void ScriptEngine::push<int>(HSQUIRRELVM v, int value) {
  sq_pushinteger(v, value);
}

template<>
void ScriptEngine::push<const char *>(HSQUIRRELVM v, const char *value) {
  sq_pushstring(v, value, -1);
}

template<>
void ScriptEngine::push<char *>(HSQUIRRELVM v, char *value) {
  sq_pushstring(v, value, -1);
}

template<>
void ScriptEngine::push<std::string>(HSQUIRRELVM v, std::string value) {
  if (!value.empty()) {
    sq_pushstring(v, value.data(), -1);
  } else {
    sq_pushnull(v);
  }
}

template<>
void ScriptEngine::push<SQFloat>(HSQUIRRELVM v, SQFloat value) {
  sq_pushfloat(v, value);
}

template<>
void ScriptEngine::push<sf::Vector2i>(HSQUIRRELVM v, sf::Vector2i pos) {
  sq_newtable(v);
  sq_pushstring(v, _SC("x"), -1);
  sq_pushinteger(v, static_cast<int>(pos.x));
  sq_newslot(v, -3, SQFalse);
  sq_pushstring(v, _SC("y"), -1);
  sq_pushinteger(v, static_cast<int>(pos.y));
  sq_newslot(v, -3, SQFalse);
}

template<>
void ScriptEngine::push<sf::IntRect>(HSQUIRRELVM v, sf::IntRect rect) {
  sq_newtable(v);
  sq_pushstring(v, _SC("x1"), -1);
  sq_pushinteger(v, rect.left);
  sq_newslot(v, -3, SQFalse);
  sq_pushstring(v, _SC("y1"), -1);
  sq_pushinteger(v, rect.top);
  sq_newslot(v, -3, SQFalse);
  sq_pushstring(v, _SC("x2"), -1);
  sq_pushinteger(v, rect.left + rect.width);
  sq_newslot(v, -3, SQFalse);
  sq_pushstring(v, _SC("y2"), -1);
  sq_pushinteger(v, rect.top + rect.height);
  sq_newslot(v, -3, SQFalse);
}

template<>
void ScriptEngine::push<sf::Vector2f>(HSQUIRRELVM v, sf::Vector2f pos) {
  return ScriptEngine::push(v, (sf::Vector2i) pos);
}

template<>
void ScriptEngine::push<Entity *>(HSQUIRRELVM v, Entity *pEntity) {
  if (!pEntity) {
    sq_pushnull(v);
    return;
  }
  sq_pushobject(v, pEntity->getTable());
}

template<>
void ScriptEngine::push<Actor *>(HSQUIRRELVM v, Actor *pActor) {
  if (!pActor) {
    sq_pushnull(v);
    return;
  }
  sq_pushobject(v, pActor->getTable());
}

template<>
void ScriptEngine::push<Room *>(HSQUIRRELVM v, Room *pRoom) {
  if (!pRoom) {
    sq_pushnull(v);
    return;
  }
  sq_pushobject(v, pRoom->getTable());
}

template<>
void ScriptEngine::push<Object *>(HSQUIRRELVM v, Object *pObject) {
  if (!pObject) {
    sq_pushnull(v);
    return;
  }
  sq_pushobject(v, pObject->getTable());
}

template<>
void ScriptEngine::push<ScriptObject *>(HSQUIRRELVM v, ScriptObject *pObject) {
  if (!pObject) {
    sq_pushnull(v);
    return;
  }
  auto pEntity = dynamic_cast<Entity*>(pObject);
  if(pEntity) {
    sq_pushobject(v, pEntity->getTable());
    return;
  }
  auto pRoom = dynamic_cast<Room*>(pObject);
  if(pRoom) {
    sq_pushobject(v, pRoom->getTable());
    return;
  }
  throw std::logic_error("Unable to push an unknown ScriptObject type");
}

template<>
void ScriptEngine::push<std::nullptr_t>(HSQUIRRELVM v, std::nullptr_t) {
  sq_pushnull(v);
}

template<>
void ScriptEngine::push<HSQOBJECT>(HSQUIRRELVM v, HSQOBJECT obj) {
  sq_pushobject(v, obj);
}

template<>
void ScriptEngine::push<SQObjectPtr>(HSQUIRRELVM v, SQObjectPtr obj) {
  sq_pushobject(v, obj);
}

}
