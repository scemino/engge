#include <engge/Engine/Trigger.hpp>

namespace ng {
Trigger::Trigger() = default;

Trigger::~Trigger() = default;
void Trigger::trig() {
  if (!isEnabled())
    return;
  trigCore();
}

bool Trigger::isEnabled() const { return m_isEnabled; }

void Trigger::disable() { m_isEnabled = false; }

} // namespace ng
