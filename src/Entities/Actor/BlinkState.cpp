#include "Entities/Actor/BlinkState.h"
#include "../../System/_Util.h"

namespace ng
{
BlinkState::BlinkState(Costume &costume) : _costume(costume)
{
}

void BlinkState::setRate(double min, double max)
{
    _min = min;
    _max = max;
    if (min == 0 && max == 0)
    {
        // blinking is disabled
        _state = -1;
    }
    else
    {
        _state = 0;
        _value = sf::seconds(float_rand(_min, _max));
    }
    _elapsed = sf::seconds(0);
    _costume.setLayerVisible("blink", false);
}

void BlinkState::update(sf::Time elapsed)
{
    if (_state == ObjectStateConstants::CLOSED)
    {
        // wait to blink
        _elapsed += elapsed;
        if (_elapsed > _value)
        {
            _state = 1;
            _costume.setLayerVisible("blink", true);
            _elapsed = sf::seconds(0);
        }
    }
    else if (_state == ObjectStateConstants::OPEN)
    {
        // wait time the eyes are closed
        _elapsed += elapsed;
        if (_elapsed > sf::seconds(0.2))
        {
            _costume.setLayerVisible("blink", false);
            _value = sf::seconds(float_rand(_min, _max));
            _elapsed = sf::seconds(0);
            _state = 0;
        }
    }
}
}
