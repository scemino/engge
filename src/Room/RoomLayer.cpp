#include <ngf/Graphics/Sprite.h>
#include <engge/Graphics/LightingShader.h>
#include "engge/Room/RoomLayer.hpp"
#include "engge/Graphics/ResourceManager.hpp"
#include "engge/System/Locator.hpp"

namespace ng {
RoomLayer::RoomLayer() = default;

void RoomLayer::setTexture(const std::string &textureName) {
  m_textureName = textureName;
}

void RoomLayer::addEntity(Entity &entity) { m_entities.emplace_back(entity); }

void RoomLayer::removeEntity(Entity &entity) {
  m_entities.erase(std::remove_if(m_entities.begin(), m_entities.end(),
                                  [&entity](auto &pEntity) -> bool { return &pEntity.get() == &entity; }),
                   m_entities.end());
}

void RoomLayer::draw(ngf::RenderTarget &target, ngf::RenderStates states) const {
  if (!m_enabled)
    return;

  // disable lighting for layer rendering
  auto pShader = (LightingShader *) states.shader;
  auto count = pShader->getNumberLights();
  auto ambient = pShader->getAmbientColor();
  pShader->setAmbientColor(ngf::Colors::White);
  pShader->setNumberLights(0);

  // sort entities by z-order
  std::vector<std::reference_wrapper<Entity>> entities;
  std::copy(m_entities.begin(), m_entities.end(), std::back_inserter(entities));
  std::sort(entities.begin(), entities.end(),
            [](const Entity &a, const Entity &b) {
              if (a.getZOrder() == b.getZOrder())
                return a.getId() < b.getId();
              return a.getZOrder() > b.getZOrder();
            });

  float offsetX = 0.f;
  // draw layer sprites
  for (const auto &item : m_backgrounds) {
    auto texture = Locator<ResourceManager>::get().getTexture(m_textureName);
    auto texSize = texture->getSize();
    pShader->setTexture(*texture);
    pShader->setContentSize(item.sourceSize);
    pShader->setSpriteOffset({0, -item.frame.getHeight()});
    pShader->setSpritePosInSheet({static_cast<float>(item.frame.min.x) / texSize.x,
                                  static_cast<float>(item.frame.min.y) / texSize.y});
    pShader->setSpriteSizeRelToSheet({static_cast<float>(item.sourceSize.x) / texSize.x,
                                      static_cast<float>(item.sourceSize.y) / texSize.y});

    ngf::Sprite s(*texture, item.frame);
    glm::vec2 off{item.spriteSourceSize.min.x, item.spriteSourceSize.min.y + m_roomSizeY - item.sourceSize.y};
    s.getTransform().setPosition(off + glm::vec2{offsetX, m_offsetY});
    offsetX += item.frame.getWidth();
    s.draw(target, states);
  }

  // draw layer entities: actors and objects
  for (const Entity &entity : entities) {
    if (entity.hasParent())
      continue;

    // indicates whether or not the entity needs lighting
    pShader->setAmbientColor(entity.isLit() ? ambient : ngf::Colors::White);
    pShader->setNumberLights(entity.isLit() ? count : 0);
    entity.draw(target, states);
  }

  pShader->setAmbientColor(ambient);
  pShader->setNumberLights(count);
}

void RoomLayer::drawForeground(ngf::RenderTarget &target, ngf::RenderStates states) const {
  std::for_each(m_entities.begin(), m_entities.end(),
                [&target, &states](const Entity &entity) { entity.drawForeground(target, states); });
}

void RoomLayer::update(const ngf::TimeSpan &elapsed) {
  std::for_each(m_entities.begin(), m_entities.end(), [elapsed](Entity &obj) { obj.update(elapsed); });
}

} // namespace ng
