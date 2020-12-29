#pragma once
#include <array>
#include "engge/Graphics/Screen.hpp"
#include "engge/Graphics/SpriteSheet.hpp"

namespace ng {
class Object;

class Inventory : public ngf::Drawable {
public:
  void setTextureManager(ResourceManager *pTextureManager);
  bool update(const ngf::TimeSpan &elapsed);

  void setCurrentActorIndex(int index) { _currentActorIndex = index; }
  void setCurrentActor(Actor *pActor) { _pCurrentActor = pActor; }
  void setMousePosition(const glm::vec2 &pos) { _mousePos = pos; }
  Object *getCurrentInventoryObject() { return _pCurrentInventoryObject; }
  glm::vec2 getPosition(Object *pObject) const;

  void setVerbUiColors(const VerbUiColors *pColors) { _pColors = pColors; }
  void setAlpha(float alpha) {_alpha = alpha;}
  [[nodiscard]] float getAlpha() const {return _alpha;}

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const override;
private:
  void drawUpArrow(ngf::RenderTarget &target) const;
  void drawDownArrow(ngf::RenderTarget &target) const;
  [[nodiscard]] bool hasUpArrow() const;
  [[nodiscard]] bool hasDownArrow() const;

private:
  SpriteSheet _gameSheet, _inventoryItems;
  std::array<ngf::frect, 8> _inventoryRects;
  ngf::frect _scrollUpRect;
  ngf::frect _scrollDownRect;
  Object *_pCurrentInventoryObject{nullptr};
  glm::vec2 _mousePos{0,0};
  float _jiggleTime{0};
  Actor* _pCurrentActor{nullptr};
  int _currentActorIndex{0};
  const VerbUiColors *_pColors{nullptr};
  float _alpha{1.f};
  bool _mouseWasDown{false};
};
} // namespace ng
