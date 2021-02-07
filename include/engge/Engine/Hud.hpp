#pragma once
#include "engge/Entities/Entity.hpp"
#include "engge/Entities/Actor.hpp"
#include "Verb.hpp"
#include "Inventory.hpp"

namespace ng {
class Hud : public ngf::Drawable {
private:
  enum class State {
    Off,
    FadeIn,
    On,
    FadeOut
  };

public:
  Hud();

  void setTextureManager(ResourceManager *pTextureManager);

  void setVerb(int characterSlot, int verbSlot, const Verb &verb);
  [[nodiscard]] const VerbSlot &getVerbSlot(int characterSlot) const;
  [[nodiscard]] const Verb *getVerb(int id) const;

  void setVerbUiColors(int characterSlot, VerbUiColors colors);
  [[nodiscard]] const VerbUiColors &getVerbUiColors(int characterSlot) const;

  [[nodiscard]] glm::vec2 findScreenPosition(int verbId) const;

  void setCurrentActor(ng::Actor *pActor);
  void setCurrentActorIndex(int index);

  void setCurrentVerb(const Verb *pVerb) { _pVerb = pVerb; }
  void setVerbOverride(const Verb *pVerb) { _pVerbOverride = pVerb; }
  [[nodiscard]] const Verb *getCurrentVerb() const { return _pVerb; }
  [[nodiscard]] const Verb *getVerbOverride() const { return _pVerbOverride; }
  void setHoveredEntity(Entity *pEntity) { _pHoveredEntity = pEntity; };
  [[nodiscard]] Entity *getHoveredEntity() const { return _pHoveredEntity; }

  void setMousePosition(glm::vec2 pos);
  [[nodiscard]] const Verb *getHoveredVerb() const;
  bool isMouseOver() const;

  Inventory &getInventory() { return _inventory; }

  void update(const ngf::TimeSpan &elapsed);

  void setVisible(bool visible) { _isVisible = visible; }
  void setActive(bool active);
  [[nodiscard]] bool getActive() const { return _active; }

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const override;

private:
  static std::string getVerbName(const Verb &verb);

private:
  std::array<VerbSlot, 6> _verbSlots;
  std::array<VerbUiColors, 6> _verbUiColors;
  std::array<ngf::irect, 9> _verbRects;
  int _currentActorIndex{-1};
  const Verb *_pVerb{nullptr};
  const Verb *_pVerbOverride{nullptr};
  Entity *_pHoveredEntity{nullptr};
  mutable ngf::Shader _verbShader{};
  glm::vec2 _mousePos{0,0};
  Inventory _inventory;
  bool _active{false};
  State _state{State::Off};
  float _alpha{1.f};
  bool _isVisible{true};
};
}
