#include <ngf/Graphics/Sprite.h>
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

void RoomLayer::draw(ngf::RenderTarget &target, ngf::RenderStates states) const {
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
    ngf::Sprite s(*rm.getTexture(background.m_texture), background.m_rect);
    s.getTransform().setPosition(background.m_pos);
    s.draw(target, states);
  }

  // draw layer objects
  for (const Entity &entity : entities) {
    if (entity.hasParent())
      continue;
    entity.draw(target, states);
  }
}

void RoomLayer::drawForeground(ngf::RenderTarget &target, ngf::RenderStates states) const {
  std::for_each(_entities.begin(), _entities.end(),
                [&target, &states](const Entity &entity) { entity.drawForeground(target, states); });
}

void RoomLayer::update(const ngf::TimeSpan &elapsed) {
  std::for_each(std::begin(_entities), std::end(_entities), [elapsed](Entity &obj) { obj.update(elapsed); });
}

} // namespace ng
