#include "CostumeLayer.h"
#include "Actor.h"
#include "Room.h"
#include "SFML/Graphics.hpp"

namespace ng
{
CostumeLayer::CostumeLayer() = default;
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
        updateSoundTrigger();
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

void CostumeLayer::updateSoundTrigger()
{
    if (_soundTriggers.empty())
        return;

    auto trigger = _soundTriggers[_index];
    if (trigger.has_value() && _pActor)
    {
        _pActor->trigSound(*trigger);
    }
}

void CostumeLayer::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    if (!getVisible())
        return;

    auto frame = getIndex();
    if (_frames.empty())
        return;
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
        offset.x = -offset.x;
    }
    sf::Sprite sprite(*_pTexture, rect);
    auto y = sourceRect.top - size.y / 2.f;
    sprite.setOrigin(-sf::Vector2f(x, y) + (sf::Vector2f)offset);
    sprite.setColor(_pActor->getRoom()->getAmbientLight());
    target.draw(sprite, states);
}

bool CostumeLayer::contains(const sf::Vector2f &pos) const
{
    auto frame = getIndex();
    if (_frames.empty())
        return false;

    auto rect = (sf::FloatRect)_frames[frame];
    auto sourceRect = (sf::FloatRect)_sourceFrames[frame];
    auto size = (sf::Vector2f)_sizes[frame];

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
    auto y = sourceRect.top - size.y / 2.f;

    sf::Vector2i offset;
    if (!_offsets.empty())
    {
        offset = _offsets[frame];
    }

    sf::Transformable t;
    t.setOrigin(-sf::Vector2f(x, y) + (sf::Vector2f)offset);

    sf::FloatRect r1(0, 0, rect.width, rect.height);
    auto r = t.getTransform().transformRect(r1);
    return r.contains(pos);
}

} // namespace ng
