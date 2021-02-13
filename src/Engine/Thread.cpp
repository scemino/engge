#include <squirrel.h>
#include "engge/System/Locator.hpp"
#include "engge/Engine/EntityManager.hpp"
#include "engge/Engine/Thread.hpp"
#include <utility>

namespace ng {
Thread::Thread(std::string name, bool isGlobal,
               HSQUIRRELVM v,
               HSQOBJECT thread_obj,
               HSQOBJECT env_obj,
               HSQOBJECT closureObj,
               std::vector<HSQOBJECT> args)
    : m_name(std::move(name)), m_v(v), m_threadObj(thread_obj), m_envObj(env_obj), m_closureObj(closureObj), m_args(std::move(args)),
      m_isGlobal(isGlobal) {
  sq_addref(m_v, &m_threadObj);
  sq_addref(m_v, &m_envObj);
  sq_addref(m_v, &m_closureObj);
  m_id = Locator<EntityManager>::get().getThreadId();
}

Thread::~Thread() {
  sq_release(m_v, &m_threadObj);
  sq_release(m_v, &m_envObj);
  sq_release(m_v, &m_closureObj);
}

std::string Thread::getName() const {
  return m_name;
}

HSQUIRRELVM Thread::getThread() const { return m_threadObj._unVal.pThread; }

bool Thread::call() {
  auto thread = getThread();
  // call the closure in the thread
  SQInteger top = sq_gettop(thread);
  sq_pushobject(thread, m_closureObj);
  sq_pushobject(thread, m_envObj);
  for (auto arg : m_args) {
    sq_pushobject(thread, arg);
  }
  if (SQ_FAILED(sq_call(thread, 1 + m_args.size(), SQFalse, SQTrue))) {
    sq_settop(thread, top);
    return false;
  }
  return true;
}

} // namespace ng

