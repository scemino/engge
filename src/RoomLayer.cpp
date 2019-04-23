#include "RoomLayer.h"

namespace ng
{
RoomLayer::RoomLayer() = default;

void RoomLayer::addEntity(Entity &entity)
{
    _entities.emplace_back(entity);
}

void RoomLayer::removeEntity(Entity &entity)
{
    auto it = std::find_if(std::cbegin(_entities), std::cend(_entities), [&entity](const std::reference_wrapper<Entity> &ref) {
        return &ref.get() == &entity;
    });
    if (it == std::cend(_entities))
        return;
    _entities.erase(it);
}

void RoomLayer::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    if(!_enabled) return;
    
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

void RoomLayer::drawForeground(sf::RenderTarget &target, sf::RenderStates states) const
{
    std::for_each(_entities.begin(), _entities.end(), [&target, &states](const Entity &entity) {
        entity.drawForeground(target, states);
    });
}

void RoomLayer::update(const sf::Time &elapsed)
{
    std::sort(std::begin(_entities), std::end(_entities), [](const Entity &a, const Entity &b) {
        return a.getZOrder() > b.getZOrder();
    });
    std::for_each(std::begin(_entities), std::end(_entities), [elapsed](Entity &obj) { obj.update(elapsed); });
}

} // namespace ng
