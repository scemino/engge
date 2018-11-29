#include "RoomLayer.h"

namespace gg
{
RoomLayer::RoomLayer()
    : _zsort(0), _parallax(1, 1)
{
}

void RoomLayer::addEntity(GGEntity &entity)
{
    _entities.push_back(entity);
}

void RoomLayer::removeEntity(GGEntity &entity)
{
    auto it = std::find_if(std::cbegin(_entities), std::cend(_entities), [&entity](const std::reference_wrapper<GGEntity> &ref) {
        return &ref.get() == &entity;
    });
    if (it == std::cend(_entities))
        return;
    _entities.erase(it);
}

void RoomLayer::draw(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const
{
    auto parallax = getParallax();
    auto posX = (Screen::HalfWidth - cameraPos.x) * parallax.x - Screen::HalfWidth;
    auto posY = (Screen::HalfHeight - cameraPos.y) * parallax.y - Screen::HalfHeight;

    sf::RenderStates states;
    sf::Transform t;
    t.translate(posX, posY);
    states.transform = t;

    // draw layer sprites
    for (const auto &sprite : getSprites())
    {
        window.draw(sprite, states);
    }

    // draw layer objects
    std::for_each(std::begin(_entities), std::end(_entities), [&window, &states](const sf::Drawable &entity) {
        window.draw(entity, states);
    });
}

void RoomLayer::update(const sf::Time &elapsed)
{
     std::sort(std::begin(_entities), std::end(_entities), [](const GGEntity &a, const GGEntity &b) {
        return a.getZOrder() > b.getZOrder();
    });
    std::for_each(std::begin(_entities), std::end(_entities), [elapsed](GGEntity &obj) { obj.update(elapsed); });
}

} // namespace gg
