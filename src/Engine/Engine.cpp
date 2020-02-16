#include "squirrel.h"
#include "Engine/Engine.hpp"
#include "Engine/ActorIconSlot.hpp"
#include "Engine/ActorIcons.hpp"
#include "Engine/Camera.hpp"
#include "Engine/Cutscene.hpp"
#include "Dialog/DialogManager.hpp"
#include "Font/FntFont.hpp"
#include "Font/Font.hpp"
#include "Math/PathFinding/Graph.hpp"
#include "Engine/Inventory.hpp"
#include "UI/OptionsDialog.hpp"
#include "UI/StartScreenDialog.hpp"
#include "Engine/Preferences.hpp"
#include "Room/Room.hpp"
#include "Room/RoomScaling.hpp"
#include "Graphics/Screen.hpp"
#include "Scripting/ScriptEngine.hpp"
#include "Scripting/ScriptExecute.hpp"
#include "Engine/Sentence.hpp"
#include "Audio/SoundDefinition.hpp"
#include "Audio/SoundId.hpp"
#include "Audio/SoundManager.hpp"
#include "Graphics/SpriteSheet.hpp"
#include "Engine/TextDatabase.hpp"
#include "Engine/Thread.hpp"
#include "Engine/Verb.hpp"
#include "Scripting/VerbExecute.hpp"
#include "../System/_DebugTools.hpp"
#include "../Entities/Actor/_TalkingState.hpp"
#include "System/Logger.hpp"
#include "../Math/PathFinding/_WalkboxDrawable.hpp"
#include <iostream>
#include <cmath>
#include <memory>
#include <set>
#include <string>

namespace ng
{
static const char* _verbShaderCode = "\n"
"#ifdef GL_ES\n"
"precision highp float;\n"
"#endif\n"
"\n"
"\n"
"uniform vec4 color;\n"
"uniform vec4 shadowColor;\n"
"uniform vec4 normalColor;\n"
"uniform vec4 highlightColor;\n"
"uniform vec2 ranges;\n"
"uniform sampler2D colorMap;\n"
"\n"
"void main(void)\n"
"{\n"
"    float shadows = ranges.x;\n"
"    float highlights = ranges.y;\n"
"    \n"
"    vec4 texColor = texture2D( colorMap, gl_TexCoord[0].xy);\n"
"    \n"
"    if ( texColor.g <= shadows)\n"
"    {\n"
"        texColor*=shadowColor;\n"
"    }\n"
"    else if (texColor.g >= highlights)\n"
"    {\n"
"        texColor*=highlightColor;\n"
"    }\n"
"    else\n"
"    {\n"
"        texColor*=normalColor;\n"
"    }\n"
"    texColor *= color;\n"
"    gl_FragColor = texColor;\n"
"}\n";

CursorDirection operator|=(CursorDirection &lhs, CursorDirection rhs)
{
    lhs = static_cast<CursorDirection>(static_cast<std::underlying_type<CursorDirection>::type>(lhs) |
                                       static_cast<std::underlying_type<CursorDirection>::type>(rhs));
    return lhs;
}

bool operator&(CursorDirection lhs, CursorDirection rhs)
{
    return static_cast<CursorDirection>(static_cast<std::underlying_type<CursorDirection>::type>(lhs) &
                                        static_cast<std::underlying_type<CursorDirection>::type>(rhs)) >
           CursorDirection::None;
}

enum class EngineState {
    Game, Paused, Options, StartScreen
};

struct Engine::Impl
{
    Engine *_pEngine{nullptr};
    std::unique_ptr<_DebugTools> _pDebugTools;
    TextureManager _textureManager;
    Room *_pRoom{nullptr};
    std::vector<std::unique_ptr<Actor>> _actors;
    std::vector<std::unique_ptr<Room>> _rooms;
    std::vector<std::unique_ptr<Function>> _newFunctions;
    std::vector<std::unique_ptr<Function>> _functions;
    std::vector<std::unique_ptr<Callback>> _callbacks;
    Cutscene *_pCutscene{nullptr};
    sf::RenderWindow *_pWindow{nullptr};
    TextDatabase _textDb;
    Actor *_pCurrentActor{nullptr};
    std::array<VerbSlot, 6> _verbSlots;
    std::array<VerbUiColors, 6> _verbUiColors;
    bool _inputHUD{true};
    bool _inputActive{false};
    bool _showCursor{false};
    bool _inputVerbsActive{false};
    SpriteSheet _verbSheet, _gameSheet, _saveLoadSheet;
    Actor *_pFollowActor{nullptr};
    std::array<sf::IntRect, 9> _verbRects;
    Entity *_pUseObject{nullptr};
    Entity *_pObj1{nullptr};
    Entity *_pObj2{nullptr};
    Entity *_pHoveredEntity{nullptr};
    sf::Vector2f _mousePos;
    sf::Vector2f _mousePosInRoom;
    std::unique_ptr<VerbExecute> _pVerbExecute;
    std::unique_ptr<ScriptExecute> _pScriptExecute;
    const Verb *_pVerb{nullptr};
    const Verb *_pVerbOverride{nullptr};
    std::vector<std::unique_ptr<ThreadBase>> _threads;
    DialogManager _dialogManager;
    Preferences& _preferences;
    SoundManager& _soundManager;
    CursorDirection _cursorDirection{CursorDirection::None};
    std::array<ActorIconSlot, 6> _actorsIconSlots;
    UseFlag _useFlag{UseFlag::None};
    ActorIcons _actorIcons;
    Inventory _inventory;
    HSQUIRRELVM _vm{};
    sf::Time _time;
    bool _isMouseDown{false};
    sf::Time _mouseDownTime;
    bool _isMouseRightDown{false};
    int _frameCounter{0};
    HSQOBJECT _pDefaultObject{};
    Camera _camera;
    sf::Color _fadeColor{sf::Color::Transparent};
    std::unique_ptr<Sentence> _pSentence{};
    std::set<int> _oldKeyDowns;
    std::set<int> _newKeyDowns;
    EngineState _state{EngineState::StartScreen};
    mutable sf::Shader _verbShader{};
    sf::Vector2f _ranges{0.8f, 0.8f};
    sf::Color _verbColor, _verbShadowColor, _verbNormalColor, _verbHighlightColor;
    _TalkingState _talkingState;
    int _showDrawWalkboxes{0};
    OptionsDialog _optionsDialog;
    StartScreenDialog _startScreenDialog;
    bool _run{false};

    Impl();

