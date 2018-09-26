#include "GGLayer.h"

namespace gg
{
GGLayer::GGLayer()
    : _index(0)
{
}

GGLayer::~GGLayer() = default;

bool GGLayer::update(const sf::Time &elapsed)
{
    _time += elapsed;
    if (_time.asSeconds() > (1.f / _fps))
    {
        _time = sf::seconds(0);
        _index = _index + 1;
        if (_index == _frames.size())
        {
            _index = 0;
            return true;
        }
    }
    return false;
}
} // namespace gg
