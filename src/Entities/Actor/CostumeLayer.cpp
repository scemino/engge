#include "Entities/Actor/CostumeLayer.hpp"
#include "Entities/Actor/Actor.hpp"
#include "Room/Room.hpp"
#include "SFML/Graphics.hpp"

namespace ng
{
CostumeLayer::CostumeLayer(Animation&& animation)
: _animation(std::move(animation))
{
}

bool CostumeLayer::update(const sf::Time &elapsed)
{
    _animation.update(elapsed);
    return _animation.isPlaying();
}

void CostumeLayer::reset()
{
    _animation.reset();
}

void CostumeLayer::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    if (!getVisible())
        return;

    Animation anim(_animation);
    anim.setLeftDirection(_leftDirection);
    anim.setColor(_pActor->getRoom()->getAmbientLight());
    target.draw(anim, states);
}

} // namespace ng