    void drawVerbs(sf::RenderWindow &window) const;
    void drawCursor(sf::RenderWindow &window) const;
    void drawCursorText(sf::RenderTarget &target) const;
    int getCurrentActorIndex() const;
    sf::IntRect getCursorRect() const;
    void appendUseFlag(std::wstring &sentence) const;
    bool clickedAt(const sf::Vector2f &pos);
    void updateCutscene(const sf::Time &elapsed);
    void updateFunctions(const sf::Time &elapsed);
    void updateActorIcons(const sf::Time &elapsed);
    void updateSentence(const sf::Time &elapsed);
    void updateMouseCursor();
    void updateHoveredEntity(bool isRightClick);
    SQInteger enterRoom(Room *pRoom, Object *pObject);
    SQInteger exitRoom(Object *pObject);
    void updateScreenSize();
    void updateRoomScalings();
    void setCurrentRoom(Room *pRoom);
    int32_t getFlags(HSQOBJECT obj) const;
    static int getDefaultVerb(HSQUIRRELVM vm, const Entity *pEntity) ;
    Entity *getHoveredEntity(const sf::Vector2f &mousPos);
    void actorEnter();
    void actorExit();
    void onLanguageChange(const std::string &lang);
    std::string getVerbName(const Verb &verb) const;
    void drawFade(sf::RenderTarget &target) const;
    void onVerbClick(const Verb* pVerb);
    void updateKeyboard();
    bool isKeyPressed(int key);
    void updateKeys();
    static int toKey(const std::string& keyText);
    void drawPause(sf::RenderTarget &target) const;
    void stopThreads();
    void drawWalkboxes(sf::RenderTarget &target) const;
    const Verb* getHoveredVerb() const;
    std::wstring getDisplayName(const std::wstring& name) const;
    void run(bool state);
};

Engine::Impl::Impl()
    : _preferences(Locator<Preferences>::get()),
    _soundManager(Locator<SoundManager>::get()),
    _actorIcons(_actorsIconSlots, _verbUiColors, _pCurrentActor),
    _inventory(_actorsIconSlots, _verbUiColors, _pCurrentActor)
{
    _verbSheet.setTextureManager(&_textureManager);
    _gameSheet.setTextureManager(&_textureManager);
    _saveLoadSheet.setTextureManager(&_textureManager);
    sq_resetobject(&_pDefaultObject);

    // load vertex shader
    if (!_verbShader.loadFromMemory(_verbShaderCode, sf::Shader::Type::Fragment))
    {
        std::cerr << "Error loading shaders" << std::endl;
        return;
    }
    _verbShader.setUniform("colorMap", sf::Shader::CurrentTexture);
}

void Engine::Impl::onLanguageChange(const std::string &lang)
{
    std::stringstream ss;
    ss << "ThimbleweedText_" << lang << ".tsv";
    _textDb.load(ss.str());

    ScriptEngine::call("onLanguageChange");
}

void Engine::Impl::drawFade(sf::RenderTarget &target) const
{
    sf::RectangleShape fadeShape;
    auto screen = target.getView().getSize();
    fadeShape.setSize(sf::Vector2f(screen.x, screen.y));
    fadeShape.setFillColor(_fadeColor);
    target.draw(fadeShape);
}

Engine::Engine() : _pImpl(std::make_unique<Impl>())
{
    time_t t;
    auto seed = (unsigned)time(&t);
    info("seed: {}", seed);
    srand(seed);

    _pImpl->_pEngine = this;
    _pImpl->_pDebugTools = std::make_unique<_DebugTools>(*this);
    _pImpl->_soundManager.setEngine(this);
    _pImpl->_dialogManager.setEngine(this);
    _pImpl->_actorIcons.setEngine(this);
    _pImpl->_inventory.setEngine(this);
    _pImpl->_camera.setEngine(this);
    _pImpl->_talkingState.setEngine(this);
    
    // load all messages
    std::stringstream s;
    auto lang = _pImpl->_preferences.getUserPreference<std::string>(PreferenceNames::Language, PreferenceDefaultValues::Language);
    s << "ThimbleweedText_" << lang << ".tsv";
    _pImpl->_textDb.load(s.str());

    _pImpl->_optionsDialog.setEngine(this);
    _pImpl->_optionsDialog.setCallback([this](){
        showOptions(false);
    });
    _pImpl->_startScreenDialog.setEngine(this);
    _pImpl->_startScreenDialog.setNewGameCallback([this](){
        _pImpl->_state = EngineState::Game;
        ScriptEngine::call("start",true);
    });
    _pImpl->_verbSheet.load("VerbSheet");
    _pImpl->_gameSheet.load("GameSheet");
    _pImpl->_saveLoadSheet.load("SaveLoadSheet");

    _pImpl->_preferences.subscribe([this](const std::string &name) {
        if (name == PreferenceNames::Language)
        {
            auto newLang = _pImpl->_preferences.getUserPreference<std::string>(PreferenceNames::Language,PreferenceDefaultValues::Language);
            _pImpl->onLanguageChange(newLang);
        }
    });
}

Engine::~Engine() = default;

int Engine::getFrameCounter() const { return _pImpl->_frameCounter; }

void Engine::setWindow(sf::RenderWindow &window) { _pImpl->_pWindow = &window; }

const sf::RenderWindow &Engine::getWindow() const { return *_pImpl->_pWindow; }

TextureManager &Engine::getTextureManager() { return _pImpl->_textureManager; }

Room *Engine::getRoom() { return _pImpl->_pRoom; }

std::wstring Engine::getText(int id) const
{
    auto text = _pImpl->_textDb.getText(id);
    replaceAll(text, L"\\\"", L"\"");
    removeFirstParenthesis(text);
    return text;
}

std::wstring Engine::getText(const std::string& text) const
{
    if(!text.empty() && text[0]=='@')
    {
        auto id = std::strtol(text.c_str()+1, nullptr, 10);
        return getText(id);
    }
    return towstring(text);
}

void Engine::addActor(std::unique_ptr<Actor> actor) { _pImpl->_actors.push_back(std::move(actor)); }

void Engine::addRoom(std::unique_ptr<Room> room) { _pImpl->_rooms.push_back(std::move(room)); }

std::vector<std::unique_ptr<Room>> &Engine::getRooms() { return _pImpl->_rooms; }

void Engine::addFunction(std::unique_ptr<Function> function) { _pImpl->_newFunctions.push_back(std::move(function)); }

void Engine::addCallback(std::unique_ptr<Callback> callback) { _pImpl->_callbacks.push_back(std::move(callback)); }

void Engine::removeCallback(int id)
{
    auto it = std::find_if(_pImpl->_callbacks.begin(), _pImpl->_callbacks.end(),
                           [id](auto &callback) -> bool { return callback->getId() == id; });
    if(it != _pImpl->_callbacks.end())
    {
        _pImpl->_callbacks.erase(it);
    }
}

std::vector<std::unique_ptr<Actor>> &Engine::getActors() { return _pImpl->_actors; }

Actor *Engine::getCurrentActor() { return _pImpl->_pCurrentActor; }

void Engine::setVerb(int characterSlot, int verbSlot, const Verb &verb)
{
    _pImpl->_verbSlots.at(characterSlot).setVerb(verbSlot, verb);
}

void Engine::setVerbUiColors(int characterSlot, VerbUiColors colors)
{
    _pImpl->_verbUiColors.at(characterSlot) = colors;
}

const VerbUiColors *Engine::getVerbUiColors(const std::string& name) const
{
    if(name.empty())
    {
        auto index = _pImpl->getCurrentActorIndex();
        if(index == -1) return nullptr;
        return &_pImpl->_verbUiColors.at(index);
    }
    for (auto i = 0; i < _pImpl->_actorsIconSlots.size(); i++)
    {
        const auto &selectableActor = _pImpl->_actorsIconSlots.at(i);
        if (selectableActor.pActor->getKey() == name)
        {
            return &_pImpl->_verbUiColors.at(i);
        }
    }
    return nullptr;
}

bool Engine::getInputActive() const { return _pImpl->_inputActive; }

void Engine::setInputState(int state)
{
    if ((state & InputStateConstants::UI_INPUT_ON) == InputStateConstants::UI_INPUT_ON)
    {
        _pImpl->_inputActive = true;
    }
    if ((state & InputStateConstants::UI_INPUT_OFF) == InputStateConstants::UI_INPUT_OFF)
    {
        _pImpl->_inputActive = false;
    }
    if ((state & InputStateConstants::UI_VERBS_ON) == InputStateConstants::UI_VERBS_ON)
    {
        _pImpl->_inputVerbsActive = true;
    }
    if ((state & InputStateConstants::UI_VERBS_OFF) == InputStateConstants::UI_VERBS_OFF)
    {
        _pImpl->_inputVerbsActive = false;
    }
    if ((state & InputStateConstants::UI_CURSOR_ON) == InputStateConstants::UI_CURSOR_ON)
    {
        _pImpl->_showCursor = true;
    }
    if ((state & InputStateConstants::UI_CURSOR_OFF) == InputStateConstants::UI_CURSOR_OFF)
    {
        _pImpl->_showCursor = false;
    }
    if ((state & InputStateConstants::UI_HUDOBJECTS_ON) == InputStateConstants::UI_HUDOBJECTS_ON)
    {
        _pImpl->_inputHUD = true;
    }
    if ((state & InputStateConstants::UI_HUDOBJECTS_OFF) == InputStateConstants::UI_HUDOBJECTS_OFF)
    {
        _pImpl->_inputHUD = false;
    }
}

int Engine::getInputState() const
{
    int inputState = 0;
    inputState |= (_pImpl->_inputActive ? InputStateConstants::UI_INPUT_ON : InputStateConstants::UI_INPUT_OFF);
    inputState |= (_pImpl->_inputVerbsActive ? InputStateConstants::UI_VERBS_ON : InputStateConstants::UI_VERBS_OFF);
    inputState |= (_pImpl->_showCursor ? InputStateConstants::UI_CURSOR_ON : InputStateConstants::UI_CURSOR_OFF);
    inputState |= (_pImpl->_inputHUD ? InputStateConstants::UI_HUDOBJECTS_ON : InputStateConstants::UI_HUDOBJECTS_OFF);
    return inputState;
}

void Engine::follow(Actor *pActor)
{
    auto panCamera =
        (_pImpl->_pFollowActor && pActor && _pImpl->_pFollowActor != pActor && _pImpl->_pFollowActor->getRoom() &&
         pActor->getRoom() && _pImpl->_pFollowActor->getRoom()->getId() == pActor->getRoom()->getId());
    _pImpl->_pFollowActor = pActor;
    if (!pActor)
        return;

    auto pos = pActor->getRealPosition();
    auto screen = _pImpl->_pWindow->getView().getSize();
    setRoom(pActor->getRoom());
    if (panCamera)
    {
        _pImpl->_camera.panTo(pos - sf::Vector2f(screen.x / 2, screen.y / 2), sf::seconds(4),
                              InterpolationMethod::EaseOut);
        return;
    }
    _pImpl->_camera.at(pos - sf::Vector2f(screen.x / 2, screen.y / 2));
}

void Engine::setVerbExecute(std::unique_ptr<VerbExecute> verbExecute)
{
    _pImpl->_pVerbExecute = std::move(verbExecute);
}

void Engine::setDefaultVerb()
{
    _pImpl->_pHoveredEntity = nullptr;
    auto index = _pImpl->getCurrentActorIndex();
    if (index == -1)
        return;

    const auto &verbSlot = _pImpl->_verbSlots.at(index);
    _pImpl->_pVerb = &verbSlot.getVerb(0);
    _pImpl->_useFlag = UseFlag::None;
    _pImpl->_pUseObject = nullptr;
    _pImpl->_pObj1 = nullptr;
    _pImpl->_pObj2 = nullptr;
}

void Engine::setScriptExecute(std::unique_ptr<ScriptExecute> scriptExecute)
{
    _pImpl->_pScriptExecute = std::move(scriptExecute);
}

void Engine::addThread(std::unique_ptr<ThreadBase> thread) { _pImpl->_threads.push_back(std::move(thread)); }

std::vector<std::unique_ptr<ThreadBase>>& Engine::getThreads() { return _pImpl->_threads; }

sf::Vector2f Engine::getMousePos() const { return _pImpl->_mousePos; }

sf::Vector2f Engine::getMousePositionInRoom() const { return _pImpl->_mousePosInRoom; }

sf::Vector2f Engine::findScreenPosition(int verbId) const
{
    auto pVerb = getVerb(verbId);
    auto s = _pImpl->getVerbName(*pVerb);
    auto r = _pImpl->_verbSheet.getSpriteSourceSize(s);
    return sf::Vector2f(r.left + r.width / 2.f, r.top + r.height / 2.f);
}

Preferences &Engine::getPreferences() { return _pImpl->_preferences; }

SoundManager &Engine::getSoundManager() { return _pImpl->_soundManager; }

DialogManager &Engine::getDialogManager() { return _pImpl->_dialogManager; }

Camera &Engine::getCamera() { return _pImpl->_camera; }

sf::Time Engine::getTime() const { return _pImpl->_time; }

void Engine::setVm(HSQUIRRELVM vm) { _pImpl->_vm = vm; }

HSQUIRRELVM Engine::getVm() { return _pImpl->_vm; }

SQInteger Engine::Impl::exitRoom(Object *pObject)
{
    _pEngine->setDefaultVerb();
    _talkingState.stop();

    if (!_pRoom)
        return 0;

    auto pOldRoom = _pRoom;

    // call exit room function
    trace("call exit room function of {}", pOldRoom->getId());

    sq_pushobject(_vm, pOldRoom->getTable());
    sq_pushstring(_vm, _SC("exit"), -1);
    if (SQ_FAILED(sq_get(_vm, -2)))
    {
        error("can't find exit function");
        return 0;
    }

    SQInteger nparams, nfreevars;
    sq_getclosureinfo(_vm, -1, &nparams, &nfreevars);
    trace("enter function found with {} parameters", nparams);

    actorExit();

    sq_remove(_vm, -2);
    if (nparams == 2)
    {
        ScriptEngine::rawCall(pOldRoom, "exit", pObject);
    }
    else
    {
        ScriptEngine::rawCall(pOldRoom, "exit");
    }

    pOldRoom->exit();

    ScriptEngine::rawCall("exitedRoom", pOldRoom);

    // remove all local threads
    _threads.erase(std::remove_if(_threads.begin(), _threads.end(), [](auto& pThread) -> bool {
        return !pThread->isGlobal();
    }), _threads.end());

    return 0;
}

int Engine::Impl::getDefaultVerb(HSQUIRRELVM v, const Entity *pEntity)
{
    sq_pushobject(v, pEntity->getTable());
    sq_pushstring(v, _SC("defaultVerb"), -1);
    if (SQ_SUCCEEDED(sq_get(v, -2)))
    {
        SQInteger value = 0;
        sq_getinteger(v, -1, &value);
        sq_pop(v, 2);
        return value;
    }
    sq_pop(v, 1);
    return VerbConstants::VERB_LOOKAT;
}

void Engine::Impl::updateScreenSize()
{
    if (_pRoom)
    {
        auto screen = _pRoom->getFullscreen() == 1 ? _pRoom->getRoomSize() : _pRoom->getScreenSize();
        sf::View view(sf::FloatRect(0, 0, screen.x, screen.y));
        _pWindow->setView(view);

        sf::Vector2f size(screen.x / 6.f, screen.y / 14.f);
        for (auto i = 0; i < 9; i++)
        {
            auto left = (i / 3) * size.x;
            auto top = screen.y - size.y * 3 + (i % 3) * size.y;
            _verbRects.at(i) = sf::IntRect(left, top, size.x, size.y);
        }
    }
}

void Engine::Impl::actorEnter()
{
    if (!_pCurrentActor)
        return;

    ScriptEngine::rawCall("actorEnter", _pCurrentActor);

    if (!_pRoom)
        return;

    ScriptEngine::rawCall(_pRoom, "actorEnter", _pCurrentActor);
}

void Engine::Impl::actorExit()
{
    if (!_pCurrentActor || !_pRoom)
        return;

    ScriptEngine::rawCall(_pRoom, "actorExit", _pCurrentActor);
}

SQInteger Engine::Impl::enterRoom(Room *pRoom, Object *pObject)
{
    // call enter room function
    trace("call enter room function of {}", pRoom->getName());
    auto nparams = ScriptEngine::getParameterCount(pRoom, "enter");
    if (nparams == 2)
    {
        ScriptEngine::rawCall(pRoom, "enter", pObject);
    }
    else
    {
        ScriptEngine::rawCall(pRoom, "enter");
    }

    actorEnter();

    auto &objects = pRoom->getObjects();
    for (auto & obj : objects)
    {
        if (obj->getId() == 0 || obj->isTemporary())
            continue;

        if(ScriptEngine::exists(obj.get(), "enter"))
        {
            ScriptEngine::rawCall(obj.get(), "enter");
        }
    }

    ScriptEngine::rawCall("enteredRoom", pRoom);

    return 0;
}

void Engine::Impl::run(bool state)
{
    if(_run != state)
    {
        _run = state;
        if(_pCurrentActor)
        {
            ScriptEngine::call(_pCurrentActor, "run", state);
        }
    }
}

void Engine::Impl::setCurrentRoom(Room *pRoom)
{
    if (pRoom)
    {
        std::ostringstream s;
        s << "currentRoom = " << pRoom->getName();
        _pScriptExecute->execute(s.str());
    }
    _camera.resetBounds();
    _camera.at(sf::Vector2f(0, 0));
    _pRoom = pRoom;
    updateScreenSize();
}

SQInteger Engine::setRoom(Room *pRoom)
{
    if (!pRoom)
        return 0;

    _pImpl->_fadeColor = sf::Color::Transparent;

    auto pOldRoom = _pImpl->_pRoom;
    if (pRoom == pOldRoom)
        return 0;

    auto result = _pImpl->exitRoom(nullptr);
    if (SQ_FAILED(result))
        return result;

    _pImpl->setCurrentRoom(pRoom);

    result = _pImpl->enterRoom(pRoom, nullptr);
    if (SQ_FAILED(result))
        return result;

    return 0;
}

SQInteger Engine::enterRoomFromDoor(Object *pDoor)
{
    auto dir = pDoor->getUseDirection();
    Facing facing;
    switch (dir)
    {
        case UseDirection::Back:
            facing = Facing::FACE_FRONT;
            break;
        case UseDirection::Front:
            facing = Facing::FACE_BACK;
            break;
        case UseDirection::Left:
            facing = Facing::FACE_RIGHT;
            break;
        case UseDirection::Right:
            facing = Facing::FACE_LEFT;
            break;
        default:
            throw std::invalid_argument("direction is invalid");
    }
    auto pRoom = pDoor->getRoom();
    auto pOldRoom = _pImpl->_pRoom;
    if (pRoom == pOldRoom)
        return 0;

    auto result = _pImpl->exitRoom(nullptr);
    if (SQ_FAILED(result))
        return result;

    _pImpl->setCurrentRoom(pRoom);

    auto actor = getCurrentActor();
    actor->getCostume().setFacing(facing);

    if (pRoom->getFullscreen() != 1)
    {
        actor->setRoom(pRoom);
        auto pos = pDoor->getRealPosition();
        auto usePos = pDoor->getUsePosition();
        auto roomHeight = pDoor->getRoom()->getRoomSize().y;
        pos.x += usePos.x;
        pos.y += usePos.y - roomHeight;
        actor->setPosition(pos);
        _pImpl->_camera.at(pos);
    }

    return _pImpl->enterRoom(pRoom, pDoor);
}

void Engine::setInputHUD(bool on) { _pImpl->_inputHUD = on; }

void Engine::setInputActive(bool active)
{
    _pImpl->_inputActive = active;
    _pImpl->_showCursor = active;
}

void Engine::inputSilentOff() { _pImpl->_inputActive = false; }

void Engine::setInputVerbs(bool on) { _pImpl->_inputVerbsActive = on; }

std::string Engine::Impl::getVerbName(const Verb &verb) const
{
    auto lang = _preferences.getUserPreference(PreferenceNames::Language, PreferenceDefaultValues::Language);
    auto isRetro = _preferences.getUserPreference(PreferenceNames::RetroVerbs, PreferenceDefaultValues::RetroVerbs);
    std::string s;
    s.append(verb.image).append(isRetro ? "_retro" : "").append("_").append(lang);
    return s;
}

const Verb *Engine::getVerb(int id) const
{
    auto index = _pImpl->getCurrentActorIndex();
    if (index < 0)
        return nullptr;
    const auto &verbSlot = _pImpl->_verbSlots.at(index);
    for (auto i = 0; i < 10; i++)
    {
        const auto &verb = verbSlot.getVerb(i);
        if (verb.id == id)
        {
            return &verb;
        }
    }
    return nullptr;
}

void Engine::Impl::updateCutscene(const sf::Time &elapsed)
{
    if (_pCutscene)
    {
        (*_pCutscene)(elapsed);
        if (_pCutscene->isElapsed())
        {
            _pCutscene = nullptr;
        }
    }
}

void Engine::Impl::updateSentence(const sf::Time &elapsed)
{
    if(!_pSentence) return;
    (*_pSentence)(elapsed);
    if(!_pSentence->isElapsed()) return;
    _pEngine->stopSentence();
}

void Engine::Impl::updateFunctions(const sf::Time &elapsed)
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
    for (auto &callback : _callbacks)
    {
        (*callback)(elapsed);
    }
}

