#pragma once
#include "SFML/Graphics.hpp"
#include "SpriteSheet.h"
#include "ActorIconSlot.h"
#include "Verb.h"

namespace ng
{
class Engine;
class ActorIcons : public sf::Drawable
{
  public:
    ActorIcons(Engine &engine,
               std::array<ActorIconSlot, 6> &actorsIconSlots,
               std::array<VerbUiColors, 6> &verbUiColors,
               Actor *&pCurrentActor);

    void setMousePosition(const sf::Vector2f &pos);
    void update(const sf::Time &elapsed);
    bool isMouseOver() const { return _isInside; }

  private:
    void drawActorIcon(sf::RenderTarget &target, const std::string &icon, int actorSlot, const sf::Vector2f &offset, sf::Uint8 alpha) const;
    void drawActorIcon(sf::RenderTarget &target, const std::string &icon, sf::Color backColor, sf::Color frameColor, const sf::Vector2f &offset, sf::Uint8 alpha) const;
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
    int getCurrentActorIndex() const;
    int getIconsNum() const;
    float getOffsetY(int num) const;

  private:
    Engine &_engine;
    SpriteSheet _gameSheet;
    std::array<ActorIconSlot, 6> &_actorsIconSlots;
    std::array<VerbUiColors, 6> &_verbUiColors;
    Actor *&_pCurrentActor;
    sf::Vector2f _mousePos;
    sf::Clock _clock;
    bool _isInside;
    float _position;
    bool _isMouseButtonPressed;
};
} // namespace ng
