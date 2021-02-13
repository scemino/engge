#define _USE_MATH_DEFINES
#include <cmath>
#include <ngf/Graphics/Sprite.h>
#include <ngf/System/Mouse.h>
#include "engge/System/Locator.hpp"
#include "engge/Graphics/SpriteSheet.hpp"
#include "engge/Engine/ActorIcons.hpp"
#include "engge/Engine/Engine.hpp"
#include "engge/Room/Room.hpp"

namespace ng {

static const float topMargin = 4.f;
static const float rightMargin = 6.f;
static const float iconsMargin = 6.f;
static const glm::vec2 iconSize = {48.f, 54.f};
static const float disableAlpha = 0.5f;
static const float enableAlpha = 1.0f;

ActorIcons::ActorIcons(std::array<ActorIconSlot, 6> &actorsIconSlots, Hud &hud,
                       Actor *&pCurrentActor)
    : m_actorsIconSlots(actorsIconSlots), m_hud(hud), m_pCurrentActor(pCurrentActor) {
}

void ActorIcons::setEngine(Engine *pEngine) {
  m_pEngine = pEngine;
}

void ActorIcons::setMousePosition(const glm::vec2 &pos) { m_mousePos = pos; }

void ActorIcons::update(const ngf::TimeSpan &elapsed) {
  if (m_on) {
    m_time += elapsed;
    m_alpha = (160.f + 96.f * sinf(M_PI * 4 * m_time.getTotalSeconds())) / 255.f;

    if (m_time > ngf::TimeSpan::seconds(40)) {
      flash(false);
    }
  }
  ngf::frect iconRect = ngf::frect::fromPositionSize({Screen::Width - iconSize.x - rightMargin, topMargin},
                                                     {iconSize.x, iconSize.y
                                                         + (m_isInside ? getIconsNum() * (iconSize.y + iconsMargin)
                                                                       : 0.f)});
  bool wasInside = m_isInside;
  m_isInside = iconRect.contains(m_mousePos);
  if (wasInside != m_isInside) {
    m_clock.restart();
    if (m_isInside) {
      flash(false);
    }
  }
  m_position = m_clock.getElapsedTime().getTotalSeconds() / ngf::TimeSpan::milliseconds(250).getTotalSeconds();
  if (m_position > 1) {
    m_position = 1;
  }

  if (m_isInside && !m_isMouseButtonPressed && ngf::Mouse::isButtonPressed(ngf::Mouse::Button::Left)) {
    m_isMouseButtonPressed = true;
    return;
  }

  auto isEnabled = ((m_mode & ActorSlotSelectableMode::On) == ActorSlotSelectableMode::On)
      && ((m_mode & ActorSlotSelectableMode::TemporaryUnselectable) != ActorSlotSelectableMode::TemporaryUnselectable);
  if (m_isMouseButtonPressed && !ngf::Mouse::isButtonPressed(ngf::Mouse::Button::Left)) {
    m_isMouseButtonPressed = false;
    iconRect =
        ngf::frect::fromPositionSize({Screen::Width - iconSize.x - rightMargin, topMargin + iconsMargin + iconSize.y},
                                     {iconSize.x, iconSize.y});
    for (auto selectableActor : m_actorsIconSlots) {
      if (!isSelectable(selectableActor) || selectableActor.pActor == m_pCurrentActor)
        continue;

      if (isEnabled && iconRect.contains(m_mousePos)) {
        m_pEngine->setCurrentActor(selectableActor.pActor, true);
        return;
      }
      iconRect.min.y += iconsMargin + iconSize.y;
      iconRect.max.y += iconsMargin + iconSize.y;
    }
    if (iconRect.contains(m_mousePos)) {
      m_pEngine->showOptions(true);
      return;
    }
  }
}

float ActorIcons::getOffsetY(int num) const {
  if (m_isInside)
    return (topMargin + (iconSize.y / 2.f) + (iconSize.y + iconsMargin) * num) * m_position;
  return topMargin + (iconSize.y / 2.f) + (iconSize.y + iconsMargin) * num;
}

int ActorIcons::getIconsNum() const {
  int numIcons = 1;
  for (auto selectableActor : m_actorsIconSlots) {
    if (!selectableActor.selectable || !selectableActor.pActor || selectableActor.pActor == m_pCurrentActor)
      continue;

    numIcons++;
  }
  return numIcons;
}

void ActorIcons::flash(bool on) {
  m_time = ngf::TimeSpan::seconds(0);
  m_alpha = disableAlpha;
  m_on = on;
}

void ActorIcons::setMode(ActorSlotSelectableMode mode) { m_mode = mode; }

bool ActorIcons::isSelectable(const ActorIconSlot &slot) {
  if (!slot.selectable)
    return false;

  // the actor is not selectable if he is in room "Void"
  const auto *pRoom = slot.pActor ? slot.pActor->getRoom() : nullptr;
  if (pRoom && pRoom->getName() == "Void") {
    pRoom = nullptr;
  }
  return pRoom;
}

void ActorIcons::draw(ngf::RenderTarget &target, ngf::RenderStates) const {
  if ((m_mode & ActorSlotSelectableMode::TemporaryUnselectable) == ActorSlotSelectableMode::TemporaryUnselectable)
    return;

  auto currentActorIndex = getCurrentActorIndex();
  if (currentActorIndex < 0)
    return;

  const auto view = target.getView();
  target.setView(ngf::View(ngf::frect::fromPositionSize({0, 0}, {Screen::Width, Screen::Height})));

  float alpha;
  auto isEnabled = ((m_mode & ActorSlotSelectableMode::On) == ActorSlotSelectableMode::On)
      && ((m_mode & ActorSlotSelectableMode::TemporaryUnselectable) != ActorSlotSelectableMode::TemporaryUnselectable);
  if (isEnabled) {
    alpha = m_isInside ? enableAlpha : m_alpha;
  } else {
    alpha = disableAlpha;
  }

  const auto &icon = m_actorsIconSlots.at(currentActorIndex).pActor->getIcon();

  int numIcons = 0;
  glm::vec2 offset(Screen::Width - (iconSize.x / 2.f) - rightMargin, getOffsetY(numIcons));
  drawActorIcon(target, icon, currentActorIndex, offset, alpha);
  numIcons++;

  if (!m_isInside) {
    target.setView(view);
    return;
  }

  for (size_t i = 0; i < m_actorsIconSlots.size(); i++) {
    const auto &selectableActor = m_actorsIconSlots.at(i);
    if (!isSelectable(selectableActor) || selectableActor.pActor == m_pCurrentActor)
      continue;

    offset.y = getOffsetY(numIcons);
    const auto &icon2 = selectableActor.pActor->getIcon();
    drawActorIcon(target, icon2, i, offset, isEnabled ? enableAlpha : disableAlpha);
    numIcons++;
  }

  offset.y = getOffsetY(numIcons);
  drawActorIcon(target, "icon_gear", ngf::Colors::Black, ngf::Color(128, 128, 128), offset, enableAlpha);

  target.setView(view);
}

void ActorIcons::drawActorIcon(ngf::RenderTarget &target, const std::string &icon, int actorSlot,
                               const glm::vec2 &offset, float alpha) const {
  const auto &colors = m_hud.getVerbUiColors(actorSlot);
  drawActorIcon(target, icon, colors.inventoryBackground, colors.inventoryFrame, offset, alpha);
}

void ActorIcons::drawActorIcon(ngf::RenderTarget &target, const std::string &icon, ngf::Color backColor,
                               ngf::Color frameColor, const glm::vec2 &offset, float alpha) {
  ngf::RenderStates states;
  auto &gameSheet = Locator<ResourceManager>::get().getSpriteSheet("GameSheet");
  const auto &texture = gameSheet.getTexture();
  auto backRect = gameSheet.getRect("icon_background");
  auto backSpriteSourceSize = gameSheet.getSpriteSourceSize("icon_background");
  auto backSourceSize = gameSheet.getSourceSize("icon_background");

  auto rect = gameSheet.getRect(icon);
  auto spriteSourceSize = gameSheet.getSpriteSourceSize(icon);
  auto sourceSize = gameSheet.getSourceSize(icon);

  auto frameRect = gameSheet.getRect("icon_frame");
  auto frameSpriteSourceSize = gameSheet.getSpriteSourceSize("icon_frame");
  auto frameSourceSize = gameSheet.getSourceSize("icon_frame");

  // draw icon background
  ngf::Sprite s;
  glm::vec2 pos(-backSourceSize.x / 2.f + backSpriteSourceSize.getTopLeft().x,
                -backSourceSize.y / 2.f + backSpriteSourceSize.getTopLeft().y);
  ngf::Color c(backColor);
  c.a = alpha;
  s.getTransform().setScale({2, 2});
  s.setColor(c);
  s.getTransform().setPosition(offset);
  s.getTransform().setOrigin(-pos);
  s.setTexture(*texture);
  s.setTextureRect(backRect);
  s.draw(target, states);

  // draw actor's icon
  pos = glm::vec2(-sourceSize.x / 2.f + spriteSourceSize.getTopLeft().x,
                  -sourceSize.y / 2.f + spriteSourceSize.getTopLeft().y);
  s.getTransform().setOrigin(-pos);
  c = ngf::Colors::White;
  c.a = alpha;
  s.setColor(c);
  s.setTextureRect(rect);
  s.draw(target, states);

  // draw frame
  pos = glm::vec2(-frameSourceSize.x / 2.f + frameSpriteSourceSize.getTopLeft().x,
                  -frameSourceSize.y / 2.f + frameSpriteSourceSize.getTopLeft().y);
  s.getTransform().setOrigin(-pos);
  c = frameColor;
  c.a = alpha;
  s.setColor(c);
  s.setTextureRect(frameRect);
  s.draw(target, states);
}

int ActorIcons::getCurrentActorIndex() const {
  for (size_t i = 0; i < m_actorsIconSlots.size(); i++) {
    const auto &selectableActor = m_actorsIconSlots.at(i);
    if (selectableActor.pActor == m_pCurrentActor) {
      return i;
    }
  }
  return -1;
}
} // namespace ng
