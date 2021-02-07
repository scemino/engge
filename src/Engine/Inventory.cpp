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
  _gameSheet.setTextureManager(pTextureManager);
  _gameSheet.load("GameSheet");

  _inventoryItems.setTextureManager(pTextureManager);
  _inventoryItems.load("InventoryItems");

  // compute all inventory rects
  auto scrollUpFrameRect = _gameSheet.getRect("scroll_up");
  auto inventoryRect = _gameSheet.getRect("inventory_background");

  _scrollUpRect = ngf::frect::fromPositionSize({Screen::Width - 627.f, Screen::Height - 167.f},
                                               {static_cast<float>(scrollUpFrameRect.getWidth()),
                                                static_cast<float>(scrollUpFrameRect.getHeight())});
  _scrollDownRect = ngf::frect::fromPositionSize({Screen::Width - 627.f, Screen::Height - 73.f},
                                                 {static_cast<float>(scrollUpFrameRect.getWidth()),
                                                  static_cast<float>(scrollUpFrameRect.getHeight())});

  glm::vec2 sizeBack = {static_cast<float>(inventoryRect.getWidth()), static_cast<float>(inventoryRect.getHeight())};
  glm::vec2 scrollUpSize(scrollUpFrameRect.getWidth(), scrollUpFrameRect.getHeight());
  glm::vec2 scrollUpMargin(4, 7);

  auto startX = sizeBack.x / 2.f + _scrollUpRect.getTopLeft().x + scrollUpSize.x + scrollUpMargin.x;
  auto startY = sizeBack.y / 2.f + _scrollUpRect.getTopLeft().y;

  auto x = 0, y = 0;
  glm::vec2 gap = {7.f, 7.f};

  for (size_t i = 0; i < 8; i++) {
    _inventoryRects[i] = ngf::frect::fromPositionSize({x + startX, y + startY}, {sizeBack.x, sizeBack.y});
    if ((i % 4) == 3) {
      x = 0;
      y += sizeBack.y + gap.y;
    } else {
      x += sizeBack.x + gap.x;
    }
  }
}

