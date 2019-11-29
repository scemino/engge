#pragma once
#include <vector>
#include "squirrel.h"
#include "ThreadBase.h"

namespace ng
{
class Thread : public ThreadBase
{
public:
    Thread(HSQUIRRELVM v, HSQOBJECT thread_obj, HSQOBJECT env_obj, HSQOBJECT closureObj, const std::vector<HSQOBJECT> &args);
    ~Thread() override;

    HSQUIRRELVM getThread() const override;

    bool call();
    

private:
    HSQUIRRELVM _v;
    HSQOBJECT _thread_obj;
    HSQOBJECT _env_obj;
    HSQOBJECT _closureObj;
    std::vector<HSQOBJECT> _args;
};
} // namespace ng
