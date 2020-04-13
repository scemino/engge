#include <algorithm>
#include "Engine/Engine.hpp"
#include "Engine/Inventory.hpp"
#include "Entities/Objects/Object.hpp"
#include "Engine/Preferences.hpp"
#include "Room/Room.hpp"
#include "Graphics/Screen.hpp"
#include "System/Locator.hpp"

namespace ng {

void Inventory::setTextureManager(TextureManager *pTextureManager) {
  _gameSheet.setTextureManager(pTextureManager);
  _gameSheet.load("GameSheet");

  _inventoryItems.setTextureManager(pTextureManager);
  _inventoryItems.load("InventoryItems");

  // compute all inventory rects
  auto scrollUpFrameRect = _gameSheet.getRect("scroll_up");
  auto inventoryRect = _gameSheet.getRect("inventory_background");

  _scrollUpRect = {Screen::Width - 627.f, Screen::Height - 167.f, static_cast<float>(scrollUpFrameRect.width), static_cast<float>(scrollUpFrameRect.height)};
  _scrollDownRect = {Screen::Width - 627.f, Screen::Height - 73.f, static_cast<float>(scrollUpFrameRect.width), static_cast<float>(scrollUpFrameRect.height)};

  sf::Vector2f sizeBack = {static_cast<float>(inventoryRect.width), static_cast<float>(inventoryRect.height)};
  sf::Vector2f scrollUpSize(scrollUpFrameRect.width, scrollUpFrameRect.height);
  sf::Vector2f scrollUpMargin(4, 7);

  auto startX = sizeBack.x / 2.f + _scrollUpRect.left + scrollUpSize.x + scrollUpMargin.x;
  auto startY = sizeBack.y / 2.f + _scrollUpRect.top;

  auto x = 0, y = 0;
  sf::Vector2f gap = {7.f, 7.f};

  for (size_t i = 0; i < 8; i++) {
    _inventoryRects[i] = {x + startX, y + startY, sizeBack.x, sizeBack.y};
    if ((i % 4) == 3) {
      x = 0;
      y += sizeBack.y + gap.y;
    } else {
      x += sizeBack.x + gap.x;
    }
  }
}

bool Inventory::update(const sf::Time &elapsed) {
  _jiggleTime += 20.f * elapsed.asSeconds();
  _pCurrentInventoryObject = nullptr;

  if (_pCurrentActor == nullptr)
    return false;

  auto inventoryOffset = _pCurrentActor->getInventoryOffset();
  for (size_t i = 0; i < _inventoryRects.size(); i++) {
    auto r = _inventoryRects.at(i);
    r.left -= r.width/2.f;
    r.top -= r.height/2.f;
    if (r.contains(_mousePos)) {
      auto &objects = _pCurrentActor->getObjects();
      if ((inventoryOffset * 4 + i) < objects.size()) {
        _pCurrentInventoryObject = objects[inventoryOffset * 4 + i];
        return false;
      }
    }
  }

  auto mouseDown = sf::Mouse::isButtonPressed(sf::Mouse::Left);
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

void Inventory::drawUpArrow(sf::RenderTarget &target) const {
  if (!hasUpArrow())
    return;

  const auto &preferences = Locator<Preferences>::get();
  auto isRetro =
      preferences.getUserPreference(PreferenceNames::RetroVerbs, PreferenceDefaultValues::RetroVerbs);
  auto rect = _gameSheet.getRect(isRetro ? "scroll_up_retro" : "scroll_up");

  auto color = _pColors->verbNormal;
  color.a = static_cast<sf::Uint8>(255.f * _alpha);

  sf::Vector2f scrollUpSize(rect.width, rect.height);
  sf::RectangleShape scrollUpShape;
  scrollUpShape.setFillColor(color);
  scrollUpShape.setPosition(_scrollUpRect.left, _scrollUpRect.top);
  scrollUpShape.setSize(scrollUpSize);
  scrollUpShape.setTexture(&_gameSheet.getTexture());
  scrollUpShape.setTextureRect(rect);
  target.draw(scrollUpShape);
}

void Inventory::drawDownArrow(sf::RenderTarget &target) const {
  if (!hasDownArrow())
    return;

  const auto &preferences = Locator<Preferences>::get();
  auto isRetro =
      preferences.getUserPreference(PreferenceNames::RetroVerbs, PreferenceDefaultValues::RetroVerbs);

  auto scrollDownFrameRect = _gameSheet.getRect(isRetro ? "scroll_down_retro" : "scroll_down");
  sf::Vector2f scrollDownSize(scrollDownFrameRect.width, scrollDownFrameRect.height);

  auto color = _pColors->verbNormal;
  color.a = static_cast<sf::Uint8>(255.f * _alpha);

  sf::RectangleShape scrollDownShape;
  scrollDownShape.setFillColor(color);
  scrollDownShape.setPosition(_scrollDownRect.left, _scrollDownRect.top);
  scrollDownShape.setSize(scrollDownSize);
  scrollDownShape.setTexture(&_gameSheet.getTexture());
  scrollDownShape.setTextureRect(scrollDownFrameRect);
  target.draw(scrollDownShape);
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

void Inventory::draw(sf::RenderTarget &target, sf::RenderStates) const {
  if (_currentActorIndex == -1)
    return;

  const auto view = target.getView();
  target.setView(sf::View(sf::FloatRect(0, 0, Screen::Width, Screen::Height)));

  auto color = sf::Color::White;
  color.a = static_cast<sf::Uint8>(_alpha * 255.f);

  // draw inventory items
  sf::Color c(_pColors->inventoryBackground);
  c.a = static_cast<sf::Uint8>(_alpha * 128.f);

  auto inventoryRect = _gameSheet.getRect("inventory_background");
  sf::Sprite inventoryShape;
  inventoryShape.setColor(c);
  inventoryShape.setTexture(_gameSheet.getTexture());
  inventoryShape.setTextureRect(inventoryRect);
  for (auto i = 0; i < 8; i++) {
    inventoryShape.setPosition(_inventoryRects[i].left, _inventoryRects[i].top);
    inventoryShape.setOrigin(_inventoryRects[i].width/2.f, _inventoryRects[i].height/2.f);
    target.draw(inventoryShape);
  }

  // draw inventory objects
  if (!_pCurrentActor)
    return;

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
    sf::Vector2f origin(sourceSize.x / 2.f - spriteSourceSize.left, sourceSize.y / 2.f - spriteSourceSize.top);

    sf::Sprite sprite;
    sprite.setOrigin(origin);
    if (object->getJiggle()) {
      sprite.setRotation(3.f * sinf(_jiggleTime));
    }
    sprite.setPosition(_inventoryRects[i].left, _inventoryRects[i].top);
    sprite.setTexture(_inventoryItems.getTexture());
    sprite.setTextureRect(rect);
    sprite.scale(4, 4);
    sprite.setColor(color);
    target.draw(sprite);
  }
  target.setView(view);
}

sf::Vector2f Inventory::getPosition(Object *pObject) const {
  if(!_pCurrentActor) return sf::Vector2f();
  auto inventoryOffset = _pCurrentActor->getInventoryOffset() * 4;
  const auto &objects = _pCurrentActor->getObjects();
  auto it = std::find(objects.cbegin(), objects.cend(), pObject);
  auto index = std::distance(objects.cbegin(), it);
  if( index >= inventoryOffset && index < (inventoryOffset+8)){
    const auto& rect = _inventoryRects.at(index - inventoryOffset);
    return sf::Vector2f(rect.left, rect.top);
  }
  return sf::Vector2f();
}

} // namespace ng
