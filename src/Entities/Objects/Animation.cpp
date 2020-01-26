#include <iostream>
#include <utility>
#include "Entities/Objects/Animation.hpp"
#include "Entities/Objects/Object.hpp"

namespace ng
{

Animation::Animation() = default;

Animation::Animation(const sf::Texture &texture, std::string name)
    : _pTexture(&texture), _name(std::move(name))
{
}

Animation::~Animation() = default;

size_t Animation::size() const noexcept { return _frames.size(); }

bool Animation::empty() const noexcept { return _frames.empty(); }

void Animation::addFrame(AnimationFrame&& frame)
{
    _frames.push_back(std::move(frame));
}

void Animation::reset()
{
    if (_frames.empty())
        return;
    _index = _frames.size() - 1;
}

void Animation::play(bool loop)
{
    _loop = loop;
    _state = AnimState::Play;
    _index = 0;
}

void Animation::update(const sf::Time &elapsed)
{
    if (_state == AnimState::Pause)
        return;

    if (_frames.empty())
        return;

    _time += elapsed;
    if (_time.asSeconds() > (1.f / _fps))
    {
        _time = sf::seconds(0);
        if(_loop || _index != _frames.size()-1)
        {
            _index = (_index + 1) % _frames.size();
            _frames.at(_index).call();
        }
    }
}

AnimationFrame& Animation::at(size_t index)
{
    return _frames.at(index);
}

void Animation::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    if (_frames.empty())
        return;

    const auto &frame = _frames.at(_index);
    sf::Sprite sprite;
    sprite.setColor(_color);
    sprite.setTexture(*_pTexture);
    sprite.setTextureRect(frame.getRect());
    sprite.setOrigin(frame.getOrigin());
    sprite.move(-frame.getOffset());
    target.draw(sprite, states);
}

bool Animation::contains(const sf::Vector2f &pos) const
{
    if (_frames.empty())
        return false;

    const auto &frame = _frames.at(_index);

    sf::Transformable t;
    t.setOrigin(frame.getOrigin());
    t.move(-frame.getOffset());

    auto rect = frame.getRect();
    sf::FloatRect r1(0, 0, rect.width, rect.height);
    auto r = t.getTransform().transformRect(r1);
    return r.contains(pos);
}

} // namespace ng
