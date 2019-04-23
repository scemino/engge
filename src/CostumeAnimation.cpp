#include <utility>

#include "CostumeAnimation.h"

namespace ng
{
CostumeAnimation::CostumeAnimation(std::string name)
    : _name(std::move(name)), _state(AnimationState::Pause), _loop(false)
{
}

CostumeAnimation::~CostumeAnimation() = default;

void CostumeAnimation::play(bool loop)
{
    _loop = loop;
    for (auto &layer : _layers)
    {
        layer->setLoop(loop);
    }
    _state = AnimationState::Play;
}

void CostumeAnimation::update(const sf::Time &elapsed)
{
    if (!isPlaying())
        return;
    bool isFinished = !_loop;
    for (auto &layer : _layers)
    {
        isFinished &= layer->update(elapsed);
    }
    if (isFinished)
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
} // namespace ng
