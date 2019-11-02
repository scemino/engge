#include "Light.h"
#include "Locator.h"
#include "ResourceManager.h"

namespace ng
{
Light::Light(sf::Color color, sf::Vector2i pos)
    : _color(color), _pos(pos)
{
    sq_resetobject(&_table);
    _id = Locator::getResourceManager().getLightId();
}

Light::~Light() = default;

} // namespace ng
