#include "Engine.h"
#include "Inventory.h"
#include "Object.h"
#include "Screen.h"

namespace ng
{
Inventory::Inventory(std::array<ActorIconSlot, 6> &actorsIconSlots,
                     std::array<VerbUiColors, 6> &verbUiColors,
                     Actor *&pCurrentActor)
    : _actorsIconSlots(actorsIconSlots),
      _verbUiColors(verbUiColors),
      _pCurrentActor(pCurrentActor)
{
}

void Inventory::setEngine(Engine *pEngine)
{
    _pEngine = pEngine;
    _gameSheet.setTextureManager(&_pEngine->getTextureManager());
    _gameSheet.setSettings(&_pEngine->getSettings());
    _gameSheet.load("GameSheet");

    _inventoryItems.setTextureManager(&_pEngine->getTextureManager());
    _inventoryItems.setSettings(&_pEngine->getSettings());
    _inventoryItems.load("InventoryItems");
}

void Inventory::update(const sf::Time &elapsed)
{
    if (_pCurrentActor == nullptr)
        return;
    _pCurrentInventoryObject = nullptr;

    auto screen = _pEngine->getWindow().getView().getSize();
    // inventory rects
    auto x = 0, y = 0;
    auto ratio = sf::Vector2f(screen.x / 1280.f, screen.y / 720.f);
    auto scrollUpFrameRect = _gameSheet.getRect("scroll_up");
    sf::Vector2f scrollUpPosition(screen.x / 2.f, screen.y - 3 * screen.y / 14.f);
    sf::Vector2f scrollUpSize(scrollUpFrameRect.width * ratio.x, scrollUpFrameRect.height * ratio.y);
    for (auto i = 0; i < 8; i++)
    {
        sf::Vector2f pos(x + scrollUpPosition.x + scrollUpSize.x, y + screen.y - 3 * screen.y / 14.f);
        auto size = sf::Vector2f(206.f * screen.x / 1920.f, 112.f * screen.y / 1080.f);
        _inventoryRects[i] = sf::IntRect(pos.x, pos.y, size.x, size.y);
        x += size.x;
        if (i == 3)
        {
            x = 0;
            y += size.y;
        }
    }

    for (size_t i = 0; i < _inventoryRects.size(); i++)
    {
        const auto &r = _inventoryRects.at(i);
        if (r.contains((sf::Vector2i)_mousePos))
        {
            auto &objects = _pCurrentActor->getObjects();
            if (i < objects.size())
            {
                _pCurrentInventoryObject = objects[i];
                return;
            }
        }
    }
}

int Inventory::getCurrentActorIndex() const
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

void Inventory::drawUpArrow(sf::RenderTarget &target) const
{
    int currentActorIndex = getCurrentActorIndex();
    auto rect = _gameSheet.getRect("scroll_up");

    sf::Vector2f scrollUpSize(rect.width, rect.height);
    sf::Vector2f scrollUpPosition(Screen::Width / 2.f, 580.f);
    sf::RectangleShape scrollUpShape;
    scrollUpShape.setFillColor(_verbUiColors.at(currentActorIndex).verbNormal);
    scrollUpShape.setPosition(scrollUpPosition);
    scrollUpShape.setSize(scrollUpSize);
    scrollUpShape.setTexture(&_gameSheet.getTexture());
    scrollUpShape.setTextureRect(rect);
    target.draw(scrollUpShape);
}

void Inventory::drawDownArrow(sf::RenderTarget &target) const
{
    int currentActorIndex = getCurrentActorIndex();
    auto scrollUpFrameRect = _gameSheet.getRect("scroll_up");
    sf::Vector2f scrollUpPosition(Screen::Width / 2.f, 580.f);
    sf::Vector2f scrollUpSize(scrollUpFrameRect.width, scrollUpFrameRect.height);

    auto scrollDownFrameRect = _gameSheet.getRect("scroll_down");
    sf::RectangleShape scrollDownShape;
    scrollDownShape.setFillColor(_verbUiColors.at(currentActorIndex).verbNormal);
    scrollDownShape.setPosition(scrollUpPosition.x, scrollUpPosition.y + scrollUpFrameRect.height);
    scrollDownShape.setSize(scrollUpSize);
    scrollDownShape.setTexture(&_gameSheet.getTexture());
    scrollDownShape.setTextureRect(scrollDownFrameRect);
    target.draw(scrollDownShape);
}

void Inventory::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    int currentActorIndex = getCurrentActorIndex();
    if (currentActorIndex == -1)
        return;

    const auto view = target.getView();
    target.setView(sf::View(sf::FloatRect(0, 0, Screen::Width, Screen::Height)));

    // inventory arrows
    auto scrollUpFrameRect = _gameSheet.getRect("scroll_up");
    sf::Vector2f scrollUpPosition(Screen::Width / 2.f,  Screen::Height - 3 *  Screen::Height / 14.f);
    sf::Vector2f scrollUpSize(scrollUpFrameRect.width, scrollUpFrameRect.height);

    sf::Color c(_verbUiColors.at(currentActorIndex).inventoryBackground);
    c.a = 128;

    auto inventoryRect = _gameSheet.getRect("inventory_background");
    sf::Vector2i sizeBack(inventoryRect.width, inventoryRect.height);
    sf::Sprite inventoryShape;
    inventoryShape.setColor(c);
    inventoryShape.setTexture(_gameSheet.getTexture());
    inventoryShape.setTextureRect(inventoryRect);
    auto gapX = 10.f;
    auto gapY = 10.f;
    for (auto i = 0; i < 8; i++)
    {
        auto x = (i % 4) * (sizeBack.x + gapX);
        auto y = (i / 4) * (sizeBack.y + gapY);
        inventoryShape.setPosition(sf::Vector2f(scrollUpPosition.x + scrollUpSize.x + x, y + Screen::Height - 3 *  Screen::Height / 14.f));
        target.draw(inventoryShape);
    }

    // draw inventory objects
    if (!_pCurrentActor)
        return;

    auto startX = sizeBack.x / 2.f + scrollUpPosition.x + scrollUpSize.x;
    auto startY = sizeBack.y / 2.f + Screen::Height - 3 *  Screen::Height / 14.f;

    auto x = 0, y = 0;
    int i = 0;
    auto &objects = _pCurrentActor->getObjects();
    for (const auto &object : objects)
    {
        auto icon = object->getIcon();
        auto rect = _inventoryItems.getRect(icon);
        auto spriteSourceSize = _inventoryItems.getSpriteSourceSize(icon);
        auto sourceSize = _inventoryItems.getSourceSize(icon);
        sf::Vector2f origin(sourceSize.x / 2.f - spriteSourceSize.left, sourceSize.y / 2.f - spriteSourceSize.top);

        sf::Sprite sprite;
        sprite.setOrigin(origin);
        sprite.setPosition(sf::Vector2f(x + startX, y + startY));
        sprite.setTexture(_inventoryItems.getTexture());
        sprite.setTextureRect(rect);
        sprite.scale(4, 4);
        target.draw(sprite);
        i++;
        if (i == 8)
            break;
        if ((i % 4) == 0)
        {
            x = 0;
            y += sourceSize.y;
        }
        else
        {
            x += sizeBack.x + 5;
        }
    }
    target.setView(view);
}

} // namespace ng
