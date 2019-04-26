#include <utility>

#include <iostream>
#include "Animation.h"
#include "Object.h"

namespace ng
{
Animation::Animation(const sf::Texture &texture, std::string name)
    : _sprite(texture), _name(std::move(name)), _fps(10), _index(0), _state(AnimState::Pause)
{
}

Animation::~Animation() = default;

void Animation::reset()
{
    _index = 0;
    if (_rects.empty())
        return;
    auto &sourceRect = _sourceRects.at(_index);
    auto size = _sizes.at(_index);
    _sprite.setTextureRect(_rects.at(_index));
    _sprite.setOrigin(sf::Vector2f(size.x/ 2.f - sourceRect.left, size.y / 2.f - sourceRect.top));
}

void Animation::play(bool loop)
{
    _loop = loop;
    _state = AnimState::Play;
    reset();
}

void Animation::update(const sf::Time &elapsed)
{
    if (_state == AnimState::Pause)
        return;

    if (_rects.empty())
        return;

    _time += elapsed;
    if (_time.asSeconds() > (1.f / _fps))
    {
        _time = sf::seconds(0);
        _index = (_index + 1) % _rects.size();

        auto sourceRect = _sourceRects.at(_index);
        auto size = _sizes.at(_index);
        _sprite.setTextureRect(_rects.at(_index));
        _sprite.setOrigin(sf::Vector2f(size.x / 2.f - sourceRect.left, size.y / 2.f - sourceRect.top));

        updateTrigger();
    }
}

void Animation::updateTrigger()
{
    if (_triggers.empty())
        return;

    auto trigger = _triggers.at(_index);
    if (trigger.has_value() && _pObject)
    {
        _pObject->trig(*trigger);
    }
}

void Animation::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    if (_rects.empty())
        return;
    target.draw(_sprite, states);
}
} // namespace ng
