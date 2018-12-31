#include <memory>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <math.h>
#include "Engine.h"
#include "Screen.h"
#include "Font.h"
#include "Text.h"
#include "_NGUtil.h"

namespace ng
{
CursorDirection operator|=(CursorDirection &lhs, CursorDirection rhs)
{
    lhs = static_cast<CursorDirection>(
        static_cast<std::underlying_type<CursorDirection>::type>(lhs) |
        static_cast<std::underlying_type<CursorDirection>::type>(rhs));
    return lhs;
}

bool operator&(CursorDirection lhs, CursorDirection rhs)
{
    return static_cast<CursorDirection>(
               static_cast<std::underlying_type<CursorDirection>::type>(lhs) &
               static_cast<std::underlying_type<CursorDirection>::type>(rhs)) > CursorDirection::None;
}

Engine::Engine(const EngineSettings &settings)
    : _settings(settings),
      _textureManager(settings),
      _fadeColor(0, 0, 0, 255),
      _pWindow(nullptr),
      _pRoom(nullptr),
      _pCurrentActor(nullptr),
      _verbSheet(_textureManager, settings),
      _gameSheet(_textureManager, settings),
      _inventoryItems(_textureManager, settings),
      _inputActive(false),
      _showCursor(false),
      _pFollowActor(nullptr),
      _pCurrentObject(nullptr),
      _pCurrentInventoryObject(nullptr),
      _pVerb(nullptr),
      _dialogManager(*this),
      _soundManager(settings),
      _useFlag(UseFlag::None),
      _pUseObject(nullptr),
      _cursorDirection(CursorDirection::None)
{
    time_t t;
    auto seed = (unsigned)time(&t);
    std::cout << "seed: " << seed << std::endl;
    srand(seed);

    std::string path(settings.getGamePath());
    path.append("SentenceFont.fnt");
    _fntFont.loadFromFile(path);

    // load all messages
    path = settings.getGamePath();
    path.append("ThimbleweedText_en.tsv");
    _textDb.load(path);

    _verbSheet.load("VerbSheet");
    _gameSheet.load("GameSheet");
    _inventoryItems.load("InventoryItems");

    sf::Vector2f size(Screen::Width / 6.f, Screen::Height / 14.f);
    for (auto i = 0; i < 9; i++)
    {
        auto left = (i / 3) * size.x;
        auto top = Screen::Height - size.y * 3 + (i % 3) * size.y;
        _verbRects[i] = sf::IntRect(left, top, size.x, size.y);
    }

    // inventory
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

Engine::~Engine() = default;

sf::IntRect Engine::getVerbRect(const std::string &name, std::string lang, bool isRetro) const
{
    std::ostringstream s;
    s << name << (isRetro ? "_retro" : "") << "_" << lang;
    return _verbSheet.getRect(s.str());
}

const Verb *Engine::getVerb(const std::string &id) const
{
    auto index = getCurrentActorIndex();
    for (auto i = 0; i < 10; i++)
    {
        const auto &verb = _verbSlots[index].getVerb(i);
        if (verb.id == id)
        {
            return &verb;
            break;
        }
    }
    return nullptr;
}

void Engine::setCameraAt(const sf::Vector2f &at)
{
    _cameraPos = at;
}

void Engine::moveCamera(const sf::Vector2f &offset)
{
    _cameraPos += offset;
    clampCamera();
}

void Engine::clampCamera()
{
    if (_cameraPos.x < 0)
        _cameraPos.x = 0;
    if (_cameraPos.y < 0)
        _cameraPos.y = 0;
    if (!_pRoom)
        return;
    const auto &size = _pRoom->getRoomSize();
    if (_cameraPos.x > size.x - Screen::Width)
        _cameraPos.x = size.x - Screen::Width;
    if (_cameraPos.y > size.y - Screen::Height)
        _cameraPos.y = size.y - Screen::Height;
}

void Engine::update(const sf::Time &elapsed)
{
    for (auto &function : _newFunctions)
    {
        _functions.push_back(std::move(function));
    }
    _newFunctions.clear();
    for (auto &function : _functions)
    {
        (*function)();
    }
    _functions.erase(std::remove_if(_functions.begin(), _functions.end(),
                                    [](std::unique_ptr<Function> &f) { return f->isElapsed(); }),
                     _functions.end());
    for (auto &actor : _actors)
    {
        actor->update(elapsed);
    }

    if (!_pRoom)
        return;

    _pRoom->update(elapsed);
    if (_pFollowActor)
    {
        auto pos = _pFollowActor->getPosition();
        _cameraPos = pos - sf::Vector2f(Screen::HalfWidth, Screen::HalfHeight);
        clampCamera();
    }

    _mousePos = _pWindow->mapPixelToCoords(sf::Mouse::getPosition(*_pWindow));
    if (_pRoom)
    {
        _cursorDirection = CursorDirection::None;
        if (_mousePos.x < 20)
            _cursorDirection |= CursorDirection::Left;
        else if (_mousePos.x > Screen::Width - 20)
            _cursorDirection |= CursorDirection::Right;
        if (_mousePos.y < 10)
            _cursorDirection |= CursorDirection::Up;
        else if (_mousePos.y > Screen::Height - 10)
            _cursorDirection |= CursorDirection::Down;
        if (_pCurrentObject)
            _cursorDirection |= CursorDirection::Hotspot;
    }
    auto mousePosInRoom = _mousePos + _cameraPos;

    _pCurrentObject = nullptr;
    _pCurrentInventoryObject = nullptr;
    const auto &objects = _pRoom->getObjects();
    auto it = std::find_if(objects.cbegin(), objects.cend(), [mousePosInRoom](const std::unique_ptr<Object> &pObj) {
        if (!pObj->isTouchable())
            return false;
        auto rect = pObj->getRealHotspot();
        return rect.contains((sf::Vector2i)mousePosInRoom);
    });
    if (it != objects.cend())
    {
        _pCurrentObject = it->get();
    }
    else
    {
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

    _dialogManager.update(elapsed);

    if (!sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
        return;

    if (_dialogManager.isActive())
        return;

    sf::FloatRect iconRect(Screen::Width - 16, 15, 16, 16);
    for (auto i = 0; i < _actorsIconSlots.size(); i++)
    {
        const auto &selectableActor = _actorsIconSlots[i];
        if (!selectableActor.selectable || !selectableActor.pActor || selectableActor.pActor == _pCurrentActor)
            continue;

        if (iconRect.contains(_mousePos))
        {
            _pCurrentActor = selectableActor.pActor;
            setCameraAt(selectableActor.pActor->getUsePosition());
            follow(selectableActor.pActor);
            return;
        }
        iconRect.top += 15;
    }

    auto verbId = -1;
    for (auto i = 0; i < 9; i++)
    {
        if (_verbRects[i].contains((sf::Vector2i)_mousePos))
        {
            verbId = i;
            break;
        }
    }

    int currentActorIndex = getCurrentActorIndex();
    if (verbId != -1)
    {
        _pVerb = &_verbSlots[currentActorIndex].getVerb(1 + verbId);
        _useFlag = UseFlag::None;
        _pUseObject = nullptr;
        std::cout << "select verb: " << _pVerb->id << std::endl;
    }
    else if (_pVerb && _pVerb->id == "walkto" && !_pCurrentObject && _pCurrentActor)
    {
        _pCurrentActor->walkTo(mousePosInRoom);
    }
    else if (_pCurrentObject)
    {
        if (_pUseObject)
        {
            _pVerbExecute->use(_pUseObject, _pCurrentObject);
        }
        else
        {
            _pVerbExecute->execute(_pCurrentObject, _pVerb);
        }
    }
    else if (_pCurrentInventoryObject)
    {
        _pVerbExecute->execute(_pCurrentInventoryObject, _pVerb);
    }
    else
    {
        _pVerb = &_verbSlots[currentActorIndex].getVerb(0);
        _useFlag = UseFlag::None;
        _pUseObject = nullptr;
    }
}

void Engine::draw(sf::RenderWindow &window) const
{
    if (!_pRoom)
        return;

    _pRoom->draw(window, _cameraPos);

    window.draw(_dialogManager);

    if (!_dialogManager.isActive())
    {
        drawVerbs(window);
        drawInventory(window);
        drawActorIcons(window);
    }

    drawFade(window);
    drawCursor(window);
    drawCursorText(window);
}

void Engine::drawFade(sf::RenderWindow &window) const
{
    sf::RectangleShape fadeShape;
    fadeShape.setSize(sf::Vector2f(Screen::Width, Screen::Height));
    fadeShape.setFillColor(_fadeColor);
    window.draw(fadeShape);
}

void Engine::drawCursor(sf::RenderWindow &window) const
{
    if (!_inputActive)
        return;

    auto cursorSize = sf::Vector2f(68.f * Screen::Width / 1284, 68.f * Screen::Height / 772);
    sf::RectangleShape shape;
    shape.setPosition(_mousePos);
    shape.setOrigin(cursorSize / 2.f);
    shape.setSize(cursorSize);
    shape.setTexture(&_gameSheet.getTexture());

    shape.setTextureRect(getCursorRect());
    window.draw(shape);
}

sf::IntRect Engine::getCursorRect() const
{
    const auto &size = _pRoom->getRoomSize();
    if (_cursorDirection & CursorDirection::Left && _cameraPos.x > 0)
    {
        return _cursorDirection & CursorDirection::Hotspot ? _gameSheet.getRect("hotspot_cursor_left") : _gameSheet.getRect("cursor_left");
    }
    if (_cursorDirection & CursorDirection::Right && _cameraPos.x < size.x - Screen::Width)
    {
        return _cursorDirection & CursorDirection::Hotspot ? _gameSheet.getRect("hotspot_cursor_right") : _gameSheet.getRect("cursor_right");
    }
    if (_cursorDirection & CursorDirection::Up && _cameraPos.y > 0)
    {
        return _cursorDirection & CursorDirection::Hotspot ? _gameSheet.getRect("hotspot_cursor_back") : _gameSheet.getRect("cursor_back");
    }
    if (_cursorDirection & CursorDirection::Down && _cameraPos.y < size.y - Screen::Height)
    {
        return _cursorDirection & CursorDirection::Hotspot ? _gameSheet.getRect("hotspot_cursor_front") : _gameSheet.getRect("cursor_front");
    }
    return _cursorDirection & CursorDirection::Hotspot ? _gameSheet.getRect("hotspot_cursor") : _gameSheet.getRect("cursor");
}

void Engine::drawCursorText(sf::RenderWindow &window) const
{
    if (!_inputActive)
        return;

    auto scale = Screen::HalfHeight / 512.f;
    if (_pCurrentObject)
    {
        Text text;
        text.setFont(_fntFont);
        text.setScale(scale, scale);
        text.setFillColor(sf::Color::White);
        std::string s;
        if (_pVerb && !_pVerb->text.empty())
        {
            auto id = std::strtol(_pVerb->text.substr(1).data(), nullptr, 10);
            s.append(getText(id));
        }
        else
        {
            // Walk to
            s.append(getText(30011));
        }
        if (_pUseObject)
        {
            s.append(" ").append(_pUseObject->getName());
        }
        else
        {
            s.append(" ").append(_pCurrentObject->getName());
        }
        appendUseFlag(s);
        if (_pUseObject)
        {
            s.append(" ").append(_pCurrentObject->getName());
        }
        text.setString(s);
        auto offset = sf::Vector2f(text.getGlobalBounds().width / 2.f, 0);
        text.setPosition((sf::Vector2f)_mousePos - sf::Vector2f(0, 22) - offset);
        window.draw(text, sf::RenderStates::Default);

        sf::RenderStates states;
        states.transform.translate(-_cameraPos);
        _pCurrentObject->drawHotspot(window, states);
    }
    if (_pCurrentInventoryObject && _pVerb)
    {
        Text text;
        text.setFont(_fntFont);
        text.setScale(scale, scale);
        text.setFillColor(sf::Color::White);
        std::string s;
        if (_pVerb)
        {
            auto id = std::strtol(_pVerb->text.substr(1).data(), nullptr, 10);
            s.append(getText(id));
        }
        s.append(" ").append(_pCurrentInventoryObject->getName());
        appendUseFlag(s);
        text.setString(s);
        auto offset = sf::Vector2f(text.getGlobalBounds().width / 2.f, 0);
        text.setPosition((sf::Vector2f)_mousePos - sf::Vector2f(0, 22) - offset);
        window.draw(text, sf::RenderStates::Default);
    }
}

void Engine::appendUseFlag(std::string &sentence) const
{
    switch (_useFlag)
    {
    case UseFlag::UseWith:
        sentence.append(" ").append(getText(10000));
        break;
    case UseFlag::UseIn:
        sentence.append(" ").append(getText(10002));
        break;
    case UseFlag::UseOn:
        sentence.append(" ").append(getText(10001));
        break;
    case UseFlag::None:
        break;
    }
}

int Engine::getCurrentActorIndex() const
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

void Engine::drawVerbs(sf::RenderWindow &window) const
{
    int currentActorIndex = getCurrentActorIndex();
    if (!_inputActive || currentActorIndex == -1 || _verbSlots[currentActorIndex].getVerb(0).id.empty())
        return;

    auto verbId = -1;
    if (_pCurrentObject)
    {
        auto defaultVerb = _pCurrentObject->getDefaultVerb();
        if (!defaultVerb.empty())
        {
            verbId = _verbSlots[currentActorIndex].getVerbIndex(defaultVerb);
        }
    }
    else
    {
        for (auto i = 0; i < 9; i++)
        {
            if (_verbRects[i].contains((sf::Vector2i)_mousePos))
            {
                verbId = i;
                break;
            }
        }
    }

    sf::RenderStates states;
    states.texture = &_verbSheet.getTexture();

    sf::Vector2f size(Screen::Width / 6.f, Screen::Height / 14.f);
    auto ratio = sf::Vector2f(Screen::Width / 1280.f, Screen::Height / 720.f);
    for (int x = 0; x < 3; x++)
    {
        auto maxW = 0;
        for (int y = 0; y < 3; y++)
        {
            auto verb = _verbSlots[currentActorIndex].getVerb(x * 3 + y + 1);
            auto rect = getVerbRect(verb.id);
            maxW = fmax(maxW, rect.width * ratio.x);
        }
        auto padding = (size.x - maxW) / 2.f;
        auto left = padding + x * size.x;

        for (int y = 0; y < 3; y++)
        {
            auto top = Screen::Height - size.y * 3 + y * size.y;
            auto verb = _verbSlots[currentActorIndex].getVerb(x * 3 + y + 1);
            auto rect = getVerbRect(verb.id);
            auto verbSize = sf::Vector2f(rect.width * ratio.x, rect.height * ratio.y);
            int index = x * 3 + y;
            auto color = index == verbId ? _verbUiColors[currentActorIndex].verbHighlight : _verbUiColors[currentActorIndex].verbNormalTint;
            sf::RectangleShape verbShape;
            verbShape.setFillColor(color);
            verbShape.setPosition(left, top);
            verbShape.setSize(verbSize);
            verbShape.setTexture(&_verbSheet.getTexture());
            verbShape.setTextureRect(rect);
            window.draw(verbShape);
        }
    }
}

void Engine::drawInventory(sf::RenderWindow &window) const
{
    int currentActorIndex = getCurrentActorIndex();
    if (!_inputActive || currentActorIndex == -1)
        return;

    auto ratio = sf::Vector2f(Screen::Width / 1280.f, Screen::Height / 720.f);

    // inventory arrows
    auto scrollUpFrameRect = _gameSheet.getRect("scroll_up");
    // sf::RectangleShape scrollUpShape;
    sf::Vector2f scrollUpPosition(Screen::Width / 2.f, Screen::Height - 3 * Screen::Height / 14.f);
    sf::Vector2f scrollUpSize(scrollUpFrameRect.width * ratio.x, scrollUpFrameRect.height * ratio.y);
    // scrollUpShape.setFillColor(_verbUiColors[currentActorIndex].verbNormal);
    // scrollUpShape.setPosition(scrollUpPosition);
    // scrollUpShape.setSize(scrollUpSize);
    // scrollUpShape.setTexture(&_gameSheet.getTexture());
    // scrollUpShape.setTextureRect(scrollUpFrameRect);
    // window.draw(scrollUpShape);

    // auto scrollDownFrameRect = _gameSheet.getRect("scroll_down");
    // sf::RectangleShape scrollDownShape;
    // scrollDownShape.setFillColor(_verbUiColors[currentActorIndex].verbNormal);
    // scrollDownShape.setPosition(scrollUpPosition.x, scrollUpPosition.y + scrollUpFrameRect.height * ratio.y);
    // scrollDownShape.setSize(scrollUpSize);
    // scrollDownShape.setTexture(&_gameSheet.getTexture());
    // scrollDownShape.setTextureRect(scrollDownFrameRect);
    // window.draw(scrollDownShape);

    // inventory frame
    // auto inventoryFrameRect = _gameSheet.getRect("inventory_frame");
    // sf::RectangleShape inventoryShape;
    // inventoryShape.setFillColor(_verbUiColors[currentActorIndex].inventoryFrame);
    // inventoryShape.setPosition(sf::Vector2f(scrollUpPosition.x + scrollUpSize.x, Screen::Height - 3 * Screen::Height / 14.f));
    // inventoryShape.setSize(sf::Vector2f(Screen::Width / 2.f - scrollUpSize.x, 3 * Screen::Height / 14.f));
    // inventoryShape.setTexture(&_gameSheet.getTexture());
    // inventoryShape.setTextureRect(inventoryFrameRect);
    // target.draw(inventoryShape);

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
        window.draw(inventoryShape);
    }

    // draw inventory objects
    if (_pCurrentActor)
    {
        auto x = 0;
        auto &objects = _pCurrentActor->getObjects();
        for (const auto &object : objects)
        {
            auto rect = _inventoryItems.getRect(object->getIcon());
            sf::RectangleShape inventoryShape;
            inventoryShape.setPosition(sf::Vector2f(x + scrollUpPosition.x + scrollUpSize.x, Screen::Height - 3 * Screen::Height / 14.f));
            inventoryShape.setSize(sf::Vector2f(rect.width, rect.height));
            inventoryShape.setTexture(&_inventoryItems.getTexture());
            inventoryShape.setTextureRect(rect);
            window.draw(inventoryShape);
            x += rect.width;
        }
    }
}

bool Engine::isThreadAlive(HSQUIRRELVM thread) const
{
    return std::find(_threads.begin(), _threads.end(), thread) != _threads.end();
}

void Engine::startDialog(const std::string &dialog)
{
    _dialogManager.start(dialog);
}

void Engine::execute(const std::string &code)
{
    _pScriptExecute->execute(code);
}

bool Engine::executeCondition(const std::string &code)
{
    return _pScriptExecute->executeCondition(code);
}

void Engine::stopThread(HSQUIRRELVM thread)
{
    auto it = std::find(_threads.begin(), _threads.end(), thread);
    if (it == _threads.end())
        return;
    _threads.erase(it);
}

void Engine::addSelectableActor(int index, Actor *pActor)
{
    _actorsIconSlots[index - 1].selectable = true;
    _actorsIconSlots[index - 1].pActor = pActor;
}

void Engine::drawActorIcon(sf::RenderWindow &window, const std::string &icon, int actorSlot, const sf::Vector2f &offset, sf::Uint8 alpha) const
{
    const auto &colors = _verbUiColors[actorSlot];
    drawActorIcon(window, icon, colors.inventoryBackground, colors.inventoryFrame, offset, alpha);
}

void Engine::drawActorIcon(sf::RenderWindow &window, const std::string &icon, sf::Color backColor, sf::Color frameColor, const sf::Vector2f &offset, sf::Uint8 alpha) const
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
    window.draw(s, states);

    auto rect = _gameSheet.getRect(icon);
    auto spriteSourceSize = _gameSheet.getSpriteSourceSize(icon);
    auto sourceSize = _gameSheet.getSourceSize(icon);
    pos = sf::Vector2f(-sourceSize.x / 2.f + spriteSourceSize.left, -sourceSize.y / 2.f + spriteSourceSize.top);
    s.setOrigin(-pos);
    c = sf::Color::White;
    c.a = alpha;
    s.setColor(c);
    s.setTextureRect(rect);
    window.draw(s, states);

    pos = sf::Vector2f(-frameSourceSize.x / 2.f + frameSpriteSourceSize.left, -frameSourceSize.y / 2.f + frameSpriteSourceSize.top);
    s.setOrigin(-pos);
    c = frameColor;
    c.a = alpha;
    s.setColor(c);
    s.setTextureRect(frameRect);
    window.draw(s, states);
}

void Engine::drawActorIcons(sf::RenderWindow &window) const
{
    if (!_pCurrentActor)
        return;

    sf::Vector2f offset(Screen::Width - 8, 8);

    if (_pCurrentActor)
    {
        sf::FloatRect iconRect(Screen::Width - 16, 0, 16, 16);
        sf::Uint8 alpha = iconRect.contains(_mousePos) ? 0xFF : 0x20;

        auto i = getCurrentActorIndex();
        const auto &icon = _actorsIconSlots[i].pActor->getIcon();

        drawActorIcon(window, icon, i, offset, alpha);
        offset.y += 15;
    }

    for (auto i = 0; i < _actorsIconSlots.size(); i++)
    {
        const auto &selectableActor = _actorsIconSlots[i];
        if (!selectableActor.selectable || !selectableActor.pActor || selectableActor.pActor == _pCurrentActor)
            continue;

        const auto &icon = selectableActor.pActor->getIcon();
        drawActorIcon(window, icon, i, offset, 0xFF);
        offset.y += 15;
    }

    drawActorIcon(window, "icon_gear", sf::Color::Black, sf::Color(128, 128, 128), offset, 0xFF);
}

void Engine::actorSlotSelectable(Actor *pActor, bool selectable)
{
    for (auto &selectableActor : _actorsIconSlots)
    {
        if (selectableActor.pActor == pActor)
        {
            selectableActor.selectable = selectable;
            return;
        }
    }
}

} // namespace ng
