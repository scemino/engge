#include "Engine/Light.hpp"
#include "System/Locator.hpp"
#include "Engine/ResourceManager.hpp"

namespace ng {
Light::Light(sf::Color color, sf::Vector2i pos)
    : _color(color), _pos(pos) {
  sq_resetobject(&_table);
  _id = Locator<ResourceManager>::get().getLightId();
}

Light::~Light() = default;

} // namespace ng
