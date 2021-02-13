#pragma once
#include "engge/Entities/Entity.hpp"
#include "engge/Entities/Actor.hpp"
#include "Verb.hpp"
#include "Inventory.hpp"

namespace ng {
class Hud final : public ngf::Drawable {
private:
  enum class State {
    Off,
    FadeIn,
    On,
    FadeOut
  };

public:
  Hud();
  ~Hud() final;

  void setTextureManager(ResourceManager *pTextureManager);

  void setVerb(int characterSlot, int verbSlot, const Verb &verb);
  [[nodiscard]] const VerbSlot &getVerbSlot(int characterSlot) const;
  [[nodiscard]] const Verb *getVerb(int id) const;

  void setVerbUiColors(int characterSlot, VerbUiColors colors);
  [[nodiscard]] const VerbUiColors &getVerbUiColors(int characterSlot) const;

  [[nodiscard]] glm::vec2 findScreenPosition(int verbId) const;

  void setCurrentActor(ng::Actor *pActor);
  void setCurrentActorIndex(int index);

  void setCurrentVerb(const Verb *pVerb) { m_pVerb = pVerb; }
  void setVerbOverride(const Verb *pVerb) { m_pVerbOverride = pVerb; }
  [[nodiscard]] const Verb *getCurrentVerb() const { return m_pVerb; }
  [[nodiscard]] const Verb *getVerbOverride() const { return m_pVerbOverride; }
  void setHoveredEntity(Entity *pEntity) { m_pHoveredEntity = pEntity; };
  [[nodiscard]] Entity *getHoveredEntity() const { return m_pHoveredEntity; }

  void setMousePosition(glm::vec2 pos);
  [[nodiscard]] const Verb *getHoveredVerb() const;
  bool isMouseOver() const;

  Inventory &getInventory() { return m_inventory; }

  void update(const ngf::TimeSpan &elapsed);

  void setVisible(bool visible) { m_isVisible = visible; }
  void setActive(bool active);
  [[nodiscard]] bool getActive() const { return m_active; }

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const final;

private:
  static std::string getVerbName(const Verb &verb);

private:
  std::array<VerbSlot, 6> m_verbSlots;
  std::array<VerbUiColors, 6> m_verbUiColors;
  std::array<ngf::irect, 9> m_verbRects;
  int m_currentActorIndex{-1};
  const Verb *m_pVerb{nullptr};
  const Verb *m_pVerbOverride{nullptr};
  Entity *m_pHoveredEntity{nullptr};
  mutable ngf::Shader m_verbShader{};
  glm::vec2 m_mousePos{0, 0};
  Inventory m_inventory;
  bool m_active{false};
  State m_state{State::Off};
  float m_alpha{1.f};
  bool m_isVisible{true};
};
}