void Engine::Impl::updateActorIcons(const sf::Time &elapsed)
{
    _actorIcons.setMousePosition(_mousePos);
    _actorIcons.update(elapsed);
}

void Engine::Impl::updateMouseCursor()
{
    auto flags = _pObj1 ? getFlags(_pObj1->getTable()) : 0;
    auto screen = _pWindow->getView().getSize();
    _cursorDirection = CursorDirection::None;
    if ((_mousePos.x < 20) || (flags & ObjectFlagConstants::DOOR_LEFT) == ObjectFlagConstants::DOOR_LEFT)
        _cursorDirection |= CursorDirection::Left;
    else if ((_mousePos.x > screen.x - 20) ||
             (flags & ObjectFlagConstants::DOOR_RIGHT) == ObjectFlagConstants::DOOR_RIGHT)
        _cursorDirection |= CursorDirection::Right;
    if ((flags & ObjectFlagConstants::DOOR_FRONT) == ObjectFlagConstants::DOOR_FRONT)
        _cursorDirection |= CursorDirection::Down;
    else if ((flags & ObjectFlagConstants::DOOR_BACK) == ObjectFlagConstants::DOOR_BACK)
        _cursorDirection |= CursorDirection::Up;
    if ((_cursorDirection == CursorDirection::None) && _pObj1)
        _cursorDirection |= CursorDirection::Hotspot;
}

