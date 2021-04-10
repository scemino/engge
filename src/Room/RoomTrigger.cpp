#include "RoomTrigger.hpp"
#include "RoomTriggerThread.hpp"
#include <engge/System/Locator.hpp>
#include <engge/System/Logger.hpp>
#include <engge/Entities/Object.hpp>
#include <engge/Scripting/ScriptEngine.hpp>
#include <squirrel.h>

namespace ng {
RoomTrigger::RoomTrigger(Engine &engine, Object &object, HSQOBJECT inside, HSQOBJECT outside)
    : m_engine(engine), m_object(object), m_inside(inside), m_outside(outside) {
  m_vm = ScriptEngine::getVm();
  sq_addref(m_vm, &inside);
  sq_addref(m_vm, &outside);

  SQInteger top = sq_gettop(m_vm);

  const SQChar *insideName{nullptr};
  SQInteger nfreevars;
  sq_pushobject(m_vm, m_inside);
  sq_getclosureinfo(m_vm, -1, &m_insideParamsCount, &nfreevars);
  if (SQ_SUCCEEDED(sq_getclosurename(m_vm, -1))) {
    sq_getstring(m_vm, -1, &insideName);
    if (insideName)
      m_insideName = insideName;
  }

  const SQChar *outsideName{nullptr};
  sq_pushobject(m_vm, m_outside);
  sq_getclosureinfo(m_vm, -1, &m_outsideParamsCount, &nfreevars);
  if (SQ_SUCCEEDED(sq_getclosurename(m_vm, -1))) {
    sq_getstring(m_vm, -1, &outsideName);
    if (outsideName)
      m_outsideName = outsideName;
  }
  sq_settop(m_vm, top);
  m_name.append(m_object.getName())
      .append(" [")
      .append(m_insideName)
      .append(",")
      .append(m_outsideName)
      .append("]");

  trace("add trigger: {}", m_name);
}

RoomTrigger::~RoomTrigger() {
  trace("end room trigger thread: {}", m_id);
  sq_release(m_vm, &m_inside);
  sq_release(m_vm, &m_outside);
}

HSQUIRRELVM RoomTrigger::createThread() {
  HSQOBJECT thread_obj{};
  sq_resetobject(&thread_obj);

  HSQUIRRELVM thread = sq_newthread(m_vm, 1024);
  if (SQ_FAILED(sq_getstackobj(m_vm, -1, &thread_obj))) {
    error("Couldn't get coroutine thread from stack");
    return {};
  }

  auto pUniquethread = std::make_unique<RoomTriggerThread>(m_vm, m_name, thread_obj);
  sq_pop(m_vm , 1); // pop thread
  m_id = pUniquethread->getId();
  trace("start room trigger thread: {}", m_id);
  m_engine.addThread(std::move(pUniquethread));
  return thread;
}

void RoomTrigger::trigCore() {
  auto actor = m_engine.getCurrentActor();
  if (!actor)
    return;

  auto inObjectHotspot = m_object.getRealHotspot().contains(actor->getPosition());
  if (!m_isInside && inObjectHotspot) {
    m_isInside = true;

    std::vector<HSQOBJECT> params;
    if (m_insideParamsCount == 2) {
      params.push_back(m_inside);
      params.push_back(m_object.getTable());
      params.push_back(actor->getTable());
    } else {
      params.push_back(m_inside);
      params.push_back(m_object.getTable());
    }

    std::string name;
    name.append("inside");
    if (!m_insideName.empty()) {
      name.append(" ").append(m_insideName);
    }
    callTrigger(params, name);
  } else if (m_isInside && !inObjectHotspot) {
    m_isInside = false;
    if (m_outside._type != SQObjectType::OT_NULL) {
      std::vector<HSQOBJECT> params;
      if (m_outsideParamsCount == 2) {
        params.push_back(m_outside);
        params.push_back(m_object.getTable());
        params.push_back(actor->getTable());
      } else {
        params.push_back(m_outside);
        params.push_back(m_object.getTable());
      }

      std::string name;
      name.append("outside");
      if (!m_outsideName.empty()) {
        name.append(" ").append(m_outsideName);
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

  trace("call room {} trigger ({})", name, m_id);
  if (SQ_FAILED(sq_call(thread, params.size() - 1, SQFalse, SQTrue))) {
    error("failed to call room {} trigger", name);
    return;
  }
}

std::string RoomTrigger::getName() { return m_name; }
} // namespace ng
