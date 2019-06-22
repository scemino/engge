#pragma once
#include "squirrel.h"

namespace ng
{
class ThreadBase
{
public:
    virtual ~ThreadBase() = default;

    virtual HSQUIRRELVM getThread() = 0;
};
}
