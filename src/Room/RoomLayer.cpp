#include "engge/Room/RoomLayer.hpp"
#include "engge/Graphics/ResourceManager.hpp"
#include "engge/System/Locator.hpp"

namespace ng {
RoomLayer::RoomLayer() = default;

void RoomLayer::addEntity(Entity &entity) { _entities.emplace_back(entity); }

void RoomLayer::removeEntity(Entity &entity) {
  _entities.erase(std::remove_if(_entities.begin(), _entities.end(),
                                 [&entity](auto &pEntity) -> bool { return &pEntity.get() == &entity; }),
                  _entities.end());
}

void RoomLayer::draw(sf::RenderTarget &target, sf::RenderStates states) const {
  if (!_enabled)
    return;

  std::vector<std::reference_wrapper<Entity>> entities;
  std::copy(_entities.begin(), _entities.end(), std::back_inserter(entities));
  std::sort(entities.begin(), entities.end(),
            [](const Entity &a, const Entity &b) {
              if (a.getZOrder() == b.getZOrder())
                return a.getId() < b.getId();
              return a.getZOrder() > b.getZOrder();
            });

  // draw layer sprites
  auto &rm = Locator<ResourceManager>::get();
  for (const auto &background : _backgrounds) {
    sf::Sprite s;
    s.move((sf::Vector2f) background.m_pos);
    s.setTexture(*rm.getTexture(background.m_texture));
    s.setTextureRect(background.m_rect);
    target.draw(s, states);
  }

  // draw layer objects
  for (const Entity &entity : entities) {
    if (entity.hasParent())
      continue;
    target.draw(entity, states);
  }
}

void RoomLayer::drawForeground(sf::RenderTarget &target, sf::RenderStates states) const {
  std::for_each(_entities.begin(), _entities.end(),
                [&target, &states](const Entity &entity) { entity.drawForeground(target, states); });
}

void RoomLayer::update(const sf::Time &elapsed) {
  std::for_each(std::begin(_entities), std::end(_entities), [elapsed](Entity &obj) { obj.update(elapsed); });
}

} // namespace ng
