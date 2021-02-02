#include "RoomTriggerThread.hpp"
#include <utility>

namespace ng {
RoomTriggerThread::RoomTriggerThread(HSQUIRRELVM vm, std::string name, HSQOBJECT thread_obj)
    : m_vm(vm), m_name(std::move(name)), m_thread_obj(thread_obj) {
  sq_addref(m_vm, &m_thread_obj);
}

RoomTriggerThread::~RoomTriggerThread() {
  sq_release(m_vm, &m_thread_obj);
}

HSQUIRRELVM RoomTriggerThread::getThread() const {
  return m_thread_obj._unVal.pThread;
}

std::string RoomTriggerThread::getName() const {
  return m_name;
}
}
