#pragma once
#include <array>
#include "engge/Graphics/Screen.hpp"
#include "engge/Graphics/SpriteSheet.hpp"

namespace ng {
class Object;

class Inventory final : public ngf::Drawable {
public:
  void setTextureManager(ResourceManager *pTextureManager);
  bool update(const ngf::TimeSpan &elapsed);

  void setCurrentActorIndex(int index) { m_currentActorIndex = index; }
  void setCurrentActor(Actor *pActor) { m_pCurrentActor = pActor; }
  void setMousePosition(const glm::vec2 &pos) { m_mousePos = pos; }
  Object *getCurrentInventoryObject() { return m_pCurrentInventoryObject; }
  glm::vec2 getPosition(Object *pObject) const;

  void setVerbUiColors(const VerbUiColors *pColors) { m_pColors = pColors; }
  void setAlpha(float alpha) { m_alpha = alpha;}
  [[nodiscard]] float getAlpha() const {return m_alpha;}

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const final;

private:
  void drawUpArrow(ngf::RenderTarget &target) const;
  void drawDownArrow(ngf::RenderTarget &target) const;
  [[nodiscard]] bool hasUpArrow() const;
  [[nodiscard]] bool hasDownArrow() const;

private:
  SpriteSheet m_gameSheet, m_inventoryItems;
  std::array<ngf::frect, 8> m_inventoryRects;
  ngf::frect m_scrollUpRect;
  ngf::frect m_scrollDownRect;
  Object *m_pCurrentInventoryObject{nullptr};
  glm::vec2 m_mousePos{0, 0};
  float m_jiggleTime{0};
  Actor* m_pCurrentActor{nullptr};
  int m_currentActorIndex{0};
  const VerbUiColors *m_pColors{nullptr};
  float m_alpha{1.f};
  bool m_mouseWasDown{false};
};
} // namespace ng