Entity *Engine::Impl::getHoveredEntity(const sf::Vector2f &mousPos)
{
    Entity *pCurrentObject = nullptr;

    // mouse on actor ?
    for (auto &&actor : _actors)
    {
        if (actor.get() == _pCurrentActor)
            continue;
        if (actor->getRoom() != _pRoom)
            continue;

        if (actor->contains(mousPos))
        {
            if (!pCurrentObject || actor->getZOrder() < pCurrentObject->getZOrder())
            {
                pCurrentObject = actor.get();
            }
        }
    }

    // mouse on object ?
    const auto &objects = _pRoom->getObjects();
    std::for_each(objects.cbegin(), objects.cend(), [mousPos, &pCurrentObject](const auto &pObj) {
        if (!pObj->isTouchable())
            return;
        auto rect = pObj->getRealHotspot();
        if (!rect.contains((sf::Vector2i)mousPos))
            return;
        if (!pCurrentObject || pObj->getZOrder() < pCurrentObject->getZOrder())
            pCurrentObject = pObj.get();
    });

    if (!pCurrentObject)
    {
        // mouse on inventory object ?
        pCurrentObject = _inventory.getCurrentInventoryObject();
    }

    return pCurrentObject;
}

void Engine::Impl::updateHoveredEntity(bool isRightClick)
{
    _pVerbOverride = nullptr;
    if (!_pVerb)
    {
        _pVerb = _pEngine->getVerb(VerbConstants::VERB_WALKTO);
    }

    if (_pUseObject)
    {
        _pObj1 = _pUseObject;
        _pObj2 = _pHoveredEntity;
    }
    else
    {
        _pObj1 = _pHoveredEntity;
        _pObj2 = nullptr;
    }

    // abort some invalid actions
    if (!_pObj1 || !_pVerb)
    {
        return;
    }

    if (_pObj2 == _pObj1)
    {
        _pObj2 = nullptr;
    }

    if (_pObj1 && isRightClick)
    {
        _pVerbOverride = _pEngine->getVerb(getDefaultVerb(_vm, _pObj1));
    }

    if (_pVerb->id == VerbConstants::VERB_WALKTO)
    {
        if (_pObj1 && _pObj1->isInventoryObject())
        {
            _pVerbOverride = _pEngine->getVerb(getDefaultVerb(_vm, _pObj1));
        }
    }
    else if (_pVerb->id == VerbConstants::VERB_TALKTO)
    {
        // select actor/object only if talkable flag is set
        auto flags = getFlags(_pObj1->getTable());
        if (!(flags & ObjectFlagConstants::TALKABLE))
            _pObj1 = nullptr;
    }
    else if (_pVerb->id == VerbConstants::VERB_GIVE)
    {
        if (!_pObj1->isInventoryObject())
            _pObj1 = nullptr;

        // select actor/object only if giveable flag is set
        if (_pObj2)
        {
            auto flags = getFlags(_pObj2->getTable());
            if (!(flags & ObjectFlagConstants::GIVEABLE))
                _pObj2 = nullptr;
        }
    }
}

