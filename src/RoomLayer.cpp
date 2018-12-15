#include "RoomLayer.h"

namespace ng
{
RoomLayer::RoomLayer()
    : _zsort(0), _parallax(1, 1)
{
}

void RoomLayer::addEntity(NGEntity &entity)
{
    _entities.push_back(entity);
}

void RoomLayer::removeEntity(NGEntity &entity)
{
    auto it = std::find_if(std::cbegin(_entities), std::cend(_entities), [&entity](const std::reference_wrapper<NGEntity> &ref) {
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
     std::sort(std::begin(_entities), std::end(_entities), [](const NGEntity &a, const NGEntity &b) {
        return a.getZOrder() > b.getZOrder();
    });
    std::for_each(std::begin(_entities), std::end(_entities), [elapsed](NGEntity &obj) { obj.update(elapsed); });
}

} // namespace ng
