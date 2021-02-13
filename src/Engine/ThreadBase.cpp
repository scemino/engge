#include <engge/Engine/ThreadBase.hpp>
#include "engge/System/Locator.hpp"
#include "engge/System/Logger.hpp"
#include <engge/Engine/EntityManager.hpp>

namespace ng {
ThreadBase::ThreadBase() {
  m_id = Locator<EntityManager>::get().getThreadId();
}

ThreadBase::~ThreadBase() {
  trace("stop thread {}", m_id);
}

void ThreadBase::stop() {
  m_isStopped = true;
}

bool ThreadBase::pause() {
  if (!m_isPauseable)
    return false;
  suspend();
  return true;
}

void ThreadBase::suspend() {
  if (isSuspended())
    return;
  sq_suspendvm(getThread());
  m_isSuspended = true;
}

void ThreadBase::resume() {
  if (!isSuspended())
    return;
  sq_wakeupvm(getThread(), SQFalse, SQFalse, SQTrue, SQFalse);
  m_isSuspended = false;
}

bool ThreadBase::isSuspended() const {
  if (m_isSuspended)
    return true;
  auto state = sq_getvmstate(getThread());
  return state != SQ_VMSTATE_RUNNING;
}

bool ThreadBase::isStopped() const {
  if (m_isStopped)
    return true;
  auto state = sq_getvmstate(getThread());
  return state == SQ_VMSTATE_IDLE;
}
}