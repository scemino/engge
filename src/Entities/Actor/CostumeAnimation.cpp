#include <utility>

#include "Entities/Actor/CostumeAnimation.hpp"

namespace ng
{
CostumeAnimation::CostumeAnimation(std::string name)
    : _name(std::move(name)), _state(AnimationState::Pause)
{
}

CostumeAnimation::~CostumeAnimation() = default;

void CostumeAnimation::play(bool loop)
{
    _loop = loop;
    _state = AnimationState::Play;
}

void CostumeAnimation::update(const sf::Time &elapsed)
{
    if (!isPlaying())
        return;
    bool loop = _loop;
    for (auto &layer : _layers)
    {
        bool end = layer->update(elapsed);
        if(_loop && end)
        {
            layer->reset();
        }
        loop |= !end;
    }
    if (!loop)
    {
        pause();
    }
}

void CostumeAnimation::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    for (auto &layer : _layers)
    {
        target.draw(*layer, states);
    }
}

void CostumeAnimation::setFps(int fps)
{
    for (auto &layer : _layers)
    {
        layer->setFps(fps);
    }
}

bool CostumeAnimation::contains(const sf::Vector2f& pos) const
{
    for (auto &layer : _layers)
    {
        if(layer->contains(pos))
            return true;
    }
    return false;
}

} // namespace ng
