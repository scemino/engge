#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <Graphics/SpriteSheet.hpp>
#include <Entities/Entity.hpp>
#include <Entities/Actor/Actor.hpp>
#include "Verb.hpp"
#include "Inventory.hpp"

namespace ng {
class Hud : public sf::Drawable {
private:
  enum class State {
    Off,
    FadeIn,
    On,
    FadeOut
  };
public:
  Hud();

  void setTextureManager(TextureManager *pTextureManager);

  void setVerb(int characterSlot, int verbSlot, const Verb &verb);
  [[nodiscard]] const VerbSlot &getVerbSlot(int characterSlot) const;
  const Verb *getVerb(int id) const;

  void setVerbUiColors(int characterSlot, VerbUiColors colors);
  [[nodiscard]] const VerbUiColors &getVerbUiColors(int characterSlot) const;

  sf::Vector2f findScreenPosition(int verbId) const;

  void setCurrentActor(ng::Actor *pActor);
  void setCurrentActorIndex(int index);

  void setCurrentVerb(const Verb *pVerb) { _pVerb = pVerb; }
  void setVerbOverride(const Verb *pVerb) { _pVerbOverride = pVerb; }
  const Verb *getCurrentVerb() const { return _pVerb; }
  const Verb *getVerbOverride() const { return _pVerbOverride; }
  void setHoveredEntity(Entity *pEntity) { _pHoveredEntity = pEntity; };
  Entity *getHoveredEntity() const { return _pHoveredEntity; }

  void setMousePosition(sf::Vector2f pos);
  const Verb *getHoveredVerb() const;

  const Inventory &getInventory() const { return _inventory; }
  Inventory &getInventory() { return _inventory; }

  void update(const sf::Time &elapsed);

  void setActive(bool active);
  bool getActive() const { return _active; }

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
  std::string getVerbName(const Verb &verb) const;
  int getDefaultVerb(Entity *pEntity) const;

private:
  std::array<VerbSlot, 6> _verbSlots;
  std::array<VerbUiColors, 6> _verbUiColors;
  std::array<sf::IntRect, 9> _verbRects;
  int _currentActorIndex{-1};
  SpriteSheet _verbSheet, _gameSheet;
  const Verb *_pVerb{nullptr};
  const Verb *_pVerbOverride{nullptr};
  Entity *_pHoveredEntity{nullptr};
  mutable sf::Shader _verbShader{};
  sf::Vector2f _mousePos;
  Inventory _inventory;
  bool _active{false};
  State _state{State::Off};
  float _alpha{1.f};
};
}
