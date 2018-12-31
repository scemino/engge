#include "CostumeLayer.h"
#include "Actor.h"

namespace ng
{
CostumeLayer::CostumeLayer()
    : _index(0),
     _isVisible(true),
     _pActor(nullptr)
{
}

CostumeLayer::~CostumeLayer() = default;

bool CostumeLayer::update(const sf::Time &elapsed)
{
    _time += elapsed;
    if (_time.asSeconds() > (1.f / _fps))
    {
        _time = sf::seconds(0);
        _index = _index + 1;
        if (_index == _frames.size())
        {
            _index--;
            return true;
        }
        updateTrigger();
    }
    return false;
}

void CostumeLayer::updateTrigger()
{
    if (_triggers.empty())
        return;

    auto trigger = _triggers[_index];
    if (trigger.has_value() && _pActor)
    {
        _pActor->trig(*trigger);
    }
}

} // namespace ng