int32_t Engine::Impl::getFlags(HSQOBJECT obj) const
{
    SQInteger flags = 0;
    sq_pushobject(_vm, obj);
    sq_pushstring(_vm, _SC("flags"), -1);
    if (SQ_SUCCEEDED(sq_rawget(_vm, -2)))
    {
        sq_getinteger(_vm, -1, &flags);
        sq_pop(_vm, 1);
    }
    sq_pop(_vm, 1);
    return flags;
}

void Engine::Impl::updateRoomScalings()
{
    auto actor = _pCurrentActor;
    if (!actor)
        return;

    auto &scalings = _pRoom->getScalings();
    auto &objects = _pRoom->getObjects();
    for (auto &&object : objects)
    {
        if (object->getType() != ObjectType::Trigger)
            continue;
        if (object->getRealHotspot().contains((sf::Vector2i)actor->getPosition()))
        {
            auto it = std::find_if(scalings.begin(), scalings.end(), [&object](const auto &s) -> bool {
                return s.getName() == object->getName();
            });
            if (it != scalings.end())
            {
                _pRoom->setRoomScaling(*it);
                return;
            }
        }
    }
    if (!scalings.empty())
    {
        _pRoom->setRoomScaling(scalings[0]);
    }
}

const Verb* Engine::Impl::getHoveredVerb() const {
    if (!_inputVerbsActive) return nullptr;
    if(_pRoom && _pRoom->getFullscreen() == 1)
        return nullptr;

    auto currentActorIndex = getCurrentActorIndex();
    if(currentActorIndex == -1) return nullptr;

    for (size_t i = 0; i < _verbRects.size(); i++) {
        if (_verbRects.at(i).contains((sf::Vector2i) _mousePos)) {
            auto verbId = _verbSlots.at(currentActorIndex).getVerb(1 + i).id;
            return _pEngine->getVerb(verbId);
        }
    }
    return nullptr;
}

void Engine::update(const sf::Time &el)
{
    auto gameSpeedFactor = getPreferences().getUserPreference(PreferenceNames::GameSpeedFactor, PreferenceDefaultValues::GameSpeedFactor);
    const sf::Time elapsed(sf::seconds(el.asSeconds()*gameSpeedFactor));
    _pImpl->stopThreads();
    _pImpl->_mousePos = _pImpl->_pWindow->mapPixelToCoords(sf::Mouse::getPosition(*_pImpl->_pWindow));
    if(_pImpl->_state == EngineState::Options)
    {
        _pImpl->_optionsDialog.update(elapsed);
    }
    else if(_pImpl->_state == EngineState::StartScreen)
    {
        _pImpl->_startScreenDialog.update(elapsed);
    }
    else if(_pImpl->isKeyPressed(32))
    { 
        _pImpl->_state = _pImpl->_state == EngineState::Game ? EngineState::Paused : EngineState::Game; 
        if(_pImpl->_state == EngineState::Paused)
        { 
            _pImpl->_soundManager.pauseAllSounds();
        }
        else
        {
            _pImpl->_soundManager.resumeAllSounds();
        }
    }
    
    if(_pImpl->_state == EngineState::Paused)
    {  
        _pImpl->updateKeys();
        return;
    }

    _pImpl->_talkingState.update(elapsed);

    ImGuiIO &io = ImGui::GetIO();
    _pImpl->_frameCounter++;
    auto wasMouseDown = !io.WantCaptureMouse && _pImpl->_isMouseDown;
    auto wasMouseRightDown = !io.WantCaptureMouse && _pImpl->_isMouseRightDown;
    _pImpl->_isMouseDown = !io.WantCaptureMouse && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && _pImpl->_pWindow->hasFocus();
    if(!wasMouseDown || !_pImpl->_isMouseDown)
    {
        _pImpl->_mouseDownTime = sf::seconds(0);
        _pImpl->run(false);
    }
    else 
    {
        _pImpl->_mouseDownTime += elapsed;
        if(_pImpl->_mouseDownTime > sf::seconds(0.5f))
        {
            _pImpl->run(true);
        }
    }
    _pImpl->_isMouseRightDown = sf::Mouse::isButtonPressed(sf::Mouse::Button::Right) && _pImpl->_pWindow->hasFocus();
    bool isRightClick = wasMouseRightDown != _pImpl->_isMouseRightDown && !_pImpl->_isMouseRightDown;
    auto isMouseClick = wasMouseDown != _pImpl->_isMouseDown && !_pImpl->_isMouseDown;

    _pImpl->_time += elapsed;

    _pImpl->_camera.update(elapsed);
    _pImpl->_soundManager.update(elapsed);
    _pImpl->updateCutscene(elapsed);
    _pImpl->updateFunctions(elapsed);
    _pImpl->updateSentence(elapsed);

    if (!_pImpl->_pRoom)
        return;

    _pImpl->updateRoomScalings();

    auto screen = _pImpl->_pWindow->getView().getSize();
    _pImpl->_pRoom->update(elapsed);
    if (_pImpl->_pFollowActor && _pImpl->_pFollowActor->isVisible() && _pImpl->_pFollowActor->getRoom() == getRoom())
    {
        auto pos = _pImpl->_pFollowActor->getPosition() - sf::Vector2f(screen.x / 2, screen.y / 2);
        auto margin = screen.x / 4;
        auto cameraPos = _pImpl->_camera.getAt();
        if (_pImpl->_camera.isMoving() || (cameraPos.x > pos.x + margin) || (cameraPos.x < pos.x - margin))
        {
            _pImpl->_camera.panTo(pos, sf::seconds(4), InterpolationMethod::EaseOut);
        }
    }

    _pImpl->updateActorIcons(elapsed);

    if(_pImpl->_state == EngineState::Options)
        return;

    _pImpl->_cursorDirection = CursorDirection::None;
    _pImpl->updateMouseCursor();

    _pImpl->_mousePosInRoom = _pImpl->_mousePos + _pImpl->_camera.getAt();

    _pImpl->_inventory.setMousePosition(_pImpl->_mousePos);
    _pImpl->_dialogManager.update(elapsed);
    _pImpl->_pHoveredEntity = _pImpl->getHoveredEntity(_pImpl->_mousePosInRoom);
    _pImpl->updateHoveredEntity(isRightClick);

    if (_pImpl->_pCurrentActor)
    {
        auto &objects = _pImpl->_pCurrentActor->getObjects();
        for (auto &object : objects)
        {
            object->update(elapsed);
        }
    }

    if (!_pImpl->_inputActive)
        return;

    if(_pImpl->_inventory.update(elapsed))
        return;

    _pImpl->updateKeyboard();
    _pImpl->updateKeys();

    if (_pImpl->_dialogManager.getState() != DialogManagerState::None)
        return;

    if (_pImpl->_actorIcons.isMouseOver())
        return;

    if (isMouseClick && _pImpl->clickedAt(_pImpl->_mousePosInRoom))
        return;

    if(!_pImpl->_pCurrentActor)
        return;

    if (!isMouseClick && !isRightClick && !_pImpl->_isMouseDown)
        return;

    stopSentence();

    const auto* pVerb = _pImpl->getHoveredVerb();
    // input click on a verb ?
    if (pVerb) {
        _pImpl->onVerbClick(pVerb);
        return;
    }

    if (!isMouseClick && !isRightClick)
    {
        if(!pVerb && !_pImpl->_pHoveredEntity)
            _pImpl->_pCurrentActor->walkTo(_pImpl->_mousePosInRoom);
        return;
    }

    if (_pImpl->_pHoveredEntity)
    {
        ScriptEngine::rawCall("onObjectClick", _pImpl->_pHoveredEntity);
        auto pVerbOverride = _pImpl->_pVerbOverride;
        if (!pVerbOverride)
        {
            pVerbOverride = _pImpl->_pVerb;
        }
        if (_pImpl->_pObj1)
        {
            _pImpl->_pVerbExecute->execute(pVerbOverride, _pImpl->_pObj1, _pImpl->_pObj2);
        }
        return;
    }

    _pImpl->_pCurrentActor->walkTo(_pImpl->_mousePosInRoom);
    setDefaultVerb();
}

