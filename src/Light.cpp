#include "Light.h"

namespace ng
{
Light::Light(sf::Color color, sf::Vector2i pos)
    : _color(color), _pos(pos)
{
    sq_resetobject(&_table);
}

Light::~Light() = default;
} // namespace ng
