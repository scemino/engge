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

void RoomLayer::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    // draw layer sprites
    for (const auto &sprite : getSprites())
    {
        target.draw(sprite, states);
    }

    // draw layer objects
    for (const auto &entity : _entities)
    {
        target.draw(entity, states);
    }
}

void RoomLayer::update(const sf::Time &elapsed)
{
     std::sort(std::begin(_entities), std::end(_entities), [](const NGEntity &a, const NGEntity &b) {
        return a.getZOrder() > b.getZOrder();
    });
    std::for_each(std::begin(_entities), std::end(_entities), [elapsed](NGEntity &obj) { obj.update(elapsed); });
}

} // namespace ng