void Engine::setCurrentActor(Actor *pCurrentActor, bool userSelected)
{
    _pImpl->_pCurrentActor = pCurrentActor;
    if (_pImpl->_pCurrentActor)
    {
        follow(_pImpl->_pCurrentActor);
    }

    int currentActorIndex = _pImpl->getCurrentActorIndex();
    setVerbColor(_pImpl->_verbUiColors.at(currentActorIndex).verbHighlight);
    setVerbShadowColor(_pImpl->_verbUiColors.at(currentActorIndex).verbNormalTint);
    setVerbNormalColor(_pImpl->_verbUiColors.at(currentActorIndex).verbHighlight);
    setVerbHighlightColor(_pImpl->_verbUiColors.at(currentActorIndex).verbHighlightTint);

    ScriptEngine::rawCall("onActorSelected", pCurrentActor, userSelected);
}

void Engine::Impl::updateKeys()
{
    _oldKeyDowns.clear();
    for(auto key : _newKeyDowns)
    {
        _oldKeyDowns.insert(key);
    }
    _newKeyDowns.clear();
}

bool Engine::Impl::isKeyPressed(int key)
{
    auto wasDown = _oldKeyDowns.find(key) != _oldKeyDowns.end();
    auto isDown = _newKeyDowns.find(key) != _newKeyDowns.end();
    return wasDown && !isDown;
}

int Engine::Impl::toKey(const std::string& keyText)
{
    if(keyText.length() == 1)
    {
        return keyText[0];
    }
    return 0;
}

void Engine::Impl::updateKeyboard()
{
    ImGuiIO &io = ImGui::GetIO();
    if(io.WantTextInput) return;

    if(_oldKeyDowns.empty()) return;

    if(_pRoom)
    {
        for(auto key : _oldKeyDowns)
        {
            if(isKeyPressed(key))
            {
                ScriptEngine::rawCall(_pRoom, "pressedKey", key);
            }
        }
    }

    int currentActorIndex = getCurrentActorIndex();
    if (currentActorIndex == -1) return;

    const auto& verbSlot = _verbSlots.at(currentActorIndex);
    for(auto i = 0; i < 10; i++)
    {
        const auto& verb = verbSlot.getVerb(i);
        if(verb.key.length() == 0) continue;
        auto id = std::strtol(verb.key.substr(1,verb.key.length()-1).c_str(), nullptr, 10);
        auto key = toKey(tostring(_pEngine->getText(id)));
        if(isKeyPressed(key))
        {
            onVerbClick(&verb);
        }
    }
}

void Engine::Impl::onVerbClick(const Verb* pVerb)
{
    _pVerb = pVerb;
    _useFlag = UseFlag::None;
    _pUseObject = nullptr;
    _pObj1 = nullptr;
    _pObj2 = nullptr;

    ScriptEngine::rawCall("onVerbClick");
}

bool Engine::Impl::clickedAt(const sf::Vector2f &pos)
{
    if (!_pRoom)
        return false;

    bool handled = false;
    ScriptEngine::rawCallFunc(handled, _pRoom, "clickedAt", pos.x, pos.y);
    return handled;
}

void Engine::draw(sf::RenderWindow &window) const
{
    if (_pImpl->_pRoom)
    {
        _pImpl->_pRoom->draw(window, _pImpl->_camera.getAt());
        _pImpl->drawFade(window);
        _pImpl->_pRoom->drawForeground(window, _pImpl->_camera.getAt());

        _pImpl->drawWalkboxes(window);

        window.draw(_pImpl->_talkingState);

        window.draw(_pImpl->_dialogManager);

        if ((_pImpl->_dialogManager.getState() == DialogManagerState::None))
        {
            if (_pImpl->_inputHUD && _pImpl->_pRoom->getFullscreen() != 1)
            {
                _pImpl->drawVerbs(window);
                window.draw(_pImpl->_inventory);
                if(_pImpl->_inputActive) window.draw(_pImpl->_actorIcons);
            }
        }

        if(_pImpl->_state == EngineState::Options)
        {
            window.draw(_pImpl->_optionsDialog);
        }
        else if(_pImpl->_state == EngineState::StartScreen)
        {
            window.draw(_pImpl->_startScreenDialog);
        }
        _pImpl->drawPause(window);

        _pImpl->drawCursor(window);
        _pImpl->drawCursorText(window);
        _pImpl->_pDebugTools->render();
    }
}

void Engine::setWalkboxesFlags(int show) { _pImpl->_showDrawWalkboxes = show; }

int Engine::getWalkboxesFlags() const { return _pImpl->_showDrawWalkboxes; }

void Engine::Impl::drawWalkboxes(sf::RenderTarget &target) const
{
    if (!_pRoom || _showDrawWalkboxes == 0)
        return;

    auto screen = target.getView().getSize();
    auto w = screen.x/2.f;
    auto h = screen.y/2.f;
    sf::Transform t;
    t.rotate(_pRoom->getRotation(), w, h);
    t.translate(-_camera.getAt());
    sf::RenderStates states;
    states.transform = t;

    if(_showDrawWalkboxes&4) {
        for (const auto &walkbox : _pRoom->getWalkboxes()) {
            _WalkboxDrawable wd(walkbox);
            target.draw(wd, states);
        }
    }

    if(_showDrawWalkboxes&1) {
        for (const auto &walkbox : _pRoom->getGraphWalkboxes()) {
            _WalkboxDrawable wd(walkbox);
            target.draw(wd, states);
        }
    }

    if(_showDrawWalkboxes&2) {
        const auto *pGraph = _pRoom->getGraph();
        if (pGraph) {
            target.draw(*pGraph, states);
        }
    }
}

void Engine::Impl::drawPause(sf::RenderTarget &target) const
{
    if(_state != EngineState::Paused) return;

    const auto view = target.getView();
    auto viewRect = sf::FloatRect(0, 0, 320, 176);
    target.setView(sf::View(viewRect));

    auto viewCenter = sf::Vector2f(viewRect.width/2,viewRect.height/2);
    auto rect = _saveLoadSheet.getRect("pause_dialog");

    sf::Sprite sprite;
    sprite.setPosition(viewCenter);
    sprite.setTexture(_saveLoadSheet.getTexture());
    sprite.setOrigin(rect.width/2,rect.height/2);
    sprite.setTextureRect(rect);
    target.draw(sprite);

    viewRect = sf::FloatRect(0, 0, Screen::Width, Screen::Height);
    viewCenter = sf::Vector2f(viewRect.width/2,viewRect.height/2);
    target.setView(sf::View(viewRect));

    auto retroFonts = _pEngine->getPreferences().getUserPreference(PreferenceNames::RetroFonts, PreferenceDefaultValues::RetroFonts);
    const Font& font = _pEngine->getTextureManager().getFont(retroFonts ? "FontRetroSheet": "FontModernSheet");

    NGText text;
    auto screen = target.getView().getSize();
    auto scale = screen.y / 512.f;
    text.setScale(scale, scale);
    text.setAlignment(NGTextAlignment::Center);
    text.setPosition(viewCenter);
    text.setFont(font);
    text.setColor(sf::Color::White);
    text.setText(_pEngine->getText(99951));
    auto bounds = text.getBoundRect();
    text.move(0, -bounds.height);
    target.draw(text);
    
    target.setView(view);
}

