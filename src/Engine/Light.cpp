#include "Engine/Light.h"
#include "System/Locator.h"
#include "Engine/ResourceManager.h"

namespace ng
{
Light::Light(sf::Color color, sf::Vector2i pos)
    : _color(color), _pos(pos)
{
    sq_resetobject(&_table);
    _id = Locator<ResourceManager>::get().getLightId();
}

Light::~Light() = default;

} // namespace ng
