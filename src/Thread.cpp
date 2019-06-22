#include "Thread.h"

namespace ng
{
Thread::Thread(HSQUIRRELVM v, HSQOBJECT thread_obj, HSQOBJECT env_obj, HSQOBJECT closureObj, const std::vector<HSQOBJECT> &args)
    : _v(v), _thread_obj(thread_obj), _env_obj(env_obj), _closureObj(closureObj), _args(args)
{
    sq_addref(_v, &_thread_obj);
}

Thread::~Thread()
{
    sq_release(_v, &_thread_obj);
}

HSQUIRRELVM Thread::getThread() { return _thread_obj._unVal.pThread; }

bool Thread::call()
{
    auto thread = getThread();
    // call the closure in the thread
    SQInteger top = sq_gettop(thread);
    sq_pushobject(thread, _closureObj);
    sq_pushobject(thread, _env_obj);
    for (auto arg : _args)
    {
        sq_pushobject(thread, arg);
    }
    if (SQ_FAILED(sq_call(thread, 1 + _args.size(), SQFalse, SQTrue)))
    {
        sq_settop(thread, top);
        return false;
    }
    return true;
}
} // namespace ng