void Engine::Impl::stopThreads()
{
    _threads.erase(std::remove_if(_threads.begin(), _threads.end(), [](const auto &t) -> bool {
        return !t || t->isStopped(); }), _threads.end());
}

void Engine::Impl::drawCursor(sf::RenderWindow &window) const
{
    if (!_showCursor)
        return;

    auto screen = _pWindow->getView().getSize();
    auto cursorSize = sf::Vector2f(68.f * screen.x / 1284, 68.f * screen.y / 772);
    
    sf::RectangleShape shape;
    shape.setPosition(_mousePos);
    shape.setOrigin(cursorSize / 2.f);
    shape.setSize(cursorSize);
    shape.setTexture(&_gameSheet.getTexture());
    shape.setTextureRect(getCursorRect());
    window.draw(shape);
}

sf::IntRect Engine::Impl::getCursorRect() const
{
    if(_state == EngineState::Paused)
        return _gameSheet.getRect("cursor_pause");

    if(_state == EngineState::Options)
        return _gameSheet.getRect("cursor");
    
    if (_dialogManager.getState() != DialogManagerState::None)
        return _gameSheet.getRect("cursor");

    if (_cursorDirection & CursorDirection::Left)
    {
        return _cursorDirection & CursorDirection::Hotspot ? _gameSheet.getRect("hotspot_cursor_left")
                                                           : _gameSheet.getRect("cursor_left");
    }
    if (_cursorDirection & CursorDirection::Right)
    {
        return _cursorDirection & CursorDirection::Hotspot ? _gameSheet.getRect("hotspot_cursor_right")
                                                           : _gameSheet.getRect("cursor_right");
    }
    if (_cursorDirection & CursorDirection::Up)
    {
        return _cursorDirection & CursorDirection::Hotspot ? _gameSheet.getRect("hotspot_cursor_back")
                                                           : _gameSheet.getRect("cursor_back");
    }
    if (_cursorDirection & CursorDirection::Down)
    {
        return (_cursorDirection & CursorDirection::Hotspot) ? _gameSheet.getRect("hotspot_cursor_front")
                                                             : _gameSheet.getRect("cursor_front");
    }
    return (_cursorDirection & CursorDirection::Hotspot) ? _gameSheet.getRect("hotspot_cursor")
                                                         : _gameSheet.getRect("cursor");
}

std::wstring Engine::Impl::getDisplayName(const std::wstring& name) const
{ 
    auto len = name.length();
    if(len>2 && name[len-2]=='#') {
        return name.substr(0, len - 2);
    }
    return name;
}

void Engine::Impl::drawCursorText(sf::RenderTarget &target) const
{
    if (!_showCursor ||  _state != EngineState::Game)
        return;

    if (_dialogManager.getState() != DialogManagerState::None)
        return;

    auto pVerb = _pVerbOverride;
    if (!pVerb)
        pVerb = _pVerb;
    if (!pVerb)
        return;

    auto currentActorIndex = getCurrentActorIndex();
    if(currentActorIndex == -1)
        return;

    auto retroFonts = _pEngine->getPreferences().getUserPreference(PreferenceNames::RetroFonts, PreferenceDefaultValues::RetroFonts);
    const Font& font = _pEngine->getTextureManager().getFont(retroFonts ? "FontRetroSheet": "FontModernSheet");

    NGText text;
    auto screen = target.getView().getSize();
    auto scale = screen.y / (2.f * 512.f);
    text.setAlignment(NGTextAlignment::Center);
    text.setScale(scale, scale);
    text.setFont(font);
    text.setColor(_verbUiColors.at(currentActorIndex).sentence);

    std::wstring s;
    if (pVerb->id != VerbConstants::VERB_WALKTO || _pHoveredEntity)
    {
        auto id = std::strtol(pVerb->text.substr(1).data(), nullptr, 10);
        s.append(_pEngine->getText(id));
    }
    if (_pObj1)
    {
        s.append(L" ").append(getDisplayName(_pEngine->getText(_pObj1->getName())));
    }
    appendUseFlag(s);
    if (_pObj2)
    {
        s.append(L" ").append(getDisplayName(_pEngine->getText(_pObj2->getName())));
    }
    text.setText(s);

    // do display cursor position:
    // auto mousePosInRoom = _mousePos + _camera.getAt();
    // std::wstringstream ss;
    // std::wstring txt = text.getText();
    // ss << txt << L" (" << std::fixed << std::setprecision(0) << mousePosInRoom.x << L"," << mousePosInRoom.y << L")";
    // text.setText(ss.str());

    auto y = _mousePos.y - 22 < 8 ? _mousePos.y + 8 : _mousePos.y - 22;
    if (y < 0)
        y = 0;
    auto x = std::clamp((int)_mousePos.x, 20, (int)(screen.x - 20 - (int)text.getBoundRect().width / 2));
    text.setPosition(x, y);
    target.draw(text, sf::RenderStates::Default);
}

void Engine::Impl::appendUseFlag(std::wstring &sentence) const
{
    switch (_useFlag)
    {
        case UseFlag::UseWith:
            sentence.append(L" ").append(_pEngine->getText(10000));
            break;
        case UseFlag::UseOn:
            sentence.append(L" ").append(_pEngine->getText(10001));
            break;
        case UseFlag::UseIn:
            sentence.append(L" ").append(_pEngine->getText(10002));
            break;
        case UseFlag::GiveTo:
            sentence.append(L" ").append(_pEngine->getText(10003));
            break;
        case UseFlag::None:
            break;
    }
}

int Engine::Impl::getCurrentActorIndex() const
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

void Engine::setRanges(sf::Vector2f ranges) { _pImpl->_ranges = ranges; }

sf::Vector2f Engine::getRanges() const{ return _pImpl->_ranges; }

void Engine::setVerbColor(sf::Color color) { _pImpl->_verbColor = color; }
sf::Color Engine::getVerbColor() const { return _pImpl->_verbColor; }

void Engine::setVerbShadowColor(sf::Color color) { _pImpl->_verbShadowColor = color; }
sf::Color Engine::getVerbShadowColor() const { return _pImpl->_verbShadowColor; }

void Engine::setVerbNormalColor(sf::Color color){ _pImpl->_verbNormalColor = color; }
sf::Color Engine::getVerbNormalColor() const { return _pImpl->_verbNormalColor; }

void Engine::setVerbHighlightColor(sf::Color color){ _pImpl->_verbHighlightColor = color; }
sf::Color Engine::getVerbHighlightColor() const { return _pImpl->_verbHighlightColor; }

