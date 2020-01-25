#pragma once
#include "squirrel.h"
#include "Scripting/ScriptObject.hpp"
#include "System/Locator.hpp"
#include "System/Logger.hpp"
#include "Engine/ResourceManager.hpp"

namespace ng
{
class ThreadBase: public ScriptObject
{
protected:
    ThreadBase() { _id = Locator<ResourceManager>::get().getThreadId(); }
    
public:
    ~ThreadBase() override
    { 
        trace("stop thread {}", _id);
    }
    
    [[nodiscard]] virtual HSQUIRRELVM getThread() const = 0;

    inline void setPauseable(bool value) { _isPauseable = value; }
    inline bool isPauseable() const { return _isPauseable; }
    virtual bool isGlobal() const { return false; }

    inline void stop() { _isStopped = true; }

    bool pause()
    {
        if(!_isPauseable) return false;
        suspend();
        return true;
    }

    void suspend()
    {
        if(isSuspended()) return;
        sq_suspendvm(getThread());
        _isSuspended = true;
    }

    void resume()
    {
        if(!isSuspended()) return;
        sq_wakeupvm(getThread(), SQFalse, SQFalse, SQTrue, SQFalse);
        _isSuspended = false;
    }

    [[nodiscard]] bool isSuspended() const
    { 
        if(_isSuspended) return true;
        auto state = sq_getvmstate(getThread());
        return state != SQ_VMSTATE_RUNNING;
    }

    [[nodiscard]] virtual bool isStopped() const
    {
        if(_isStopped) return true;
        auto state = sq_getvmstate(getThread());
        return state == SQ_VMSTATE_IDLE;
    }

private:
    bool _isSuspended{false};
    bool _isPauseable{true};
    bool _isStopped{false};
};
}
