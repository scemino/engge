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
class ActorIcons final : public ngf::Drawable {
public:
  ActorIcons(std::array<ActorIconSlot, 6> &actorsIconSlots, Hud &hud,
             Actor *&pCurrentActor);

  void setEngine(Engine *pEngine);
  void setMousePosition(const glm::vec2 &pos);
  void update(const ngf::TimeSpan &elapsed);
  [[nodiscard]] bool isMouseOver() const { return m_isInside; }
  void flash(bool on);
  void setMode(ActorSlotSelectableMode mode);
  [[nodiscard]] inline ActorSlotSelectableMode getMode() const { return m_mode; }
  void setVisible(bool visible) { m_visible = visible; }

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const final;

private:
  void drawActorIcon(ngf::RenderTarget &target, const std::string &icon, int actorSlot, const glm::vec2 &offset,
                     float alpha) const;
  static void drawActorIcon(ngf::RenderTarget &target,
                            const std::string &icon,
                            ngf::Color backColor,
                            ngf::Color frameColor,
                            const glm::vec2 &offset,
                            float alpha);
  [[nodiscard]] int getCurrentActorIndex() const;
  [[nodiscard]] int getIconsNum() const;
  [[nodiscard]] float getOffsetY(int num) const;
  static bool isSelectable(const ActorIconSlot &slot);

private:
  Engine *m_pEngine{nullptr};
  std::array<ActorIconSlot, 6> &m_actorsIconSlots;
  Hud &m_hud;
  Actor *&m_pCurrentActor;
  glm::vec2 m_mousePos{0, 0};
  ngf::StopWatch m_clock;
  bool m_isInside{false};
  bool m_on{true};
  float m_position{0};
  bool m_isMouseButtonPressed{false};
  ngf::TimeSpan m_time;
  float m_alpha{0};
  ActorSlotSelectableMode m_mode{ActorSlotSelectableMode::On};
  bool m_visible{true};
};
} // namespace ng
