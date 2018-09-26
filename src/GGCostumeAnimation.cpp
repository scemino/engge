#include "GGCostumeAnimation.h"

namespace gg
{
GGCostumeAnimation::GGCostumeAnimation(const std::string &name, sf::Texture &texture)
    : _texture(texture), _name(name), _state(AnimationState::Pause)
{
}

GGCostumeAnimation::~GGCostumeAnimation() = default;

void GGCostumeAnimation::update(const sf::Time &elapsed)
{
    if (!isPlaying())
        return;
    bool isFinished = true;
    for (auto &layer : _layers)
    {
        isFinished &= layer->update(elapsed);
    }
    if (isFinished)
    {
        pause();
    }
}

void GGCostumeAnimation::draw(sf::RenderWindow &window, const sf::RenderStates &states) const
{
    for (auto &layer : _layers)
    {
        auto frame = layer->getIndex();
        auto &rect = layer->getFrames()[frame];
        auto &sourceRect = layer->getSourceFrames()[frame];
        sf::Sprite sprite(_texture, rect);
        sprite.setOrigin(-sourceRect.left, -sourceRect.top);

        window.draw(sprite, states);
    }
}
} // namespace gg