void Engine::Impl::drawVerbs(sf::RenderWindow &window) const
{
    if (!_inputVerbsActive)
        return;

    if(_pRoom && _pRoom->getFullscreen() == 1)
        return;

    int currentActorIndex = getCurrentActorIndex();
    if (currentActorIndex == -1 || _verbSlots.at(currentActorIndex).getVerb(0).id == 0)
        return;

    auto pVerb = _pVerbOverride;
    if (!pVerb)
    {
        pVerb = _pVerb;
    }
    auto verbId = pVerb->id;
    if (_pHoveredEntity && verbId == VerbConstants::VERB_WALKTO)
    {
        verbId = getDefaultVerb(_vm, _pHoveredEntity);
    }
    else if(_state == EngineState::Game)
    {
        for (size_t i = 0; i < _verbRects.size(); i++)
        {
            if (_verbRects.at(i).contains((sf::Vector2i)_mousePos))
            {
                verbId = _verbSlots.at(currentActorIndex).getVerb(1 + i).id;
                break;
            }
        }
    }

    const auto view = window.getView();
    window.setView(sf::View(sf::FloatRect(0, 0, Screen::Width, Screen::Height)));

    // draw UI background
    auto hudSentence = _preferences.getUserPreference(PreferenceNames::HudSentence, PreferenceDefaultValues::HudSentence);
    auto uiBackingAlpha = _preferences.getUserPreference(PreferenceNames::UiBackingAlpha, PreferenceDefaultValues::UiBackingAlpha);
    auto invertVerbHighlight = _preferences.getUserPreference(PreferenceNames::InvertVerbHighlight, PreferenceDefaultValues::InvertVerbHighlight);
    auto verbHighlight = invertVerbHighlight ? sf::Color::White : _verbColor;
    auto verbColor = invertVerbHighlight ? _verbColor : sf::Color::White;
    auto uiBackingRect = hudSentence ? _gameSheet.getRect("ui_backing_tall") : _gameSheet.getRect("ui_backing");
    sf::Sprite uiBacking;
    uiBacking.setColor(sf::Color(0, 0, 0, uiBackingAlpha * 255));
    uiBacking.setPosition(0, 720.f - uiBackingRect.height);
    uiBacking.setTexture(_gameSheet.getTexture());
    uiBacking.setTextureRect(uiBackingRect);
    window.draw(uiBacking);

    // draw verbs
    _verbShader.setUniform("ranges", _ranges);
    _verbShader.setUniform("shadowColor", sf::Glsl::Vec4(_verbShadowColor));
    _verbShader.setUniform("normalColor", sf::Glsl::Vec4(_verbNormalColor));
    _verbShader.setUniform("highlightColor", sf::Glsl::Vec4(_verbHighlightColor));
    
    sf::RenderStates verbStates;
    verbStates.shader = &_verbShader;
    for (int i = 1; i <= 9; i++)
    {
        auto verb = _verbSlots.at(currentActorIndex).getVerb(i);
        _verbShader.setUniform("color", sf::Glsl::Vec4(verb.id == verbId ? verbHighlight : verbColor));

        auto verbName = getVerbName(verb);
        auto rect = _verbSheet.getRect(verbName);
        auto s = _verbSheet.getSpriteSourceSize(verbName);
        sf::Sprite verbSprite;
        verbSprite.setPosition(s.left, s.top);
        verbSprite.setTexture(_verbSheet.getTexture());
        verbSprite.setTextureRect(rect);
        window.draw(verbSprite, verbStates);
    }

    window.setView(view);
}

void Engine::startDialog(const std::string &dialog, const std::string &node)
{
    _pImpl->_dialogManager.start(dialog, node);
}

void Engine::execute(const std::string &code) { _pImpl->_pScriptExecute->execute(code); }

SoundDefinition *Engine::getSoundDefinition(const std::string &name)
{
    return _pImpl->_pScriptExecute->getSoundDefinition(name);
}

bool Engine::executeCondition(const std::string &code) { return _pImpl->_pScriptExecute->executeCondition(code); }

std::string Engine::executeDollar(const std::string &code) { return _pImpl->_pScriptExecute->executeDollar(code); }

void Engine::addSelectableActor(int index, Actor *pActor)
{
    _pImpl->_actorsIconSlots.at(index - 1).selectable = true;
    _pImpl->_actorsIconSlots.at(index - 1).pActor = pActor;
}

void Engine::actorSlotSelectable(Actor *pActor, bool selectable)
{
    auto it = std::find_if(_pImpl->_actorsIconSlots.begin(), _pImpl->_actorsIconSlots.end(),
                           [&pActor](auto &selectableActor) -> bool { return selectableActor.pActor == pActor; });
    if (it != _pImpl->_actorsIconSlots.end())
    {
        it->selectable = selectable;
    }
}

void Engine::actorSlotSelectable(int index, bool selectable)
{
    _pImpl->_actorsIconSlots.at(index - 1).selectable = selectable;
}

bool Engine::isActorSelectable(Actor* pActor) const
{
    for (auto &&slot : _pImpl->_actorsIconSlots)
    {
        if(slot.pActor == pActor)
            return slot.selectable;
    }
    return false;
}

void Engine::setActorSlotSelectable(ActorSlotSelectableMode mode) { _pImpl->_actorIcons.setMode(mode); }

void Engine::setUseFlag(UseFlag flag, Entity *object)
{
    _pImpl->_useFlag = flag;
    _pImpl->_pUseObject = object;
}

void Engine::cutsceneOverride()
{
    if (!_pImpl->_pCutscene)
        return;
    _pImpl->_pCutscene->cutsceneOverride();
}

void Engine::cutscene(std::unique_ptr<Cutscene> function)
{
    _pImpl->_pCutscene = function.get();
    addThread(std::move(function));
}

Cutscene* Engine::getCutscene() const { return _pImpl->_pCutscene; }

bool Engine::inCutscene() const { return _pImpl->_pCutscene && !_pImpl->_pCutscene->isElapsed(); }

HSQOBJECT &Engine::getDefaultObject() { return _pImpl->_pDefaultObject; }

void Engine::flashSelectableActor(bool on) { _pImpl->_actorIcons.flash(on); }

const Verb *Engine::getActiveVerb() const { return _pImpl->_pVerb; }

void Engine::setFadeAlpha(float fade) { _pImpl->_fadeColor.a = static_cast<uint8_t>(fade * 255); }

float Engine::getFadeAlpha() const { return _pImpl->_fadeColor.a / 255.f; }

void Engine::fadeTo(float destination, sf::Time time, InterpolationMethod method)
{
    auto m = ScriptEngine::getInterpolationMethod(method);
    auto get = [this]() -> float {return getFadeAlpha();};
    auto set = [this](const float& a){ setFadeAlpha(a);};
    auto f = std::make_unique<ChangeProperty<float>>(get, set, destination, time, m);
    addFunction(std::move(f));
}

void Engine::pushSentence(int id, Entity* pObj1, Entity* pObj2)
{
    const Verb* pVerb = getVerb(id);
    if(!pVerb) return;
    _pImpl->_pVerbExecute->execute(pVerb, pObj1, pObj2);
}

void Engine::setSentence(std::unique_ptr<Sentence> sentence)
{
    _pImpl->_pSentence = std::move(sentence);
}

void Engine::stopSentence()
{
    if(!_pImpl->_pSentence) return;
    _pImpl->_pSentence->stop();
    _pImpl->_pSentence.reset();
}

void Engine::keyDown(int key)
{
    _pImpl->_newKeyDowns.insert(key);
}

void Engine::keyUp(int key)
{
    auto it = _pImpl->_newKeyDowns.find(key);
    if(it == _pImpl->_newKeyDowns.end()) return;
    _pImpl->_newKeyDowns.erase(it);
}

void Engine::sayLineAt(sf::Vector2i pos, sf::Color color, sf::Time duration, const std::string& text)
{
    _pImpl->_talkingState.setTalkColor(color);
    auto y = getRoom()->getRoomSize().y;
    sf::Vector2f p(pos.x, y - pos.y);
    _pImpl->_talkingState.setPosition(p);
    _pImpl->_talkingState.setText(getText(text));
    _pImpl->_talkingState.setDuration(duration);
}

void Engine::sayLineAt(sf::Vector2i pos, Actor& actor, const std::string& text)
{
    _pImpl->_talkingState.setPosition((sf::Vector2f)pos);
    _pImpl->_talkingState.loadLip(text, &actor);
}

void Engine::showOptions(bool visible)
{
    _pImpl->_state = visible ? EngineState::Options : EngineState::Game;
}

void Engine::quit()
{
    _pImpl->_pWindow->close();
}

void Engine::run()
{
    std::ifstream is("engge.nut");
    if (is.is_open())
    {
        info("execute engge.nut");
        _pImpl->_state = EngineState::Game;
        ScriptEngine::executeScript("engge.nut");
        return;
    }

    ng::info("execute boot script");
    ScriptEngine::executeNutScript("Defines.nut");
    ScriptEngine::executeNutScript("Boot.nut");
    execute("cameraInRoom(StartScreen)");
}

} // namespace ng
