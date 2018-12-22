#include "Layer.h"

namespace ng
{
Layer::Layer()
    : _index(0),
     _isVisible(true)
{
}

Layer::~Layer() = default;

bool Layer::update(const sf::Time &elapsed)
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
} // namespace ng
