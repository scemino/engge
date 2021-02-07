#include "RoomTrigger.hpp"
#include "RoomTriggerThread.hpp"
#include <engge/System/Locator.hpp>
#include <engge/System/Logger.hpp>
#include <engge/Entities/Object.hpp>
#include <engge/Scripting/ScriptEngine.hpp>
#include <squirrel.h>

namespace ng {
RoomTrigger::RoomTrigger(Engine &engine, Object &object, HSQOBJECT inside, HSQOBJECT outside)
    : _engine(engine), _object(object), _inside(inside), _outside(outside) {
  _vm = ScriptEngine::getVm();
  sq_addref(_vm, &inside);
  sq_addref(_vm, &outside);

  SQInteger top = sq_gettop(_vm);

  const SQChar *insideName{nullptr};
  SQInteger nfreevars;
  sq_pushobject(_vm, _inside);
  sq_getclosureinfo(_vm, -1, &_insideParamsCount, &nfreevars);
  if (SQ_SUCCEEDED(sq_getclosurename(_vm, -1))) {
    sq_getstring(_vm, -1, &insideName);
    if (insideName)
      _insideName = insideName;
  }

  const SQChar *outsideName{nullptr};
  sq_pushobject(_vm, _outside);
  sq_getclosureinfo(_vm, -1, &_outsideParamsCount, &nfreevars);
  if (SQ_SUCCEEDED(sq_getclosurename(_vm, -1))) {
    sq_getstring(_vm, -1, &outsideName);
    if (outsideName)
      _outsideName = outsideName;
  }
  sq_settop(_vm, top);
  _name.append(_object.getName())
      .append(" [")
      .append(_insideName)
      .append(",")
      .append(_outsideName)
      .append("]");

  trace("add trigger: {}", _name);
}

RoomTrigger::~RoomTrigger() {
  trace("end room trigger thread: {}", _id);
  sq_release(_vm, &_inside);
  sq_release(_vm, &_outside);
}

HSQUIRRELVM RoomTrigger::createThread() {
  HSQOBJECT thread_obj{};
  sq_resetobject(&thread_obj);

  HSQUIRRELVM thread = sq_newthread(_vm, 1024);
  if (SQ_FAILED(sq_getstackobj(_vm, -1, &thread_obj))) {
    error("Couldn't get coroutine thread from stack");
    return {};
  }

  auto pUniquethread = std::make_unique<RoomTriggerThread>(_vm, _name, thread_obj);
  sq_pop(_vm ,1); // pop thread
  _id = pUniquethread->getId();
  trace("start room trigger thread: {}", _id);
  _engine.addThread(std::move(pUniquethread));
  return thread;
}

void RoomTrigger::trigCore() {
  auto actor = _engine.getCurrentActor();
  if (!actor)
    return;

  auto inObjectHotspot = _object.getRealHotspot().contains(actor->getPosition());
  if (!_isInside && inObjectHotspot) {
    _isInside = true;

    std::vector<HSQOBJECT> params;
    if (_insideParamsCount == 2) {
      params.push_back(_inside);
      params.push_back(_object.getTable());
      params.push_back(actor->getTable());
    } else {
      params.push_back(_inside);
      params.push_back(_object.getTable());
    }

    std::string name;
    name.append("inside");
    if (!_insideName.empty()) {
      name.append(" ").append(_insideName);
    }
    callTrigger(params, name);
  } else if (_isInside && !inObjectHotspot) {
    _isInside = false;
    if (_outside._type != SQObjectType::OT_NULL) {
      std::vector<HSQOBJECT> params;
      if (_outsideParamsCount == 2) {
        params.push_back(_outside);
        params.push_back(_object.getTable());
        params.push_back(actor->getTable());
      } else {
        params.push_back(_outside);
        params.push_back(_object.getTable());
      }

      std::string name;
      name.append("outside");
      if (!_outsideName.empty()) {
        name.append(" ").append(_outsideName);
      }
      callTrigger(params, "outside");
    }
  }
}

void RoomTrigger::callTrigger(std::vector<HSQOBJECT> &params, const std::string &name) {
  auto thread = createThread();
  for (auto param : params) {
    sq_pushobject(thread, param);
  }

  trace("call room {} trigger ({})", name, _id);
  if (SQ_FAILED(sq_call(thread, params.size() - 1, SQFalse, SQTrue))) {
    error("failed to call room {} trigger", name);
    return;
  }
}

std::string RoomTrigger::getName() { return _name; }
} // namespace ng
