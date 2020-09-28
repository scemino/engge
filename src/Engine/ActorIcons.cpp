#define _USE_MATH_DEFINES
#include <cmath>
#include "engge/System/Locator.hpp"
#include "engge/Graphics/SpriteSheet.hpp"
#include "engge/Engine/ActorIcons.hpp"
#include "engge/Engine/Engine.hpp"
#include "engge/Room/Room.hpp"

namespace ng {

static const float topMargin = 4.f;
static const float rightMargin = 6.f;
static const float iconsMargin = 6.f;
static const sf::Vector2f iconSize = {48.f, 54.f};
static const sf::Uint8 disableAlpha = 0x60;
static const sf::Uint8 enableAlpha = 0xFF;

ActorIcons::ActorIcons(std::array<ActorIconSlot, 6> &actorsIconSlots, Hud &hud,
                       Actor *&pCurrentActor)
    : _actorsIconSlots(actorsIconSlots), _hud(hud), _pCurrentActor(pCurrentActor) {
}

void ActorIcons::setEngine(Engine *pEngine) {
  _pEngine = pEngine;
}

void ActorIcons::setMousePosition(const sf::Vector2f &pos) { _mousePos = pos; }

void ActorIcons::update(const sf::Time &elapsed) {
  if (_on) {
    _time += elapsed;
    _alpha = 160 + 96 * sinf(M_PI * 4 * _time.asSeconds());

    if (_time > sf::seconds(40)) {
      flash(false);
    }
  }
  sf::FloatRect iconRect
      (Screen::Width - iconSize.x - rightMargin,
       topMargin,
       iconSize.x,
       iconSize.y + (_isInside ? getIconsNum() * (iconSize.y + iconsMargin) : 0.f));
  bool wasInside = _isInside;
  _isInside = iconRect.contains(_mousePos);
  if (wasInside != _isInside) {
    _clock.restart();
    if (_isInside) {
      flash(false);
    }
  }
  _position = _clock.getElapsedTime() / sf::milliseconds(250);
  if (_position > 1) {
    _position = 1;
  }

  if (_isInside && !_isMouseButtonPressed && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
    _isMouseButtonPressed = true;
    return;
  }

  auto isEnabled = ((_mode & ActorSlotSelectableMode::On) == ActorSlotSelectableMode::On)
      && ((_mode & ActorSlotSelectableMode::TemporaryUnselectable) != ActorSlotSelectableMode::TemporaryUnselectable);
  if (_isMouseButtonPressed && !sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
    _isMouseButtonPressed = false;
    iconRect = sf::FloatRect(Screen::Width - iconSize.x - rightMargin,
                             topMargin + iconsMargin + iconSize.y,
                             iconSize.x,
                             iconSize.y);
    for (auto selectableActor : _actorsIconSlots) {
      if (!isSelectable(selectableActor) || selectableActor.pActor == _pCurrentActor)
        continue;

      if (isEnabled && iconRect.contains(_mousePos)) {
        _pEngine->setCurrentActor(selectableActor.pActor, true);
        return;
      }
      iconRect.top += iconsMargin + iconSize.y;
    }
    if (iconRect.contains(_mousePos)) {
      _pEngine->showOptions(true);
      return;
    }
  }
}

float ActorIcons::getOffsetY(int num) const {
  if (_isInside)
    return (topMargin + (iconSize.y / 2.f) + (iconSize.y + iconsMargin) * num) * _position;
  return topMargin + (iconSize.y / 2.f) + (iconSize.y + iconsMargin) * num;
}

int ActorIcons::getIconsNum() const {
  int numIcons = 1;
  for (auto selectableActor : _actorsIconSlots) {
    if (!selectableActor.selectable || !selectableActor.pActor || selectableActor.pActor == _pCurrentActor)
      continue;

    numIcons++;
  }
  return numIcons;
}

void ActorIcons::flash(bool on) {
  _time = sf::seconds(0);
  _alpha = disableAlpha;
  _on = on;
}

void ActorIcons::setMode(ActorSlotSelectableMode mode) { _mode = mode; }

bool ActorIcons::isSelectable(const ActorIconSlot& slot){
  if(!slot.selectable) return false;

  // the actor is not selectable if he is in room "Void"
  const auto *pRoom = slot.pActor ? slot.pActor->getRoom() : nullptr;
  if (pRoom && pRoom->getName() == "Void") {
    pRoom = nullptr;
  }
  return pRoom;
}

void ActorIcons::draw(sf::RenderTarget &target, sf::RenderStates) const {
  if ((_mode & ActorSlotSelectableMode::TemporaryUnselectable) == ActorSlotSelectableMode::TemporaryUnselectable)
    return;

  auto currentActorIndex = getCurrentActorIndex();
  if (currentActorIndex < 0)
    return;

  const auto view = target.getView();
  target.setView(sf::View(sf::FloatRect(0, 0, Screen::Width, Screen::Height)));

  sf::Uint8 alpha;
  auto isEnabled = ((_mode & ActorSlotSelectableMode::On) == ActorSlotSelectableMode::On)
      && ((_mode & ActorSlotSelectableMode::TemporaryUnselectable) != ActorSlotSelectableMode::TemporaryUnselectable);
  if (isEnabled) {
    alpha = _isInside ? enableAlpha : _alpha;
  } else {
    alpha = disableAlpha;
  }

  const auto &icon = _actorsIconSlots.at(currentActorIndex).pActor->getIcon();

  int numIcons = 0;
  sf::Vector2f offset(Screen::Width - (iconSize.x / 2.f) - rightMargin, getOffsetY(numIcons));
  drawActorIcon(target, icon, currentActorIndex, offset, alpha);
  numIcons++;

  if (!_isInside) {
    target.setView(view);
    return;
  }

  for (size_t i = 0; i < _actorsIconSlots.size(); i++) {
    const auto &selectableActor = _actorsIconSlots.at(i);
    if (!isSelectable(selectableActor) || selectableActor.pActor == _pCurrentActor)
      continue;

    offset.y = getOffsetY(numIcons);
    const auto &icon2 = selectableActor.pActor->getIcon();
    drawActorIcon(target, icon2, i, offset, isEnabled ? enableAlpha : disableAlpha);
    numIcons++;
  }

  offset.y = getOffsetY(numIcons);
  drawActorIcon(target, "icon_gear", sf::Color::Black, sf::Color(128, 128, 128), offset, enableAlpha);

  target.setView(view);
}

void ActorIcons::drawActorIcon(sf::RenderTarget &target, const std::string &icon, int actorSlot,
                               const sf::Vector2f &offset, sf::Uint8 alpha) const {
  const auto &colors = _hud.getVerbUiColors(actorSlot);
  drawActorIcon(target, icon, colors.inventoryBackground, colors.inventoryFrame, offset, alpha);
}

void ActorIcons::drawActorIcon(sf::RenderTarget &target, const std::string &icon, sf::Color backColor,
                               sf::Color frameColor, const sf::Vector2f &offset, sf::Uint8 alpha) const {
  sf::RenderStates states;
  auto &gameSheet = Locator<ResourceManager>::get().getSpriteSheet("GameSheet");
  const auto &texture = gameSheet.getTexture();
  auto backRect = gameSheet.getRect("icon_background");
  auto backSpriteSourceSize = gameSheet.getSpriteSourceSize("icon_background");
  auto backSourceSize = gameSheet.getSourceSize("icon_background");

  auto frameRect = gameSheet.getRect("icon_frame");
  auto frameSpriteSourceSize = gameSheet.getSpriteSourceSize("icon_frame");
  auto frameSourceSize = gameSheet.getSourceSize("icon_frame");

  sf::Sprite s;
  sf::Vector2f pos(-backSourceSize.x / 2.f + backSpriteSourceSize.left,
                   -backSourceSize.y / 2.f + backSpriteSourceSize.top);
  sf::Color c(backColor);
  c.a = alpha;
  s.scale(2, 2);
  s.setColor(c);
  s.setPosition(offset);
  s.setOrigin(-pos);
  s.setTextureRect(backRect);
  s.setTexture(texture);
  target.draw(s, states);

  auto rect = gameSheet.getRect(icon);
  auto spriteSourceSize = gameSheet.getSpriteSourceSize(icon);
  auto sourceSize = gameSheet.getSourceSize(icon);
  pos = sf::Vector2f(-sourceSize.x / 2.f + spriteSourceSize.left, -sourceSize.y / 2.f + spriteSourceSize.top);
  s.setOrigin(-pos);
  c = sf::Color::White;
  c.a = alpha;
  s.setColor(c);
  s.setTextureRect(rect);
  target.draw(s, states);

  pos = sf::Vector2f(-frameSourceSize.x / 2.f + frameSpriteSourceSize.left,
                     -frameSourceSize.y / 2.f + frameSpriteSourceSize.top);
  s.setOrigin(-pos);
  c = frameColor;
  c.a = alpha;
  s.setColor(c);
  s.setTextureRect(frameRect);
  target.draw(s, states);
}

int ActorIcons::getCurrentActorIndex() const {
  for (size_t i = 0; i < _actorsIconSlots.size(); i++) {
    const auto &selectableActor = _actorsIconSlots.at(i);
    if (selectableActor.pActor == _pCurrentActor) {
      return i;
    }
  }
  return -1;
}
} // namespace ng
