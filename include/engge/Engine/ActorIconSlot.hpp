#pragma once

namespace ng {
class Actor;
struct ActorIconSlot {
  bool selectable{false};
  Actor *pActor{nullptr};
};
} // namespace ng
