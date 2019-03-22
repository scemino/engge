#include <memory>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <math.h>
#include "Cutscene.h"
#include "Engine.h"
#include "Font.h"
#include "InventoryObject.h"
#include "Screen.h"
#include "Text.h"
#include "Verb.h"
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

Engine::Engine(EngineSettings &settings)
    : _settings(settings),
      _textureManager(settings),
      _fadeColor(0, 0, 0, 255),
      _pWindow(nullptr),
      _pRoom(nullptr),
      _pCutscene(nullptr),
      _pCurrentActor(nullptr),
      _verbSheet(_textureManager, settings),
      _gameSheet(_textureManager, settings),
      _inputActive(false),
      _showCursor(false),
      _inputVerbsActive(false),
      _pFollowActor(nullptr),
      _pCurrentObject(nullptr),
      _pVerb(nullptr),
      _dialogManager(*this),
      _soundManager(settings),
      _useFlag(UseFlag::None),
      _pUseObject(nullptr),
      _cursorDirection(CursorDirection::None),
      _actorIcons(*this, _actorsIconSlots, _verbUiColors, _pCurrentActor),
      _inventory(*this, _actorsIconSlots, _verbUiColors, _pCurrentActor)
{
    time_t t;
    auto seed = (unsigned)time(&t);
    std::cout << "seed: " << seed << std::endl;
    srand(seed);

    _fntFont.setTextureManager(&_textureManager);
    _fntFont.setSettings(&settings);
    _fntFont.load("FontModernSheet");

    // load all messages
    _textDb.setSettings(settings);
    _textDb.load("ThimbleweedText_en.tsv");

    _verbSheet.load("VerbSheet");
    _gameSheet.load("GameSheet");

    sf::Vector2f size(Screen::Width / 6.f, Screen::Height / 14.f);
    for (auto i = 0; i < 9; i++)
    {
        auto left = (i / 3) * size.x;
        auto top = Screen::Height - size.y * 3 + (i % 3) * size.y;
        _verbRects[i] = sf::IntRect(left, top, size.x, size.y);
    }
}

Engine::~Engine() = default;

void Engine::setRoom(Room *room)
{
    _fadeColor = sf::Color::Transparent;
    _pRoom = room;
}

void Engine::setInputActive(bool active)
{
    _inputActive = active;
    _showCursor = active;
}

void Engine::inputSilentOff()
{
    _inputActive = false;
}

void Engine::setInputVerbs(bool on)
{
    _inputVerbsActive = on;
}

sf::IntRect Engine::getVerbRect(int id, std::string lang, bool isRetro) const
{
    std::string s;
    std::string name;
    switch (id)
    {
    case 1:
        name = "walkto";
        break;
    case 2:
        name = "lookat";
        break;
    case 3:
        name = "talkto";
        break;
    case 4:
        name = "pickup";
        break;
    case 5:
        name = "open";
        break;
    case 6:
        name = "close";
        break;
    case 7:
        name = "push";
        break;
    case 8:
        name = "pull";
        break;
    case 9:
        name = "give";
        break;
    case 10:
        name = "use";
        break;
    }
    s.append(name).append(isRetro ? "_retro" : "").append("_").append(lang);
    return _verbSheet.getRect(s);
}

