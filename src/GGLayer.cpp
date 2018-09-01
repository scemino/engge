#include "GGLayer.h"

namespace gg
{
GGLayer::GGLayer()
    : _index(0)
{
}

GGLayer::~GGLayer()
{
}

void GGLayer::update(const sf::Time &elapsed)
{
    _time += elapsed;
    if (_time.asSeconds() > (1.f / _fps))
    {
        _time = sf::seconds(0);
        _index = (_index + 1) % _frames.size();
    }
}
} // namespace gg
