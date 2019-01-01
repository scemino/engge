#include "CostumeLayer.h"
#include "Actor.h"

namespace ng
{
CostumeLayer::CostumeLayer()
    : _pTexture(nullptr),
      _fps(10),
      _flags(0),
      _index(0),
      _isVisible(true),
      _pActor(nullptr),
      _loop(false),
      _leftDirection(false)
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
            if (!_loop)
            {
                _index--;
                return true;
            }
            _index = 0;
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

void CostumeLayer::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    if (!getVisible())
        return;

    auto frame = getIndex();
    auto rect = _frames[frame];
    auto sourceRect = (sf::FloatRect)_sourceFrames[frame];
    auto size = (sf::Vector2f)_sizes[frame];
    sf::Vector2i offset;
    if (!_offsets.empty())
    {
        offset = _offsets[frame];
    }
    float x;
    if (_leftDirection)
    {
        rect.left += rect.width;
        rect.width = -rect.width;
        x = size.x - sourceRect.left - size.x / 2.f - sourceRect.width;
    }
    else
    {
        x = sourceRect.left - size.x / 2.f;
    }
    sf::Sprite sprite(*_pTexture, rect);
    auto y = sourceRect.top - size.y / 2.f;
    sprite.setOrigin(-sf::Vector2f(x, y) + (sf::Vector2f)offset);
    target.draw(sprite, states);
}

} // namespace ng
