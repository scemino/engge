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
    if(_loop == loop && _state == AnimationState::Play) return;
    _loop = loop;
    _state = AnimationState::Play;
    for (auto &&layer : _layers)
    {
        layer->play(loop);
    }
}

void CostumeAnimation::update(const sf::Time &elapsed)
{
    if (!isPlaying())
        return;
    auto loop = _loop;
    for (auto &&layer : _layers)
    {
        loop |= !layer->update(elapsed);
    }
    if (!loop)
    {
        pause();
    }
}

void CostumeAnimation::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    for (auto &&layer : _layers)
    {
        target.draw(*layer, states);
    }
}

bool CostumeAnimation::contains(const sf::Vector2f& pos) const
{
    for (auto &&layer : _layers)
    {
        if(layer->getAnimation().contains(pos)){
            return true;
        }
    }
    return false;
}

} // namespace ng
