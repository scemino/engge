
#include "ActorIcons.h"
#include "Engine.h"
#include "Screen.h"

namespace ng
{
ActorIcons::ActorIcons(std::array<ActorIconSlot, 6> &actorsIconSlots,
                       std::array<VerbUiColors, 6> &verbUiColors,
                       Actor *&pCurrentActor)
    : _pEngine(nullptr),
      _actorsIconSlots(actorsIconSlots),
      _verbUiColors(verbUiColors),
      _pCurrentActor(pCurrentActor),
      _isMouseButtonPressed(false)
{
}

void ActorIcons::setEngine(Engine *pEngine)
{
    _pEngine = pEngine;
    _gameSheet.setTextureManager(&_pEngine->getTextureManager());
    _gameSheet.setSettings(&_pEngine->getSettings());
    _gameSheet.load("GameSheet");
}

void ActorIcons::setMousePosition(const sf::Vector2f &pos)
{
    _mousePos = pos;
}

void ActorIcons::update(const sf::Time &elapsed)
{
    sf::FloatRect iconRect(Screen::Width - 16, 0, 16, 16 + (_isInside ? getIconsNum() * 15 : 0));
    bool wasInside = _isInside;
    _isInside = iconRect.contains(_mousePos);
    if (wasInside != _isInside)
    {
        _clock.restart();
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

    if (_isMouseButtonPressed && !sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
    {
        _isMouseButtonPressed = false;
        sf::FloatRect iconRect(Screen::Width - 16, 15, 16, 16);
        for (auto selectableActor : _actorsIconSlots)
        {
            if (!selectableActor.selectable || !selectableActor.pActor || selectableActor.pActor == _pCurrentActor)
                continue;

            if (iconRect.contains(_mousePos))
            {
                _pCurrentActor = selectableActor.pActor;
                _pEngine->follow(_pCurrentActor);
                return;
            }
            iconRect.top += 15;
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

void ActorIcons::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    if (!_pCurrentActor)
        return;

    int numIcons = 0;
    sf::Vector2f offset(Screen::Width - 8, 8);

    if (_pCurrentActor)
    {
        sf::Uint8 alpha = _isInside ? 0xFF : 0x60;

        auto i = getCurrentActorIndex();
        const auto &icon = _actorsIconSlots.at(i).pActor->getIcon();

        offset.y = getOffsetY(numIcons);
        drawActorIcon(target, icon, i, offset, alpha);
        numIcons++;
    }

    if (!_isInside)
        return;

    for (auto i = 0; i < _actorsIconSlots.size(); i++)
    {
        const auto &selectableActor = _actorsIconSlots.at(i);
        if (!selectableActor.selectable || !selectableActor.pActor || selectableActor.pActor == _pCurrentActor)
            continue;

        offset.y = getOffsetY(numIcons);
        const auto &icon = selectableActor.pActor->getIcon();
        drawActorIcon(target, icon, i, offset, 0xFF);
        numIcons++;
    }

    offset.y = getOffsetY(numIcons);
    drawActorIcon(target, "icon_gear", sf::Color::Black, sf::Color(128, 128, 128), offset, 0xFF);
}

void ActorIcons::drawActorIcon(sf::RenderTarget &target, const std::string &icon, int actorSlot, const sf::Vector2f &offset, sf::Uint8 alpha) const
{
    const auto &colors = _verbUiColors[actorSlot];
    drawActorIcon(target, icon, colors.inventoryBackground, colors.inventoryFrame, offset, alpha);
}

void ActorIcons::drawActorIcon(sf::RenderTarget &target, const std::string &icon, sf::Color backColor, sf::Color frameColor, const sf::Vector2f &offset, sf::Uint8 alpha) const
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
    sf::Vector2f pos(-backSourceSize.x / 2.f + backSpriteSourceSize.left, -backSourceSize.y / 2.f + backSpriteSourceSize.top);
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

    pos = sf::Vector2f(-frameSourceSize.x / 2.f + frameSpriteSourceSize.left, -frameSourceSize.y / 2.f + frameSpriteSourceSize.top);
    s.setOrigin(-pos);
    c = frameColor;
    c.a = alpha;
    s.setColor(c);
    s.setTextureRect(frameRect);
    target.draw(s, states);
}

int ActorIcons::getCurrentActorIndex() const
{
    for (auto i = 0; i < _actorsIconSlots.size(); i++)
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
