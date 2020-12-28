#pragma once
#include <ngf/System/StopWatch.h>
#include "ActorIconSlot.hpp"
#include "Verb.hpp"
#include "Hud.hpp"

namespace ng {
enum class ActorSlotSelectableMode {
  Off = 0,
  On = 1,
  TemporaryUnselectable = 2,
};

class Engine;
class ActorIcons : public ngf::Drawable {
public:
  ActorIcons(std::array<ActorIconSlot, 6> &actorsIconSlots, Hud &hud,
             Actor *&pCurrentActor);

  void setEngine(Engine *pEngine);
  void setMousePosition(const glm::vec2 &pos);
  void update(const ngf::TimeSpan &elapsed);
  [[nodiscard]] bool isMouseOver() const { return _isInside; }
  void flash(bool on);
  void setMode(ActorSlotSelectableMode mode);
  [[nodiscard]] inline ActorSlotSelectableMode getMode() const { return _mode; }
  void setVisible(bool visible) { _visible = visible; }

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const override;
private:
  void drawActorIcon(ngf::RenderTarget &target, const std::string &icon, int actorSlot, const glm::vec2 &offset,
                     float alpha) const;
  void drawActorIcon(ngf::RenderTarget &target, const std::string &icon, ngf::Color backColor, ngf::Color frameColor,
                     const glm::vec2 &offset, float alpha) const;
  [[nodiscard]] int getCurrentActorIndex() const;
  [[nodiscard]] int getIconsNum() const;
  [[nodiscard]] float getOffsetY(int num) const;
  static bool isSelectable(const ActorIconSlot& slot);

private:
  Engine *_pEngine{nullptr};
  std::array<ActorIconSlot, 6> &_actorsIconSlots;
  Hud &_hud;
  Actor *&_pCurrentActor;
  glm::vec2 _mousePos;
  ngf::StopWatch _clock;
  bool _isInside{false};
  bool _on{true};
  float _position{0};
  bool _isMouseButtonPressed{false};
  ngf::TimeSpan _time;
  float _alpha{0};
  ActorSlotSelectableMode _mode{ActorSlotSelectableMode::On};
  bool _visible{true};
};
} // namespace ng
