#include "RoomLayer.h"

namespace gg
{
RoomLayer::RoomLayer()
    : _zsort(0), _parallax(1, 1)
{
}

void RoomLayer::draw(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const
{
    for (const auto &s : getSprites())
    {
        auto sprite(s);
        auto parallax = getParallax();
        auto posX = (Screen::HalfWidth - cameraPos.x) * parallax.x - Screen::HalfWidth;
        auto posY = (Screen::HalfHeight - cameraPos.y) * parallax.y - Screen::HalfHeight;
        sprite.move(posX, posY);
        window.draw(sprite);
    }
}
} // namespace gg