const Verb *Engine::getVerb(int id) const
{
    auto index = getCurrentActorIndex();
    for (auto i = 0; i < 10; i++)
    {
        const auto &verb = _verbSlots[index].getVerb(i);
        if (verb.id == id)
        {
            return &verb;
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

void Engine::updateCutscene(const sf::Time &elapsed)
{
    if (_pCutscene)
    {
        (*_pCutscene)(elapsed);
        if (_pCutscene->isElapsed())
        {
            _pCutscene.release();
        }
    }
}

void Engine::updateFunctions(const sf::Time &elapsed)
{
    for (auto &function : _newFunctions)
    {
        _functions.push_back(std::move(function));
    }
    _newFunctions.clear();
    for (auto &function : _functions)
    {
        (*function)(elapsed);
    }
    _functions.erase(std::remove_if(_functions.begin(), _functions.end(),
                                    [](std::unique_ptr<Function> &f) { return f->isElapsed(); }),
                     _functions.end());
    for (auto &actor : _actors)
    {
        actor->update(elapsed);
    }
}

void Engine::updateActorIcons(const sf::Time &elapsed)
{
    _actorIcons.setMousePosition(_mousePos);
    _actorIcons.update(elapsed);
}

void Engine::updateMouseCursor()
{
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

void Engine::updateCurrentObject(const sf::Vector2f& mousPos)
{
    _pCurrentObject = nullptr;
    const auto &objects = _pRoom->getObjects();
    auto it = std::find_if(objects.cbegin(), objects.cend(), [mousPos](const std::unique_ptr<Object> &pObj) {
        if (!pObj->isTouchable())
            return false;
        auto rect = pObj->getRealHotspot();
        return rect.contains((sf::Vector2i)mousPos);
    });
    if (it != objects.cend())
    {
        _pCurrentObject = it->get();
    }
}

void Engine::update(const sf::Time &elapsed)
{
    auto wasMouseDown = _isMouseDown;
    _isMouseDown = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
    auto isMouseClick = wasMouseDown != _isMouseDown && !_isMouseDown;

    _time += elapsed;

    updateCutscene(elapsed);
    updateFunctions(elapsed);

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
    updateActorIcons(elapsed);

    _cursorDirection = CursorDirection::None;
    updateMouseCursor();

    auto mousePosInRoom = _mousePos + _cameraPos;

    updateCurrentObject(mousePosInRoom);

    _inventory.setMousePosition(_mousePos);
    _inventory.update(elapsed);
    _dialogManager.update(elapsed);

    if (!_inputActive)
        return;

    if (!isMouseClick)
        return;

    if (clickedAt(mousePosInRoom))
        return;

    if (_dialogManager.isActive())
        return;

    if (_actorIcons.isMouseOver())
        return;

    // input click on a verb ?
    auto verbId = -1;
    if (_inputVerbsActive)
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

    int currentActorIndex = getCurrentActorIndex();
    if (verbId != -1 && currentActorIndex != -1)
    {
        _pVerb = &_verbSlots[currentActorIndex].getVerb(1 + verbId);
        _useFlag = UseFlag::None;
        _pUseObject = nullptr;
    }
    else if (_pVerb && _pVerb->id == 1 && !_pCurrentObject && _pCurrentActor)
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
            if(_pVerb && _pVerb->id == 1)
            {
                _pVerb = getVerb(_pCurrentObject->getDefaultVerb());
            }
            _pVerbExecute->execute(_pCurrentObject, _pVerb);
        }
    }
    else if (_inventory.getCurrentInventoryObject())
    {
        _pVerbExecute->execute(_inventory.getCurrentInventoryObject(), _pVerb);
    }
    else if (currentActorIndex != -1)
    {
        _pVerb = getVerb(1);
        _useFlag = UseFlag::None;
        _pUseObject = nullptr;
    }
}

void Engine::setCurrentActor(Actor *pCurrentActor)
{
    _pCurrentActor = pCurrentActor;
    if (_pCurrentActor)
    {
        follow(_pCurrentActor);
    }
}

bool Engine::clickedAt(const sf::Vector2f &pos)
{
    if (!_pRoom)
        return false;

    auto pTable = _pRoom->getTable();
    sq_pushobject(_vm, *pTable);
    sq_pushstring(_vm, _SC("clickedAt"), -1);
    if (SQ_SUCCEEDED(sq_get(_vm, -2)))
    {
        sq_remove(_vm, -2);
        sq_pushobject(_vm, *pTable);
        sq_pushinteger(_vm, pos.x);
        sq_pushinteger(_vm, pos.y);
        sq_call(_vm, 3, SQTrue, SQTrue);
        SQInteger handled = 0;
        sq_getinteger(_vm, -1, &handled);
        return handled == 1;
    }
    return false;
}

void Engine::draw(sf::RenderWindow &window) const
{
    if (!_pRoom)
        return;

    _pRoom->draw(window, _cameraPos);

    window.draw(_dialogManager);

    if (!_dialogManager.isActive() && _inputActive)
    {
        drawVerbs(window);
        window.draw(_inventory);
        window.draw(_actorIcons);
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

    auto pVerb = _pVerb;
    if (!pVerb)
        pVerb = getVerb(1);
    if (_pCurrentObject)
    {
        NGText text;
        text.setFont(_fntFont);
        text.setColor(sf::Color::White);
        std::wstring s;

        if (pVerb->id == 1)
        {
            pVerb = getVerb(_pCurrentObject->getDefaultVerb());
        }

        if (!pVerb->text.empty())
        {
            auto id = std::strtol(pVerb->text.substr(1).data(), nullptr, 10);
            s.append(getText(id));
        }

        if (_pUseObject)
        {
            s.append(L" ").append(_pUseObject->getName());
        }
        else
        {
            s.append(L" ").append(_pCurrentObject->getName());
        }
        appendUseFlag(s);
        if (_pUseObject)
        {
            s.append(L" ").append(_pCurrentObject->getName());
        }
        text.setText(s);
        text.setPosition((sf::Vector2f)_mousePos - sf::Vector2f(0, 22));
        window.draw(text, sf::RenderStates::Default);

        sf::RenderStates states;
        states.transform.translate(-_cameraPos);
        _pCurrentObject->drawHotspot(window, states);
    }
    else if (pVerb->id != 1)
    {
        NGText text;
        text.setFont(_fntFont);
        text.setColor(sf::Color::White);

        std::wstring s;
        auto id = std::strtol(pVerb->text.substr(1).data(), nullptr, 10);
        s.append(getText(id));

        if (_inventory.getCurrentInventoryObject())
        {
            s.append(L" ").append(_inventory.getCurrentInventoryObject()->getName());
        }
        appendUseFlag(s);
        text.setText(s);

        text.setPosition((sf::Vector2f)_mousePos - sf::Vector2f(0, 22));
        window.draw(text, sf::RenderStates::Default);
    }
}

void Engine::appendUseFlag(std::wstring &sentence) const
{
    switch (_useFlag)
    {
    case UseFlag::UseWith:
        sentence.append(L" ").append(getText(10000));
        break;
    case UseFlag::UseIn:
        sentence.append(L" ").append(getText(10002));
        break;
    case UseFlag::UseOn:
        sentence.append(L" ").append(getText(10001));
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
    if (!_inputVerbsActive)
        return;

    int currentActorIndex = getCurrentActorIndex();
    if (!_inputActive || currentActorIndex == -1 || _verbSlots[currentActorIndex].getVerb(0).id == 0)
        return;

    auto verbId = -1;
    if (_pCurrentObject)
    {
        auto defaultVerb = _pCurrentObject->getDefaultVerb();
        verbId = _verbSlots[currentActorIndex].getVerbIndex(defaultVerb);
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

bool Engine::isThreadAlive(HSQUIRRELVM thread) const
{
    return std::find(_threads.begin(), _threads.end(), thread) != _threads.end();
}

void Engine::startDialog(const std::string &dialog, const std::string &node)
{
    _dialogManager.start(dialog, node);
}

void Engine::execute(const std::string &code)
{
    _pScriptExecute->execute(code);
}

SoundDefinition *Engine::getSoundDefinition(const std::string &name)
{
    return _pScriptExecute->getSoundDefinition(name);
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

void Engine::actorSlotSelectable(int index, bool selectable)
{
    _actorsIconSlots[index].selectable = selectable;
}

void Engine::cutsceneOverride()
{
    if (!_pCutscene)
        return;
    _pCutscene->cutsceneOverride();
}

void Engine::cutscene(std::unique_ptr<Cutscene> function)
{
    _pCutscene = std::move(function);
}

bool Engine::inCutscene() const
{
    return _pCutscene && !_pCutscene->isElapsed();
}

} // namespace ng
