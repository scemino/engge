#pragma once
#include "Actor.h"

namespace ng
{
struct ActorIconSlot
{
    bool selectable;
    Actor *pActor;

    ActorIconSlot()
    {
        selectable = false;
        pActor = nullptr;
    }
};
} // namespace ng
