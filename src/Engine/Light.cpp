#include "engge/Engine/Light.hpp"
#include "engge/System/Locator.hpp"
#include "engge/Engine/EntityManager.hpp"

namespace ng {
Light::Light() {
  sq_resetobject(&table);
  m_id = Locator<EntityManager>::get().getLightId();
}

Light::~Light() = default;

} // namespace ng
