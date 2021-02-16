#include <algorithm>
#include <ngf/Graphics/RectangleShape.h>
#include <ngf/Graphics/Sprite.h>
#include <ngf/System/Mouse.h>
#include <engge/Engine/Engine.hpp>
#include <engge/Engine/Inventory.hpp>
#include <engge/Entities/Object.hpp>
#include <engge/Engine/Preferences.hpp>
#include <engge/Room/Room.hpp>
#include <engge/Graphics/Screen.hpp>
#include <engge/System/Locator.hpp>

namespace ng {

void Inventory::setTextureManager(ResourceManager *pTextureManager) {
  m_gameSheet.setTextureManager(pTextureManager);
  m_gameSheet.load("GameSheet");

  m_inventoryItems.setTextureManager(pTextureManager);
  m_inventoryItems.load("InventoryItems");

  // compute all inventory rects
  auto scrollUpFrameRect = m_gameSheet.getRect("scroll_up");
  auto inventoryRect = m_gameSheet.getRect("inventory_background");

  m_scrollUpRect = ngf::frect::fromPositionSize({Screen::Width - 627.f, Screen::Height - 167.f},
                                                {static_cast<float>(scrollUpFrameRect.getWidth()),
                                                static_cast<float>(scrollUpFrameRect.getHeight())});
  m_scrollDownRect = ngf::frect::fromPositionSize({Screen::Width - 627.f, Screen::Height - 73.f},
                                                  {static_cast<float>(scrollUpFrameRect.getWidth()),
                                                  static_cast<float>(scrollUpFrameRect.getHeight())});

  glm::vec2 sizeBack = {static_cast<float>(inventoryRect.getWidth()), static_cast<float>(inventoryRect.getHeight())};
  glm::vec2 scrollUpSize(scrollUpFrameRect.getWidth(), scrollUpFrameRect.getHeight());
  glm::vec2 scrollUpMargin(4, 7);

  auto startX = sizeBack.x / 2.f + m_scrollUpRect.getTopLeft().x + scrollUpSize.x + scrollUpMargin.x;
  auto startY = sizeBack.y / 2.f + m_scrollUpRect.getTopLeft().y;

  auto x = 0, y = 0;
  glm::vec2 gap = {7.f, 7.f};

  for (size_t i = 0; i < 8; i++) {
    m_inventoryRects[i] = ngf::frect::fromPositionSize({x + startX, y + startY}, {sizeBack.x, sizeBack.y});
    if ((i % 4) == 3) {
      x = 0;
      y += sizeBack.y + gap.y;
    } else {
      x += sizeBack.x + gap.x;
    }
  }
}

bool Inventory::update(const ngf::TimeSpan &elapsed) {
  m_jiggleTime += 20.f * elapsed.getTotalSeconds();
  m_pCurrentInventoryObject = nullptr;

  if (m_pCurrentActor == nullptr)
    return false;

  auto inventoryOffset = m_pCurrentActor->getInventoryOffset();
  for (size_t i = 0; i < m_inventoryRects.size(); i++) {
    auto r = m_inventoryRects.at(i);
    r.min.x -= r.getWidth() / 2.f;
    r.min.y -= r.getHeight() / 2.f;
    r.max.x -= r.getWidth() / 2.f;
    r.max.y -= r.getHeight() / 2.f;
    if (r.contains(m_mousePos)) {
      auto &objects = m_pCurrentActor->getObjects();
      if ((inventoryOffset * 4 + i) < objects.size()) {
        m_pCurrentInventoryObject = objects[inventoryOffset * 4 + i];
        return false;
      }
    }
  }

  auto mouseDown = ngf::Mouse::isButtonPressed(ngf::Mouse::Button::Left);
  if (!m_mouseWasDown || mouseDown) {
    m_mouseWasDown = mouseDown;
    return false;
  }

  m_mouseWasDown = mouseDown;

  if (hasUpArrow()) {
    if (m_scrollUpRect.contains(m_mousePos)) {
      m_pCurrentActor->setInventoryOffset(inventoryOffset - 1);
      return true;
    }
  }

  if (hasDownArrow()) {
    if (m_scrollDownRect.contains(m_mousePos)) {
      m_pCurrentActor->setInventoryOffset(inventoryOffset + 1);
      return true;
    }
  }
  return false;
}

void Inventory::drawUpArrow(ngf::RenderTarget &target) const {
  if (!hasUpArrow())
    return;

  const auto &preferences = Locator<Preferences>::get();
  auto isRetro =
      preferences.getUserPreference(PreferenceNames::RetroVerbs, PreferenceDefaultValues::RetroVerbs);
  auto rect = m_gameSheet.getRect(isRetro ? "scroll_up_retro" : "scroll_up");

  auto color = m_pColors->verbNormal;
  color.a = m_alpha;

  glm::vec2 scrollUpSize(rect.getWidth(), rect.getHeight());
  ngf::RectangleShape scrollUpShape;
  scrollUpShape.setColor(color);
  scrollUpShape.getTransform().setPosition(m_scrollUpRect.getTopLeft());
  scrollUpShape.setSize(scrollUpSize);
  scrollUpShape.setTexture(*m_gameSheet.getTexture(), false);
  scrollUpShape.setTextureRect(m_gameSheet.getTexture()->computeTextureCoords(rect));
  scrollUpShape.draw(target, {});
}

void Inventory::drawDownArrow(ngf::RenderTarget &target) const {
  if (!hasDownArrow())
    return;

  const auto &preferences = Locator<Preferences>::get();
  auto isRetro =
      preferences.getUserPreference(PreferenceNames::RetroVerbs, PreferenceDefaultValues::RetroVerbs);

  auto scrollDownFrameRect = m_gameSheet.getRect(isRetro ? "scroll_down_retro" : "scroll_down");
  glm::vec2 scrollDownSize(scrollDownFrameRect.getWidth(), scrollDownFrameRect.getHeight());

  auto color = m_pColors->verbNormal;
  color.a = m_alpha;

  ngf::RectangleShape scrollDownShape;
  scrollDownShape.setColor(color);
  scrollDownShape.getTransform().setPosition(m_scrollDownRect.getTopLeft());
  scrollDownShape.setSize(scrollDownSize);
  scrollDownShape.setTexture(*m_gameSheet.getTexture(), false);
  scrollDownShape.setTextureRect(m_gameSheet.getTexture()->computeTextureCoords(scrollDownFrameRect));
  scrollDownShape.draw(target, {});
}

bool Inventory::hasUpArrow() const {
  auto inventoryOffset = m_pCurrentActor->getInventoryOffset();
  return inventoryOffset != 0;
}

bool Inventory::hasDownArrow() const {
  const auto &objects = m_pCurrentActor->getObjects();
  auto inventoryOffset = m_pCurrentActor->getInventoryOffset();
  return static_cast<int>(objects.size()) > (inventoryOffset * 4 + 8);
}

void Inventory::draw(ngf::RenderTarget &target, ngf::RenderStates) const {
  if (m_currentActorIndex == -1)
    return;

  const auto view = target.getView();
  target.setView(ngf::View(ngf::frect::fromPositionSize({0, 0}, {Screen::Width, Screen::Height})));

  auto color = ngf::Colors::White;
  color.a = m_alpha;

  // draw inventory items
  ngf::Color c(m_pColors->inventoryBackground);
  c.a = m_alpha * 0.5f;

  auto inventoryRect = m_gameSheet.getRect("inventory_background");
  ngf::Sprite inventoryShape;
  inventoryShape.setColor(c);
  inventoryShape.setTexture(*m_gameSheet.getTexture());
  inventoryShape.setTextureRect(inventoryRect);
  for (auto i = 0; i < 8; i++) {
    inventoryShape.getTransform().setPosition(m_inventoryRects[i].getTopLeft());
    inventoryShape.getTransform().setOrigin({m_inventoryRects[i].getWidth() / 2.f,
                                             m_inventoryRects[i].getHeight() / 2.f});
    inventoryShape.draw(target);
  }

  // draw inventory objects
  if (!m_pCurrentActor) {
    target.setView(view);
    return;
  }

  auto &objects = m_pCurrentActor->getObjects();
  drawUpArrow(target);
  drawDownArrow(target);

  auto inventoryOffset = m_pCurrentActor->getInventoryOffset();
  auto count = std::min((size_t) 8, objects.size() - inventoryOffset * 4);
  for (size_t i = 0; i < count; i++) {
    auto &object = objects.at(inventoryOffset * 4 + i);
    auto icon = object->getIcon();
    auto rect = m_inventoryItems.getRect(icon);
    auto spriteSourceSize = m_inventoryItems.getSpriteSourceSize(icon);
    auto sourceSize = m_inventoryItems.getSourceSize(icon);
    glm::vec2 origin
        (sourceSize.x / 2.f - spriteSourceSize.getTopLeft().x, sourceSize.y / 2.f - spriteSourceSize.getTopLeft().y);

    ngf::Sprite sprite;
    sprite.getTransform().setOrigin(origin);
    if (object->getJiggle()) {
      sprite.getTransform().setRotation(3.f * sinf(m_jiggleTime));
    }
    sprite.getTransform().setPosition(m_inventoryRects[i].getTopLeft());
    sprite.setTexture(*m_inventoryItems.getTexture());
    sprite.setTextureRect(rect);
    if (object->getPop() > 0) {
      const auto pop = 4.25f + object->getPopScale() * 0.25f;
      sprite.getTransform().setScale({pop, pop});
    } else {
      sprite.getTransform().setScale({4, 4});
    }
    sprite.setColor(color);
    sprite.draw(target, {});
  }
  target.setView(view);
}

glm::vec2 Inventory::getPosition(Object *pObject) const {
  if (!m_pCurrentActor)
    return glm::vec2();
  auto inventoryOffset = m_pCurrentActor->getInventoryOffset() * 4;
  const auto &objects = m_pCurrentActor->getObjects();
  auto it = std::find(objects.cbegin(), objects.cend(), pObject);
  auto index = std::distance(objects.cbegin(), it);
  if (index >= inventoryOffset && index < (inventoryOffset + 8)) {
    const auto &rect = m_inventoryRects.at(index - inventoryOffset);
    return rect.getTopLeft();
  }
  return glm::vec2();
}

} // namespace ng
