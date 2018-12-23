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
#include "_NGUtil.h"

namespace ng
{
Engine::Engine(const EngineSettings &settings)
    : _settings(settings),
      _textureManager(settings),
      _fadeAlpha(255),
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
      _pVerb(nullptr),
      _dialogManager(*this),
      _soundManager(settings)
{
    time_t t;
    auto seed = (unsigned)time(&t);
    std::cout << "seed: " << seed << std::endl;
    srand(seed);

    _font.setTextureManager(&_textureManager);
    _font.setSettings(&_settings);
    _font.load("FontModernSheet");

    // load all messages
    std::string path(settings.getGamePath());
    path.append("ThimbleweedText_en.tsv");
    _textDb.load(path);

    _verbSheet.load("VerbSheet");
    _gameSheet.load("GameSheet");
    _inventoryItems.load("InventoryItems");

    _cursorRect = _gameSheet.getRect("cursor");
    _cursorLeftRect = _gameSheet.getRect("cursor_left");
    _cursorRightRect = _gameSheet.getRect("cursor_right");
    _cursorFrontRect = _gameSheet.getRect("cursor_front");
    _cursorBackRect = _gameSheet.getRect("cursor_back");
    _hotspotCursorRect = _gameSheet.getRect("hotspot_cursor");
    _hotspotCursorLeftRect = _gameSheet.getRect("hotspot_cursor_left");
    _hotspotCursorRightRect = _gameSheet.getRect("hotspot_cursor_right");
    _hotspotCursorFrontRect = _gameSheet.getRect("hotspot_cursor_front");
    _hotspotCursorBackRect = _gameSheet.getRect("hotspot_cursor_back");

    sf::Vector2f size(Screen::Width / 6.f, Screen::Height / 14.f);
    for (auto i = 0; i < 9; i++)
    {
        auto left = (i / 3) * size.x;
        auto top = Screen::Height - size.y * 3 + (i % 3) * size.y;
        _verbRects[i] = sf::IntRect(left, top, size.x, size.y);
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
    for (auto i = 0; i < 10; i++)
    {
        const auto &verb = _verbSlots[0].getVerb(i);
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

    sf::Mouse m;
    _mousePos = _pWindow->mapPixelToCoords(m.getPosition(*_pWindow));
    auto mousePosInRoom = _mousePos + _cameraPos;

    _pCurrentObject = nullptr;
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

    _dialogManager.update(elapsed);

    if (!m.isButtonPressed(sf::Mouse::Button::Left))
        return;

    if (_dialogManager.isActive())
        return;

    auto verbId = -1;
    for (auto i = 0; i < 9; i++)
    {
        if (_verbRects[i].contains((sf::Vector2i)_mousePos))
        {
            verbId = i;
            break;
        }
    }

    if (verbId != -1)
    {
        _pVerb = &_verbSlots[0].getVerb(1 + verbId);
        std::cout << "select verb: " << _pVerb->id << std::endl;
    }
    else if (_pVerb && _pVerb->id == "walkto" && !_pCurrentObject && _pCurrentActor)
    {
        _pCurrentActor->walkTo(mousePosInRoom);
    }
    else if (_pCurrentObject)
    {
        _pVerbExecute->execute(_pCurrentObject, _pVerb);
    }
    else
    {
        _pVerb = &_verbSlots[0].getVerb(0);
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
    }

    // draw fade
    sf::RectangleShape fadeShape;
    fadeShape.setSize(sf::Vector2f(Screen::Width, Screen::Height));
    fadeShape.setFillColor(sf::Color(0, 0, 0, _fadeAlpha));
    window.draw(fadeShape);

    drawCursor(window);
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
    shape.setTextureRect(_pCurrentObject ? _hotspotCursorRect : _cursorRect);
    window.draw(shape);

    if (_pCurrentObject)
    {
        NGText text;
        text.setFont(_font);
        text.setPosition((sf::Vector2f)_mousePos - sf::Vector2f(0, 18));
        text.setColor(sf::Color::White);
        text.setText(_pCurrentObject->getName());
        window.draw(text, sf::RenderStates::Default);

        sf::RenderStates states;
        states.transform.translate(-_cameraPos);
        _pCurrentObject->drawHotspot(window, states);
    }
}

void Engine::drawVerbs(sf::RenderWindow &window) const
{
    if (!_inputActive || _verbSlots[0].getVerb(0).id.empty())
        return;

    auto verbId = -1;
    if (_pCurrentObject)
    {
        auto defaultVerb = _pCurrentObject->getDefaultVerb();
        if (!defaultVerb.empty())
        {
            verbId = _verbSlots[0].getVerbIndex(defaultVerb);
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
            auto verb = _verbSlots[0].getVerb(x * 3 + y + 1);
            auto rect = getVerbRect(verb.id);
            maxW = fmax(maxW, rect.width * ratio.x);
        }
        auto padding = (size.x - maxW) / 2.f;
        auto left = padding + x * size.x;

        for (int y = 0; y < 3; y++)
        {
            auto top = Screen::Height - size.y * 3 + y * size.y;
            auto verb = _verbSlots[0].getVerb(x * 3 + y + 1);
            auto rect = getVerbRect(verb.id);
            auto verbSize = sf::Vector2f(rect.width * ratio.x, rect.height * ratio.y);
            int index = x * 3 + y;
            auto color = index == verbId ? _verbUiColors[0].verbHighlight : _verbUiColors[0].verbNormalTint;
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
    if (!_inputActive)
        return;

    auto ratio = sf::Vector2f(Screen::Width / 1280.f, Screen::Height / 720.f);

    // inventory arrows
    auto scrollUpFrameRect = _gameSheet.getRect("scroll_up");
    // sf::RectangleShape scrollUpShape;
    sf::Vector2f scrollUpPosition(Screen::Width / 2.f, Screen::Height - 3 * Screen::Height / 14.f);
    sf::Vector2f scrollUpSize(scrollUpFrameRect.width * ratio.x, scrollUpFrameRect.height * ratio.y);
    // scrollUpShape.setFillColor(_verbUiColors[0].verbNormal);
    // scrollUpShape.setPosition(scrollUpPosition);
    // scrollUpShape.setSize(scrollUpSize);
    // scrollUpShape.setTexture(&_gameSheet.getTexture());
    // scrollUpShape.setTextureRect(scrollUpFrameRect);
    // window.draw(scrollUpShape);

    // auto scrollDownFrameRect = _gameSheet.getRect("scroll_down");
    // sf::RectangleShape scrollDownShape;
    // scrollDownShape.setFillColor(_verbUiColors[0].verbNormal);
    // scrollDownShape.setPosition(scrollUpPosition.x, scrollUpPosition.y + scrollUpFrameRect.height * ratio.y);
    // scrollDownShape.setSize(scrollUpSize);
    // scrollDownShape.setTexture(&_gameSheet.getTexture());
    // scrollDownShape.setTextureRect(scrollDownFrameRect);
    // window.draw(scrollDownShape);

    // inventory frame
    // auto inventoryFrameRect = _gameSheet.getRect("inventory_frame");
    // sf::RectangleShape inventoryShape;
    // inventoryShape.setFillColor(_verbUiColors[0].inventoryFrame);
    // inventoryShape.setPosition(sf::Vector2f(scrollUpPosition.x + scrollUpSize.x, Screen::Height - 3 * Screen::Height / 14.f));
    // inventoryShape.setSize(sf::Vector2f(Screen::Width / 2.f - scrollUpSize.x, 3 * Screen::Height / 14.f));
    // inventoryShape.setTexture(&_gameSheet.getTexture());
    // inventoryShape.setTextureRect(inventoryFrameRect);
    // target.draw(inventoryShape);

    auto inventoryFrameRect = _gameSheet.getRect("inventory_background");
    sf::RectangleShape inventoryShape;
    sf::Color c(_verbUiColors[0].inventoryBackground);
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
        for (auto it = objects.begin(); it != objects.end(); it++)
        {
            auto rect = _inventoryItems.getRect(*it);
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
} // namespace ng
