#pragma once
#include "Engine.h"
#include "ThreadBase.h"
#include "Trigger.h"
#include "squirrel.h"

namespace ng
{
class _RoomTriggerThread : public ThreadBase
{
  public:
    explicit _RoomTriggerThread(HSQOBJECT thread_obj) : _thread_obj(thread_obj) {}
    virtual ~_RoomTriggerThread() = default;

    HSQUIRRELVM getThread() override { return _thread_obj._unVal.pThread; }

  private:
    HSQOBJECT _thread_obj;
};

class _RoomTrigger : public Trigger
{
  public:
    _RoomTrigger(Engine &engine, Object &object, HSQOBJECT inside, HSQOBJECT outside);
    ~_RoomTrigger() override;

    HSQOBJECT &getInside() { return _inside; }
    HSQOBJECT &getOutside() { return _outside; }

    std::string getName() override;

  private:
    void trigCore() override;
    void callTrigger(std::vector<HSQOBJECT> &params, const std::string &name);

  private:
    Engine &_engine;
    HSQUIRRELVM _vm{};
    Object &_object;
    HSQOBJECT _inside{};
    HSQOBJECT _outside{};
    HSQOBJECT thread_obj{};
    bool _isInside{false};
    SQInteger _insideParamsCount{0};
    SQInteger _outsideParamsCount{0};
    std::string _insideName;
    std::string _outsideName;
    std::string _name;
};
} // namespace ng
