#include "Inventory.h"
#include "Engine.h"

namespace ng
{
Inventory::Inventory(Engine &engine,
                     std::array<ActorIconSlot, 6> &actorsIconSlots,
                     std::array<VerbUiColors, 6> &verbUiColors,
                     Actor *&pCurrentActor)
    : _engine(engine),
      _actorsIconSlots(actorsIconSlots),
      _verbUiColors(verbUiColors),
      _gameSheet(engine.getTextureManager(), engine.getSettings()),
      _inventoryItems(engine.getTextureManager(), engine.getSettings()),
      _pCurrentActor(pCurrentActor),
      _pCurrentInventoryObject(nullptr)
{
    _gameSheet.load("GameSheet");
    _inventoryItems.load("InventoryItems");

    // inventory rects
    auto x = 0, y = 0;
    auto ratio = sf::Vector2f(Screen::Width / 1280.f, Screen::Height / 720.f);
    auto scrollUpFrameRect = _gameSheet.getRect("scroll_up");
    sf::Vector2f scrollUpPosition(Screen::Width / 2.f, Screen::Height - 3 * Screen::Height / 14.f);
    sf::Vector2f scrollUpSize(scrollUpFrameRect.width * ratio.x, scrollUpFrameRect.height * ratio.y);
    for (auto i = 0; i < 8; i++)
    {
        sf::Vector2f pos(x + scrollUpPosition.x + scrollUpSize.x, y + Screen::Height - 3 * Screen::Height / 14.f);
        auto size = sf::Vector2f(206.f * Screen::Width / 1920.f, 112.f * Screen::Height / 1080.f);
        _inventoryRects[i] = sf::IntRect(pos.x, pos.y, size.x, size.y);
        x += size.x;
        if (i == 3)
        {
            x = 0;
            y += size.y;
        }
    }
}

void Inventory::update(const sf::Time &elapsed)
{
    _pCurrentInventoryObject = nullptr;

    for (int i = 0; i < 8; i++)
    {
        const auto &r = _inventoryRects[i];
        if (r.contains((sf::Vector2i)_mousePos))
        {
            auto &objects = _pCurrentActor->getObjects();
            if (i < objects.size())
            {
                _pCurrentInventoryObject = objects[i].get();
            }
        }
    }
}

int Inventory::getCurrentActorIndex() const
{
    for (auto i = 0; i < _actorsIconSlots.size(); i++)
    {
        const auto &selectableActor = _actorsIconSlots[i];
        if (selectableActor.pActor == _pCurrentActor)
        {
            return i;
        }
    }
    return -1;
}

void Inventory::drawUpArrow(sf::RenderTarget &target) const
{
    auto ratio = sf::Vector2f(Screen::Width / 1280.f, Screen::Height / 720.f);

    int currentActorIndex = getCurrentActorIndex();
    auto rect = _gameSheet.getRect("scroll_up");

    sf::Vector2f scrollUpSize(rect.width * ratio.x, rect.height * ratio.y);
    sf::Vector2f scrollUpPosition(Screen::Width / 2.f, Screen::Height - 3 * Screen::Height / 14.f);
    sf::RectangleShape scrollUpShape;
    scrollUpShape.setFillColor(_verbUiColors[currentActorIndex].verbNormal);
    scrollUpShape.setPosition(scrollUpPosition);
    scrollUpShape.setSize(scrollUpSize);
    scrollUpShape.setTexture(&_gameSheet.getTexture());
    scrollUpShape.setTextureRect(rect);
    target.draw(scrollUpShape);
}

void Inventory::drawDownArrow(sf::RenderTarget &target) const
{
    auto ratio = sf::Vector2f(Screen::Width / 1280.f, Screen::Height / 768.f);

    int currentActorIndex = getCurrentActorIndex();
    auto scrollUpFrameRect = _gameSheet.getRect("scroll_up");
    sf::Vector2f scrollUpPosition(Screen::Width / 2.f, Screen::Height - 3 * Screen::Height / 14.f);
    sf::Vector2f scrollUpSize(scrollUpFrameRect.width * ratio.x, scrollUpFrameRect.height * ratio.y);

    auto scrollDownFrameRect = _gameSheet.getRect("scroll_down");
    sf::RectangleShape scrollDownShape;
    scrollDownShape.setFillColor(_verbUiColors[currentActorIndex].verbNormal);
    scrollDownShape.setPosition(scrollUpPosition.x, scrollUpPosition.y + scrollUpFrameRect.height * ratio.y);
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

    auto ratio = sf::Vector2f(Screen::Width / 1280.f, Screen::Height / 720.f);

    // inventory arrows
    auto scrollUpFrameRect = _gameSheet.getRect("scroll_up");
    sf::Vector2f scrollUpPosition(Screen::Width / 2.f, Screen::Height - 3 * Screen::Height / 14.f);
    sf::Vector2f scrollUpSize(scrollUpFrameRect.width * ratio.x, scrollUpFrameRect.height * ratio.y);

    auto inventoryFrameRect = _gameSheet.getRect("inventory_background");
    sf::RectangleShape inventoryShape;
    sf::Color c(_verbUiColors[currentActorIndex].inventoryBackground);
    c.a = 128;
    inventoryShape.setFillColor(c);
    inventoryShape.setTexture(&_gameSheet.getTexture());
    inventoryShape.setTextureRect(inventoryFrameRect);
    auto sizeBack = sf::Vector2f(206.f * Screen::Width / 1920.f, 112.f * Screen::Height / 1080.f);
    inventoryShape.setSize(sizeBack);
    auto gapX = 10.f * Screen::Width / 1920.f;
    auto gapY = 10.f * Screen::Height / 1080.f;
    for (auto i = 0; i < 8; i++)
    {
        auto x = (i % 4) * (sizeBack.x + gapX);
        auto y = (i / 4) * (sizeBack.y + gapY);
        inventoryShape.setPosition(sf::Vector2f(scrollUpPosition.x + scrollUpSize.x + x, y + Screen::Height - 3 * Screen::Height / 14.f));
        target.draw(inventoryShape);
    }

    // draw inventory objects
    if (!_pCurrentActor)
        return;

    auto startX = sizeBack.x / 2.f + scrollUpPosition.x + scrollUpSize.x;
    auto startY = sizeBack.y / 2.f + Screen::Height - 3 * Screen::Height / 14.f;

    auto x = 0, y = 0;
    int i = 0;
    auto &objects = _pCurrentActor->getObjects();
    for (const auto &object : objects)
    {
        auto icon = object->getIcon();
        auto rect = _inventoryItems.getRect(icon);
        auto spriteSourceSize = _inventoryItems.getSpriteSourceSize(icon);
        auto sourceSize = _inventoryItems.getSourceSize(icon);
        sf::Vector2f origin(-sourceSize.x / 2.f + spriteSourceSize.left, -sourceSize.y / 2.f + spriteSourceSize.top);

        sf::RectangleShape inventoryShape;
        inventoryShape.setOrigin(-origin);
        inventoryShape.setPosition(sf::Vector2f(x + startX, y + startY));
        inventoryShape.setSize(sf::Vector2f(rect.width, rect.height));
        inventoryShape.setTexture(&_inventoryItems.getTexture());
        inventoryShape.setTextureRect(rect);
        target.draw(inventoryShape);
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
            x += sourceSize.x;
        }
    }
}
} // namespace ng
