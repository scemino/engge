#define _USE_MATH_DEFINES
#include <cmath>
#include "Engine/ActorIcons.hpp"
#include "Engine/Engine.hpp"

namespace ng
{
ActorIcons::ActorIcons(std::array<ActorIconSlot, 6> &actorsIconSlots, std::array<VerbUiColors, 6> &verbUiColors,
                       Actor *&pCurrentActor)
    : _actorsIconSlots(actorsIconSlots), _verbUiColors(verbUiColors), _pCurrentActor(pCurrentActor)
{
}

void ActorIcons::setEngine(Engine *pEngine)
{
    _pEngine = pEngine;
    _gameSheet.setTextureManager(&_pEngine->getTextureManager());
    _gameSheet.load("GameSheet");
}

void ActorIcons::setMousePosition(const sf::Vector2f &pos) { _mousePos = pos; }

void ActorIcons::update(const sf::Time &elapsed)
{
    if (_on)
    {
        _time += elapsed;
        _alpha = 160 + 96 * sinf(M_PI * 4 * _time.asSeconds());

        if (_time > sf::seconds(40))
        {
            flash(false);
        }
    }
    auto screen = _pEngine->getWindow().getView().getSize();
    sf::FloatRect iconRect(screen.x - 16, 0, 16, 16 + (_isInside ? getIconsNum() * 15 : 0));
    bool wasInside = _isInside;
    _isInside = iconRect.contains(_mousePos);
    if (wasInside != _isInside)
    {
        _clock.restart();
        if (_isInside)
        {
            flash(false);
        }
    }
    _position = _clock.getElapsedTime() / sf::milliseconds(250);
    if (_position > 1)
    {
        _position = 1;
    }

    if (_isInside && !_isMouseButtonPressed && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
    {
        _isMouseButtonPressed = true;
        return;
    }

    auto isEnabled = _mode == ActorSlotSelectableMode::On || _mode == ActorSlotSelectableMode::TemporarySelectable;
    if (isEnabled && _isMouseButtonPressed && !sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
    {
        _isMouseButtonPressed = false;
        iconRect = sf::FloatRect(screen.x - 16, 15, 16, 16);
        for (auto selectableActor : _actorsIconSlots)
        {
            if (!selectableActor.selectable || !selectableActor.pActor || selectableActor.pActor == _pCurrentActor)
                continue;

            if (iconRect.contains(_mousePos))
            {
                _pEngine->setCurrentActor(selectableActor.pActor, true);
                return;
            }
            iconRect.top += 15;
        }
        if (iconRect.contains(_mousePos))
        {
            _pEngine->showOptions(true);
            return;
        }
    }
}

float ActorIcons::getOffsetY(int num) const
{
    if (_isInside)
        return (8 + 15 * num) * _position;
    return 8 + 15 * num;
}

int ActorIcons::getIconsNum() const
{
    if (!_pCurrentActor)
        return 0;
    int numIcons = 1;
    for (auto selectableActor : _actorsIconSlots)
    {
        if (!selectableActor.selectable || !selectableActor.pActor || selectableActor.pActor == _pCurrentActor)
            continue;

        numIcons++;
    }
    return numIcons;
}

void ActorIcons::flash(bool on)
{
    _time = sf::seconds(0);
    _alpha = 0x60;
    _on = on;
}

void ActorIcons::setMode(ActorSlotSelectableMode mode) { _mode = mode; }

void ActorIcons::draw(sf::RenderTarget &target, sf::RenderStates) const
{
    if (!_pCurrentActor || _mode == ActorSlotSelectableMode::Off)
        return;

    int numIcons = 0;
    auto screen = target.getView().getSize();
    sf::Vector2f offset(screen.x - 8, 8);

    sf::Uint8 alpha;
    auto isEnabled = _mode == ActorSlotSelectableMode::On || _mode == ActorSlotSelectableMode::TemporarySelectable;
    if(isEnabled)
    {
        alpha = _isInside ? 0xFF : _alpha;
    }
    else
    {
        alpha = 0x60;
    }

    auto currentActorIndex = getCurrentActorIndex();
    const auto &icon = _actorsIconSlots.at(currentActorIndex).pActor->getIcon();

    offset.y = getOffsetY(numIcons);
    drawActorIcon(target, icon, currentActorIndex, offset, alpha);
    numIcons++;

    if (!_isInside)
        return;

    for (size_t i = 0; i < _actorsIconSlots.size(); i++)
    {
        const auto &selectableActor = _actorsIconSlots.at(i);
        if (!selectableActor.selectable || !selectableActor.pActor || selectableActor.pActor == _pCurrentActor)
            continue;

        offset.y = getOffsetY(numIcons);
        const auto &icon2 = selectableActor.pActor->getIcon();
        drawActorIcon(target, icon2, i, offset, isEnabled ? 0xFF : 0x60);
        numIcons++;
    }

    offset.y = getOffsetY(numIcons);
    drawActorIcon(target, "icon_gear", sf::Color::Black, sf::Color(128, 128, 128), offset, 0xFF);
}

void ActorIcons::drawActorIcon(sf::RenderTarget &target, const std::string &icon, int actorSlot,
                               const sf::Vector2f &offset, sf::Uint8 alpha) const
{
    const auto &colors = _verbUiColors[actorSlot];
    drawActorIcon(target, icon, colors.inventoryBackground, colors.inventoryFrame, offset, alpha);
}

void ActorIcons::drawActorIcon(sf::RenderTarget &target, const std::string &icon, sf::Color backColor,
                               sf::Color frameColor, const sf::Vector2f &offset, sf::Uint8 alpha) const
{
    sf::RenderStates states;
    const auto &texture = _gameSheet.getTexture();
    auto backRect = _gameSheet.getRect("icon_background");
    auto backSpriteSourceSize = _gameSheet.getSpriteSourceSize("icon_background");
    auto backSourceSize = _gameSheet.getSourceSize("icon_background");

    auto frameRect = _gameSheet.getRect("icon_frame");
    auto frameSpriteSourceSize = _gameSheet.getSpriteSourceSize("icon_frame");
    auto frameSourceSize = _gameSheet.getSourceSize("icon_frame");

    sf::Sprite s;
    sf::Vector2f pos(-backSourceSize.x / 2.f + backSpriteSourceSize.left,
                     -backSourceSize.y / 2.f + backSpriteSourceSize.top);
    s.scale(0.5f, 0.5f);
    sf::Color c(backColor);
    c.a = alpha;
    s.setColor(c);
    s.setPosition(offset);
    s.setOrigin(-pos);
    s.setTextureRect(backRect);
    s.setTexture(texture);
    target.draw(s, states);

    auto rect = _gameSheet.getRect(icon);
    auto spriteSourceSize = _gameSheet.getSpriteSourceSize(icon);
    auto sourceSize = _gameSheet.getSourceSize(icon);
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

int ActorIcons::getCurrentActorIndex() const
{
    for (size_t i = 0; i < _actorsIconSlots.size(); i++)
    {
        const auto &selectableActor = _actorsIconSlots.at(i);
        if (selectableActor.pActor == _pCurrentActor)
        {
            return i;
        }
    }
    return -1;
}
} // namespace ng
