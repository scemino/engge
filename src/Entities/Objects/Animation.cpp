#include <iostream>
#include <utility>
#include "Entities/Objects/AnimationFrame.hpp"
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

size_t Animation::getSize() const noexcept { return _frames.size(); }

bool Animation::empty() const noexcept { return _frames.size() == 0; }

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
    reset();
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
        _index = (_index + 1) % _frames.size();
        _frames.at(_index).call();
    }
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
    target.draw(sprite, states);
}
} // namespace ng
