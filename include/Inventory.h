#pragma once
#include <array>
#include "SFML/Graphics.hpp"
#include "ActorIconSlot.h"
#include "Verb.h"
#include "SpriteSheet.h"

namespace ng
{
class Engine;

class Inventory : public sf::Drawable
{
  public:
    Inventory(Engine &engine,
              std::array<ActorIconSlot, 6> &actorsIconSlots,
              std::array<VerbUiColors, 6> &verbUiColors,
              Actor *&pCurrentActor);

    void update(const sf::Time &elapsed);

    void setMousePosition(const sf::Vector2f &pos) { _mousePos = pos; }
    const InventoryObject *getCurrentInventoryObject() const { return _pCurrentInventoryObject; }

  private:
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
    void drawUpArrow(sf::RenderTarget &target) const;
    void drawDownArrow(sf::RenderTarget &target) const;

  private:
    int getCurrentActorIndex() const;

  private:
    Engine &_engine;
    std::array<ActorIconSlot, 6> &_actorsIconSlots;
    std::array<VerbUiColors, 6> &_verbUiColors;
    SpriteSheet _gameSheet, _inventoryItems;
    Actor *&_pCurrentActor;
    sf::IntRect _inventoryRects[8];
    const InventoryObject *_pCurrentInventoryObject;
    sf::Vector2f _mousePos;
};
} // namespace ng
