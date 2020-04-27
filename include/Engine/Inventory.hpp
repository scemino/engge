#pragma once
#include <array>
#include "SFML/Graphics.hpp"
#include "Graphics/Screen.hpp"
#include "Graphics/SpriteSheet.hpp"

namespace ng {
class Object;

class Inventory : public sf::Drawable {
public:
  void setTextureManager(TextureManager *pTextureManager);
  bool update(const sf::Time &elapsed);

  void setCurrentActorIndex(int index) { _currentActorIndex = index; }
  void setCurrentActor(Actor *pActor) { _pCurrentActor = pActor; }
  void setMousePosition(const sf::Vector2f &pos) { _mousePos = pos; }
  Object *getCurrentInventoryObject() { return _pCurrentInventoryObject; }
  sf::Vector2f getPosition(Object *pObject) const;

  void setVerbUiColors(const VerbUiColors *pColors) { _pColors = pColors; }
  void setAlpha(float alpha) {_alpha = alpha;}
  float getAlpha() const {return _alpha;}

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
  void drawUpArrow(sf::RenderTarget &target) const;
  void drawDownArrow(sf::RenderTarget &target) const;
  bool hasUpArrow() const;
  bool hasDownArrow() const;

private:
  SpriteSheet _gameSheet, _inventoryItems;
  std::array<sf::FloatRect, 8> _inventoryRects;
  sf::FloatRect _scrollUpRect;
  sf::FloatRect _scrollDownRect;
  Object *_pCurrentInventoryObject{nullptr};
  sf::Vector2f _mousePos;
  float _jiggleTime{0};
  Actor* _pCurrentActor{nullptr};
  int _currentActorIndex{0};
  const VerbUiColors *_pColors{nullptr};
  float _alpha{1.f};
  bool _mouseWasDown{false};
};
} // namespace ng
