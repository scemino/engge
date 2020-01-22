#include "Room/RoomLayer.h"

namespace ng
{
RoomLayer::RoomLayer() = default;

void RoomLayer::addEntity(Entity &entity) { _entities.emplace_back(entity); }

void RoomLayer::removeEntity(Entity &entity)
{
    _entities.erase(std::remove_if(_entities.begin(), _entities.end(),
                                   [&entity](auto &pEntity) -> bool { return &pEntity.get() == &entity; }),
                    _entities.end());
}

void RoomLayer::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    if (!_enabled)
        return;

    std::vector<std::reference_wrapper<Entity>> entities;
    std::copy(_entities.begin(), _entities.end(), std::back_inserter(entities));
    std::sort(entities.begin(), entities.end(),
              [](const Entity &a, const Entity &b) { 
                  if(a.getZOrder() == b.getZOrder())
                    return a.getId() < b.getId();
                  return a.getZOrder() > b.getZOrder();
                });

    // draw layer sprites
    for (const auto &sprite : getSprites())
    {
        target.draw(sprite, states);
    }

    // draw layer objects
    for (const Entity &entity : entities)
    {
        target.draw(entity, states);
    }
}

void RoomLayer::drawForeground(sf::RenderTarget &target, sf::RenderStates states) const
{
    std::for_each(_entities.begin(), _entities.end(),
                  [&target, &states](const Entity &entity) { entity.drawForeground(target, states); });
}

void RoomLayer::update(const sf::Time &elapsed)
{
    std::for_each(std::begin(_entities), std::end(_entities), [elapsed](Entity &obj) { obj.update(elapsed); });
}

} // namespace ng