bool Inventory::update(const ngf::TimeSpan &elapsed) {
  _jiggleTime += 20.f * elapsed.getTotalSeconds();
  _pCurrentInventoryObject = nullptr;

  if (_pCurrentActor == nullptr)
    return false;

  auto inventoryOffset = _pCurrentActor->getInventoryOffset();
  for (size_t i = 0; i < _inventoryRects.size(); i++) {
    auto r = _inventoryRects.at(i);
    r.min.x -= r.getWidth() / 2.f;
    r.min.y -= r.getHeight() / 2.f;
    r.max.x -= r.getWidth() / 2.f;
    r.max.y -= r.getHeight() / 2.f;
    if (r.contains(_mousePos)) {
      auto &objects = _pCurrentActor->getObjects();
      if ((inventoryOffset * 4 + i) < objects.size()) {
        _pCurrentInventoryObject = objects[inventoryOffset * 4 + i];
        return false;
      }
    }
  }

  auto mouseDown = ngf::Mouse::isButtonPressed(ngf::Mouse::Button::Left);
  if (!_mouseWasDown || mouseDown) {
    _mouseWasDown = mouseDown;
    return false;
  }

  _mouseWasDown = mouseDown;

  if (hasUpArrow()) {
    if (_scrollUpRect.contains(_mousePos)) {
      _pCurrentActor->setInventoryOffset(inventoryOffset - 1);
      return true;
    }
  }

  if (hasDownArrow()) {
    if (_scrollDownRect.contains(_mousePos)) {
      _pCurrentActor->setInventoryOffset(inventoryOffset + 1);
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
  auto rect = _gameSheet.getRect(isRetro ? "scroll_up_retro" : "scroll_up");

  auto color = _pColors->verbNormal;
  color.a = _alpha;

  glm::vec2 scrollUpSize(rect.getWidth(), rect.getHeight());
  ngf::RectangleShape scrollUpShape;
  scrollUpShape.setColor(color);
  scrollUpShape.getTransform().setPosition(_scrollUpRect.getTopLeft());
  scrollUpShape.setSize(scrollUpSize);
  scrollUpShape.setTexture(*_gameSheet.getTexture(), false);
  scrollUpShape.setTextureRect(_gameSheet.getTexture()->computeTextureCoords(rect));
  scrollUpShape.draw(target, {});
}

void Inventory::drawDownArrow(ngf::RenderTarget &target) const {
  if (!hasDownArrow())
    return;

  const auto &preferences = Locator<Preferences>::get();
  auto isRetro =
      preferences.getUserPreference(PreferenceNames::RetroVerbs, PreferenceDefaultValues::RetroVerbs);

  auto scrollDownFrameRect = _gameSheet.getRect(isRetro ? "scroll_down_retro" : "scroll_down");
  glm::vec2 scrollDownSize(scrollDownFrameRect.getWidth(), scrollDownFrameRect.getHeight());

  auto color = _pColors->verbNormal;
  color.a = _alpha;

  ngf::RectangleShape scrollDownShape;
  scrollDownShape.setColor(color);
  scrollDownShape.getTransform().setPosition(_scrollDownRect.getTopLeft());
  scrollDownShape.setSize(scrollDownSize);
  scrollDownShape.setTexture(*_gameSheet.getTexture(), false);
  scrollDownShape.setTextureRect(_gameSheet.getTexture()->computeTextureCoords(scrollDownFrameRect));
  scrollDownShape.draw(target, {});
}

bool Inventory::hasUpArrow() const {
  auto inventoryOffset = _pCurrentActor->getInventoryOffset();
  return inventoryOffset != 0;
}

bool Inventory::hasDownArrow() const {
  const auto &objects = _pCurrentActor->getObjects();
  auto inventoryOffset = _pCurrentActor->getInventoryOffset();
  return static_cast<int>(objects.size()) > (inventoryOffset * 4 + 8);
}

void Inventory::draw(ngf::RenderTarget &target, ngf::RenderStates) const {
  if (_currentActorIndex == -1)
    return;

  const auto view = target.getView();
  target.setView(ngf::View(ngf::frect::fromPositionSize({0, 0}, {Screen::Width, Screen::Height})));

  auto color = ngf::Colors::White;
  color.a = _alpha;

  // draw inventory items
  ngf::Color c(_pColors->inventoryBackground);
  c.a = _alpha * 0.5f;

  auto inventoryRect = _gameSheet.getRect("inventory_background");
  ngf::Sprite inventoryShape;
  inventoryShape.setColor(c);
  inventoryShape.setTexture(*_gameSheet.getTexture());
  inventoryShape.setTextureRect(inventoryRect);
  for (auto i = 0; i < 8; i++) {
    inventoryShape.getTransform().setPosition(_inventoryRects[i].getTopLeft());
    inventoryShape.getTransform().setOrigin({_inventoryRects[i].getWidth() / 2.f,
                                             _inventoryRects[i].getHeight() / 2.f});
    inventoryShape.draw(target);
  }

  // draw inventory objects
  if (!_pCurrentActor) {
    target.setView(view);
    return;
  }

  auto &objects = _pCurrentActor->getObjects();
  drawUpArrow(target);
  drawDownArrow(target);

  auto inventoryOffset = _pCurrentActor->getInventoryOffset();
  auto count = std::min((size_t) 8, objects.size() - inventoryOffset * 4);
  for (size_t i = 0; i < count; i++) {
    auto &object = objects.at(inventoryOffset * 4 + i);
    auto icon = object->getIcon();
    auto rect = _inventoryItems.getRect(icon);
    auto spriteSourceSize = _inventoryItems.getSpriteSourceSize(icon);
    auto sourceSize = _inventoryItems.getSourceSize(icon);
    glm::vec2 origin
        (sourceSize.x / 2.f - spriteSourceSize.getTopLeft().x, sourceSize.y / 2.f - spriteSourceSize.getTopLeft().y);

    ngf::Sprite sprite;
    sprite.getTransform().setOrigin(origin);
    if (object->getJiggle()) {
      sprite.getTransform().setRotation(3.f * sinf(_jiggleTime));
    }
    sprite.getTransform().setPosition(_inventoryRects[i].getTopLeft());
    sprite.setTexture(*_inventoryItems.getTexture());
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
  if (!_pCurrentActor)
    return glm::vec2();
  auto inventoryOffset = _pCurrentActor->getInventoryOffset() * 4;
  const auto &objects = _pCurrentActor->getObjects();
  auto it = std::find(objects.cbegin(), objects.cend(), pObject);
  auto index = std::distance(objects.cbegin(), it);
  if (index >= inventoryOffset && index < (inventoryOffset + 8)) {
    const auto &rect = _inventoryRects.at(index - inventoryOffset);
    return rect.getTopLeft() + rect.getSize() / 2.f;
  }
  return glm::vec2();
}

} // namespace ng
