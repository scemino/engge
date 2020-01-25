#pragma once
#include <vector>
#include "squirrel.h"
#include "ThreadBase.hpp"

namespace ng
{
class Thread : public ThreadBase
{
public:
    Thread(bool isGlobal, HSQUIRRELVM v, HSQOBJECT thread_obj, HSQOBJECT env_obj, HSQOBJECT closureObj, std::vector<HSQOBJECT> args);
    ~Thread() override;

    [[nodiscard]] HSQUIRRELVM getThread() const override;
    [[nodiscard]] bool isGlobal() const override { return _isGlobal; }

    bool call();
    

private:
    HSQUIRRELVM _v;
    HSQOBJECT _thread_obj;
    HSQOBJECT _env_obj;
    HSQOBJECT _closureObj;
    std::vector<HSQOBJECT> _args;
    bool _isGlobal{false};
};
} // namespace ng
