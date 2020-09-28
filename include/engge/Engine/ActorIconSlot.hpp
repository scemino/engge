#pragma once
#include "engge/Entities/Actor/Actor.hpp"

namespace ng {
struct ActorIconSlot {
  bool selectable;
  Actor *pActor;

  ActorIconSlot() {
    selectable = false;
    pActor = nullptr;
  }
};
} // namespace ng
