#include <squirrel.h>
#include "engge/Engine/Engine.hpp"
#include "engge/Engine/ActorIconSlot.hpp"
#include "engge/Engine/ActorIcons.hpp"
#include "engge/Engine/Camera.hpp"
#include "engge/Engine/Cutscene.hpp"
#include "engge/Engine/Hud.hpp"
#include "engge/Input/InputConstants.hpp"
#include "engge/Dialog/DialogManager.hpp"
#include "engge/Engine/Inventory.hpp"
#include "engge/Engine/Preferences.hpp"
#include "engge/Room/Room.hpp"
#include "engge/Room/RoomScaling.hpp"
#include "engge/Scripting/ScriptEngine.hpp"
#include "engge/Scripting/ScriptExecute.hpp"
#include "engge/Engine/Sentence.hpp"
#include "engge/Audio/SoundDefinition.hpp"
#include "engge/Audio/SoundManager.hpp"
#include "engge/Graphics/SpriteSheet.hpp"
#include "engge/Engine/TextDatabase.hpp"
#include "engge/Engine/Verb.hpp"
#include "engge/Scripting/VerbExecute.hpp"
#include "../System/_DebugTools.hpp"
#include "../Entities/Actor/_TalkingState.hpp"
#include "engge/System/Logger.hpp"
#include <cmath>
#include <ctime>
#include <cwchar>
#include <filesystem>
#include <iomanip>
#include <memory>
#include <set>
#include <string>
#include <unordered_set>
#include <ngf/Application.h>
#include <ngf/Graphics/Colors.h>
#include "engge/Engine/InputStateConstants.hpp"
#include "EngineImpl.hpp"
#include <ngf/System/Mouse.h>

namespace fs = std::filesystem;

namespace ng {

Engine::Engine() : _pImpl(std::make_unique<Impl>()) {
  _pImpl->_pEngine = this;
  _pImpl->_soundManager.setEngine(this);
  _pImpl->_dialogManager.setEngine(this);
  _pImpl->_actorIcons.setEngine(this);
  _pImpl->_camera.setEngine(this);
  _pImpl->_talkingState.setEngine(this);

  // load all messages
  std::stringstream s;
  auto lang =
      _pImpl->_preferences.getUserPreference<std::string>(PreferenceNames::Language, PreferenceDefaultValues::Language);
  s << "ThimbleweedText_" << lang << ".tsv";
  Locator<TextDatabase>::get().load(s.str());

  _pImpl->_optionsDialog.setSaveEnabled(true);
  _pImpl->_optionsDialog.setEngine(this);
  _pImpl->_optionsDialog.setCallback([this]() {
    showOptions(false);
  });
  _pImpl->_startScreenDialog.setEngine(this);
  _pImpl->_startScreenDialog.setNewGameCallback([this]() {
    _pImpl->_state = EngineState::Game;
    _pImpl->exitRoom(nullptr);
    ScriptEngine::call("start", true);
  });
  _pImpl->_startScreenDialog.setSlotCallback([this](int slot) {
    _pImpl->_state = EngineState::Game;
    loadGame(slot);
  });

  _pImpl->_preferences.subscribe([this](const std::string &name) {
    if (name == PreferenceNames::Language) {
      auto newLang = _pImpl->_preferences.getUserPreference<std::string>(PreferenceNames::Language,
                                                                         PreferenceDefaultValues::Language);
      _pImpl->onLanguageChange(newLang);
    }
  });
}

Engine::~Engine() = default;

int Engine::getFrameCounter() const { return _pImpl->_frameCounter; }

void Engine::setApplication(ngf::Application *app) { _pImpl->_pApp = app; }

const ngf::Application *Engine::getApplication() const { return _pImpl->_pApp; }

ResourceManager &Engine::getResourceManager() { return _pImpl->_textureManager; }

Room *Engine::getRoom() { return _pImpl->_pRoom; }

std::wstring Engine::getText(int id) {
  return Locator<TextDatabase>::get().getText(id);
}

std::wstring Engine::getText(const std::string &text) {
  return Locator<TextDatabase>::get().getText(text);
}

void Engine::addActor(std::unique_ptr<Actor> actor) { _pImpl->_actors.push_back(std::move(actor)); }

void Engine::addRoom(std::unique_ptr<Room> room) { _pImpl->_rooms.push_back(std::move(room)); }

std::vector<std::unique_ptr<Room>> &Engine::getRooms() { return _pImpl->_rooms; }

void Engine::addFunction(std::unique_ptr<Function> function) { _pImpl->_newFunctions.push_back(std::move(function)); }

void Engine::addCallback(std::unique_ptr<Callback> callback) { _pImpl->_callbacks.push_back(std::move(callback)); }

void Engine::removeCallback(int id) {
  auto it = std::find_if(_pImpl->_callbacks.begin(), _pImpl->_callbacks.end(),
                         [id](auto &callback) -> bool { return callback->getId() == id; });
  if (it != _pImpl->_callbacks.end()) {
    _pImpl->_callbacks.erase(it);
  }
}

std::vector<std::unique_ptr<Actor>> &Engine::getActors() { return _pImpl->_actors; }

Actor *Engine::getCurrentActor() { return _pImpl->_pCurrentActor; }

const VerbUiColors *Engine::getVerbUiColors(const std::string &name) const {
  if (name.empty()) {
    auto index = _pImpl->getCurrentActorIndex();
    if (index == -1)
      return nullptr;
    return &_pImpl->_hud.getVerbUiColors(index);
  }
  for (int i = 0; i < static_cast<int>(_pImpl->_actorsIconSlots.size()); i++) {
    const auto &selectableActor = _pImpl->_actorsIconSlots.at(i);
    if (selectableActor.pActor && selectableActor.pActor->getKey() == name) {
      return &_pImpl->_hud.getVerbUiColors(i);
    }
  }
  return nullptr;
}

bool Engine::getInputActive() const { return _pImpl->_inputActive; }

void Engine::setInputState(int state) {
  if ((state & InputStateConstants::UI_INPUT_ON) == InputStateConstants::UI_INPUT_ON) {
    _pImpl->_inputActive = true;
  }
  if ((state & InputStateConstants::UI_INPUT_OFF) == InputStateConstants::UI_INPUT_OFF) {
    _pImpl->_inputActive = false;
  }
  if ((state & InputStateConstants::UI_VERBS_ON) == InputStateConstants::UI_VERBS_ON) {
    _pImpl->_inputVerbsActive = true;
  }
  if ((state & InputStateConstants::UI_VERBS_OFF) == InputStateConstants::UI_VERBS_OFF) {
    _pImpl->_inputVerbsActive = false;
  }
  if ((state & InputStateConstants::UI_CURSOR_ON) == InputStateConstants::UI_CURSOR_ON) {
    _pImpl->_showCursor = true;
  }
  if ((state & InputStateConstants::UI_CURSOR_OFF) == InputStateConstants::UI_CURSOR_OFF) {
    _pImpl->_showCursor = false;
  }
  if ((state & InputStateConstants::UI_HUDOBJECTS_ON) == InputStateConstants::UI_HUDOBJECTS_ON) {
    _pImpl->_inputHUD = true;
  }
  if ((state & InputStateConstants::UI_HUDOBJECTS_OFF) == InputStateConstants::UI_HUDOBJECTS_OFF) {
    _pImpl->_inputHUD = false;
  }
}

int Engine::getInputState() const {
  int inputState = 0;
  inputState |= (_pImpl->_inputActive ? InputStateConstants::UI_INPUT_ON : InputStateConstants::UI_INPUT_OFF);
  inputState |= (_pImpl->_inputVerbsActive ? InputStateConstants::UI_VERBS_ON : InputStateConstants::UI_VERBS_OFF);
  inputState |= (_pImpl->_showCursor ? InputStateConstants::UI_CURSOR_ON : InputStateConstants::UI_CURSOR_OFF);
  inputState |= (_pImpl->_inputHUD ? InputStateConstants::UI_HUDOBJECTS_ON : InputStateConstants::UI_HUDOBJECTS_OFF);
  return inputState;
}

void Engine::follow(Actor *pActor) {
  auto panCamera =
      (_pImpl->_pFollowActor && pActor && _pImpl->_pFollowActor != pActor && _pImpl->_pFollowActor->getRoom() &&
          pActor->getRoom() && _pImpl->_pFollowActor->getRoom()->getId() == pActor->getRoom()->getId());
  _pImpl->_pFollowActor = pActor;
  if (!pActor)
    return;

  auto pos = pActor->getRealPosition();
  auto screen = _pImpl->_pRoom->getScreenSize();
  setRoom(pActor->getRoom());
  if (panCamera) {
    _pImpl->_camera.panTo(pos - glm::vec2(screen.x / 2, screen.y / 2), ngf::TimeSpan::seconds(4),
                          InterpolationMethod::EaseOut);
    return;
  }
  _pImpl->_camera.at(pos - glm::vec2(screen.x / 2, screen.y / 2));
}

void Engine::setVerbExecute(std::unique_ptr<VerbExecute> verbExecute) {
  _pImpl->_pVerbExecute = std::move(verbExecute);
}

void Engine::setDefaultVerb() {
  _pImpl->_hud.setHoveredEntity(nullptr);
  auto index = _pImpl->getCurrentActorIndex();
  if (index == -1)
    return;

  const auto &verbSlot = _pImpl->_hud.getVerbSlot(index);
  _pImpl->_hud.setCurrentVerb(&verbSlot.getVerb(0));
  _pImpl->_useFlag = UseFlag::None;
  _pImpl->_pUseObject = nullptr;
  _pImpl->_objId1 = 0;
  _pImpl->_pObj2 = nullptr;
}

void Engine::setScriptExecute(std::unique_ptr<ScriptExecute> scriptExecute) {
  _pImpl->_pScriptExecute = std::move(scriptExecute);
}

void Engine::addThread(std::unique_ptr<ThreadBase> thread) { _pImpl->_threads.push_back(std::move(thread)); }

std::vector<std::unique_ptr<ThreadBase>> &Engine::getThreads() { return _pImpl->_threads; }

glm::vec2 Engine::getMousePositionInRoom() const { return _pImpl->_mousePosInRoom; }

Preferences &Engine::getPreferences() { return _pImpl->_preferences; }

SoundManager &Engine::getSoundManager() { return _pImpl->_soundManager; }

DialogManager &Engine::getDialogManager() { return _pImpl->_dialogManager; }

Camera &Engine::getCamera() { return _pImpl->_camera; }

ngf::TimeSpan Engine::getTime() const { return _pImpl->_time; }

SQInteger Engine::setRoom(Room *pRoom) {
  if (!pRoom)
    return 0;

  _pImpl->_fade = 0.f;

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

SQInteger Engine::enterRoomFromDoor(Object *pDoor) {
  auto dir = pDoor->getUseDirection();
  Facing facing = Facing::FACE_FRONT;
  if (dir.has_value()) {
    switch (dir.value()) {
    case UseDirection::Back:facing = Facing::FACE_FRONT;
      break;
    case UseDirection::Front:facing = Facing::FACE_BACK;
      break;
    case UseDirection::Left:facing = Facing::FACE_RIGHT;
      break;
    case UseDirection::Right:facing = Facing::FACE_LEFT;
      break;
    default:throw std::invalid_argument("direction is invalid");
    }
  }
  auto pRoom = pDoor->getRoom();
  auto result = _pImpl->exitRoom(nullptr);
  if (SQ_FAILED(result))
    return result;

  _pImpl->setCurrentRoom(pRoom);

  auto actor = getCurrentActor();
  actor->getCostume().setFacing(facing);
  actor->setRoom(pRoom);
  auto pos = pDoor->getRealPosition();
  auto usePos = pDoor->getUsePosition().value_or(glm::vec2());
  pos += usePos;
  actor->setPosition(pos);

  if (pRoom->getFullscreen() != 1) {
    _pImpl->_camera.at(pos);
  }

  return _pImpl->enterRoom(pRoom, pDoor);
}

void Engine::setInputHUD(bool on) { _pImpl->_inputHUD = on; }

void Engine::setInputActive(bool active) {
  if (inCutscene())
    return;
  _pImpl->_inputActive = active;
  _pImpl->_showCursor = active;
}

void Engine::inputSilentOff() { _pImpl->_inputActive = false; }

void Engine::setInputVerbs(bool on) { _pImpl->_inputVerbsActive = on; }

void Engine::update(const ngf::TimeSpan &el) {
  auto gameSpeedFactor =
      getPreferences().getUserPreference(PreferenceNames::GameSpeedFactor, PreferenceDefaultValues::GameSpeedFactor);
  const ngf::TimeSpan elapsed(ngf::TimeSpan::seconds(el.getTotalSeconds() * gameSpeedFactor));
  _pImpl->stopThreads();
  auto view = ngf::View{ngf::frect::fromPositionSize({0, 0}, _pImpl->_pRoom->getScreenSize())};
  _pImpl->_mousePos = _pImpl->_pApp->getRenderTarget()->mapPixelToCoords(ngf::Mouse::getPosition(), view);
  if (_pImpl->_pRoom && _pImpl->_pRoom->getName() != "Void") {
    auto screenSize = _pImpl->_pRoom->getScreenSize();
    auto screenMouse = toDefaultView((glm::ivec2) _pImpl->_mousePos, screenSize);
    _pImpl->_hud.setMousePosition(screenMouse);
    _pImpl->_dialogManager.setMousePosition(screenMouse);
  }
  if (_pImpl->_state == EngineState::Options) {
    _pImpl->_optionsDialog.update(elapsed);
  } else if (_pImpl->_state == EngineState::StartScreen) {
    _pImpl->_startScreenDialog.update(elapsed);
  }

  if (_pImpl->_state == EngineState::Paused) {
    _pImpl->updateKeys();
    return;
  }

  _pImpl->_talkingState.update(elapsed);

  _pImpl->_frameCounter++;
  auto wasMouseDown = _pImpl->_isMouseDown;
  auto wasMouseRightDown = _pImpl->_isMouseRightDown;
  _pImpl->_isMouseDown =
      ngf::Mouse::isButtonPressed(ngf::Mouse::Button::Left);
  if (!wasMouseDown || !_pImpl->_isMouseDown) {
    _pImpl->_mouseDownTime = ngf::TimeSpan::seconds(0);
    _pImpl->run(false);
  } else {
    _pImpl->_mouseDownTime += elapsed;
    if (_pImpl->_mouseDownTime > ngf::TimeSpan::seconds(0.5f)) {
      _pImpl->run(true);
    }
  }
  _pImpl->_isMouseRightDown = ngf::Mouse::isButtonPressed(ngf::Mouse::Button::Right);
  bool isRightClick = wasMouseRightDown != _pImpl->_isMouseRightDown && !_pImpl->_isMouseRightDown;
  auto isMouseClick = wasMouseDown != _pImpl->_isMouseDown && !_pImpl->_isMouseDown;

  _pImpl->_time += elapsed;
  _pImpl->_noOverrideElapsed += elapsed;

  _pImpl->_camera.update(elapsed);
  _pImpl->_soundManager.update(elapsed);
  _pImpl->updateCutscene(elapsed);
  _pImpl->updateFunctions(elapsed);
  _pImpl->updateSentence(elapsed);
  _pImpl->updateKeys();

  if (!_pImpl->_pRoom || _pImpl->_pRoom->getName() == "Void")
    return;

  _pImpl->updateRoomScalings();

  auto screen = _pImpl->_pRoom->getScreenSize();
  _pImpl->_pRoom->update(elapsed);
  for (auto &pActor : _pImpl->_actors) {
    if (!pActor || pActor->getRoom() == _pImpl->_pRoom)
      continue;
    pActor->update(elapsed);
  }

  if (_pImpl->_pFollowActor && _pImpl->_pFollowActor->isVisible() && _pImpl->_pFollowActor->getRoom() == getRoom()) {
    auto pos = _pImpl->_pFollowActor->getPosition() - glm::vec2(screen.x / 2, screen.y / 2);
    auto margin = glm::vec2(screen.x / 4, screen.y / 4);
    auto cameraPos = _pImpl->_camera.getAt();
    if (_pImpl->_camera.isMoving() || (cameraPos.x > pos.x + margin.x) || (cameraPos.x < pos.x - margin.x) ||
        (cameraPos.y > pos.y + margin.y) || (cameraPos.y < pos.y - margin.y)) {
      _pImpl->_camera.panTo(pos, ngf::TimeSpan::seconds(4), InterpolationMethod::EaseOut);
    }
  }

  _pImpl->updateActorIcons(elapsed);

  if (_pImpl->_state == EngineState::Options)
    return;

  _pImpl->_cursorDirection = CursorDirection::None;
  _pImpl->updateMouseCursor();

  auto mousePos =
      glm::vec2(_pImpl->_mousePos.x, _pImpl->_pRoom->getScreenSize().y - _pImpl->_mousePos.y);
  _pImpl->_mousePosInRoom = mousePos + _pImpl->_camera.getAt();

  _pImpl->_dialogManager.update(elapsed);

  _pImpl->_hud.setActive(_pImpl->_inputVerbsActive && _pImpl->_dialogManager.getState() == DialogManagerState::None
                             && _pImpl->_pRoom->getFullscreen() != 1);
  _pImpl->_hud.setHoveredEntity(_pImpl->getHoveredEntity(_pImpl->_mousePosInRoom));
  _pImpl->updateHoveredEntity(isRightClick);

  if (_pImpl->_pCurrentActor) {
    auto &objects = _pImpl->_pCurrentActor->getObjects();
    for (auto &object : objects) {
      object->update(elapsed);
    }
  }

  _pImpl->_hud.update(elapsed);

  if (_pImpl->_actorIcons.isMouseOver())
    return;

  if (isMouseClick && _pImpl->clickedAt(_pImpl->_mousePosInRoom))
    return;

  if (!_pImpl->_inputActive)
    return;

  _pImpl->updateKeyboard();

  if (_pImpl->_dialogManager.getState() != DialogManagerState::None) {
    auto rightClickSkipsDialog = getPreferences().getUserPreference(PreferenceNames::RightClickSkipsDialog,
                                                                    PreferenceDefaultValues::RightClickSkipsDialog);
    if (rightClickSkipsDialog && isRightClick) {
      _pImpl->skipText();
    }
    return;
  }

  if (!_pImpl->_pCurrentActor)
    return;

  if (!isMouseClick && !isRightClick && !_pImpl->_isMouseDown)
    return;

  _pImpl->_hud.setVisible(true);
  _pImpl->_actorIcons.setVisible(true);
  _pImpl->_cursorVisible = true;
  stopSentence();

  const auto *pVerb = _pImpl->getHoveredVerb();
  // input click on a verb ?
  if (_pImpl->_hud.getActive() && pVerb) {
    _pImpl->onVerbClick(pVerb);
    return;
  }

  if (!isMouseClick && !isRightClick) {
    if (!pVerb && !_pImpl->_hud.getHoveredEntity())
      _pImpl->_pCurrentActor->walkTo(_pImpl->_mousePosInRoom);
    return;
  }

  if (_pImpl->_hud.getHoveredEntity()) {
    ScriptEngine::rawCall("onObjectClick", _pImpl->_hud.getHoveredEntity());
    auto pVerbOverride = _pImpl->_hud.getVerbOverride();
    if (!pVerbOverride) {
      pVerbOverride = _pImpl->_hud.getCurrentVerb();
    }
    pVerbOverride = _pImpl->overrideVerb(pVerbOverride);
    auto pObj1 = EntityManager::getScriptObjectFromId<Entity>(_pImpl->_objId1);
    pObj1 = pVerbOverride->id == VerbConstants::VERB_TALKTO ? _pImpl->getEntity(pObj1) : pObj1;
    auto pObj2 = pVerbOverride->id == VerbConstants::VERB_GIVE ? _pImpl->getEntity(_pImpl->_pObj2) : _pImpl->_pObj2;
    if (pObj1) {
      _pImpl->_pVerbExecute->execute(pVerbOverride, pObj1, pObj2);
    }
    return;
  }

  if (_pImpl->_hud.isMouseOver())
    return;

  _pImpl->_pCurrentActor->walkTo(_pImpl->_mousePosInRoom);
  setDefaultVerb();
}

void Engine::setCurrentActor(Actor *pCurrentActor, bool userSelected) {
  _pImpl->_pCurrentActor = pCurrentActor;

  int currentActorIndex = _pImpl->getCurrentActorIndex();
  _pImpl->_hud.setCurrentActorIndex(currentActorIndex);
  _pImpl->_hud.setCurrentActor(_pImpl->_pCurrentActor);

  ScriptEngine::rawCall("onActorSelected", pCurrentActor, userSelected);
  auto pRoom = pCurrentActor ? pCurrentActor->getRoom() : nullptr;
  if (pRoom) {
    if (ScriptEngine::rawExists(pRoom, "onActorSelected")) {
      ScriptEngine::rawCall(pRoom, "onActorSelected", pCurrentActor, userSelected);
    }
  }

  if (_pImpl->_pCurrentActor) {
    follow(_pImpl->_pCurrentActor);
  }
}

void Engine::draw(ngf::RenderTarget &target, bool screenshot) const {
  if (!_pImpl->_pRoom)
    return;

  // update room shader if necessary
  ngf::RenderStates states;
  auto effect = _pImpl->_pRoom->getEffect();
  if (_pImpl->_roomEffect != effect) {
    if (effect == RoomEffectConstants::EFFECT_BLACKANDWHITE) {
      _pImpl->_roomShader.load(_vertexShader, _bwFragmentShader);
    } else if (effect == RoomEffectConstants::EFFECT_EGA) {
      _pImpl->_roomShader.load(_vertexShader, _egaFragmenShader);
    }
  }
  states.shader = &_pImpl->_roomShader;
  if (effect != RoomEffectConstants::EFFECT_BLACKANDWHITE &&
      effect != RoomEffectConstants::EFFECT_EGA) {
    states.shader = nullptr;
  }

  // render the room to a texture, this allows to create a post process effect: room effect
  ngf::RenderTexture roomTexture(target.getSize());
  ngf::View view(ngf::frect::fromPositionSize({0, 0}, _pImpl->_pRoom->getScreenSize()));
  roomTexture.setView(view);
  roomTexture.clear();
  _pImpl->_pRoom->draw(roomTexture, _pImpl->_camera.getAt());
  roomTexture.display();

  // then render a sprite with this texture and apply the room effect
  ngf::RenderTexture roomWithEffectTexture(target.getSize());
  roomWithEffectTexture.clear();
  ngf::Sprite sprite(roomTexture.getTexture());
  sprite.draw(roomWithEffectTexture, states);
  // and render overlay
  ngf::RectangleShape fadeShape;
  fadeShape.setSize(roomWithEffectTexture.getSize());
  fadeShape.setColor(_pImpl->_pRoom->getOverlayColor());
  fadeShape.draw(roomWithEffectTexture, {});
  roomWithEffectTexture.display();

  // apply the room rotation
  ngf::Sprite fadeSprite(roomWithEffectTexture.getTexture());
  auto pos = target.getView().getSize() / 2.f;
  fadeSprite.getTransform().setOrigin(pos);
  fadeSprite.getTransform().setPosition(pos);
  fadeSprite.getTransform().setRotation(_pImpl->_pRoom->getRotation());

  // render fade
  _pImpl->_fadeShader.setUniform("u_texture2", _pImpl->_blackTexture); // TODO: change this to fade to new room
  _pImpl->_fadeShader.setUniform("u_fade", _pImpl->_fade); // fade value between [0.f,1.f]
  _pImpl->_fadeShader.setUniform("u_fadeToSep", 0);  // 1 to fade to sepia
  _pImpl->_fadeShader.setUniform("u_movement", 0.f); // change this for wobble effect
  _pImpl->_fadeShader.setUniform("u_timer", _pImpl->_time.getTotalSeconds());
  states.shader = &_pImpl->_fadeShader;
  fadeSprite.draw(target, states);

  // draw walkboxes, actor texts
  auto orgView = target.getView();
  target.setView(view);
  _pImpl->drawWalkboxes(target);
  _pImpl->_talkingState.draw(target, {});
  _pImpl->_pRoom->drawForeground(target, _pImpl->_camera.getAt());
  target.setView(orgView);

  // if we take a screenshot (for savegame) then stop drawing
  if (screenshot)
    return;

  // draw dialogs, hud
  _pImpl->_dialogManager.draw(target, {});
  _pImpl->drawHud(target);

  // draw actor icons
  if ((_pImpl->_dialogManager.getState() == DialogManagerState::None)
      && _pImpl->_inputActive) {
    _pImpl->_actorIcons.draw(target, {});
  }

  // draw options or startscreen if necessary
  if (_pImpl->_state == EngineState::Options) {
    _pImpl->_optionsDialog.draw(target, {});
  } else if (_pImpl->_state == EngineState::StartScreen) {
    _pImpl->_startScreenDialog.draw(target, {});
  }

  // draw pause, cursor and no override icon
  _pImpl->drawPause(target);
  _pImpl->drawCursor(target);
  _pImpl->drawCursorText(target);
  _pImpl->drawNoOverride(target);
}

void Engine::setWalkboxesFlags(int show) { _pImpl->_showDrawWalkboxes = show; }

int Engine::getWalkboxesFlags() const { return _pImpl->_showDrawWalkboxes; }

void Engine::startDialog(const std::string &dialog, const std::string &node) {
  std::string actor;
  if (_pImpl->_pCurrentActor)
    actor = _pImpl->_pCurrentActor->getKey();
  _pImpl->_dialogManager.start(actor, dialog, node);
}

void Engine::execute(const std::string &code) { _pImpl->_pScriptExecute->execute(code); }

SoundDefinition *Engine::getSoundDefinition(const std::string &name) {
  return _pImpl->_pScriptExecute->getSoundDefinition(name);
}

bool Engine::executeCondition(const std::string &code) { return _pImpl->_pScriptExecute->executeCondition(code); }

std::string Engine::executeDollar(const std::string &code) { return _pImpl->_pScriptExecute->executeDollar(code); }

void Engine::addSelectableActor(int index, Actor *pActor) {
  _pImpl->_actorsIconSlots.at(index - 1).selectable = true;
  _pImpl->_actorsIconSlots.at(index - 1).pActor = pActor;
}

void Engine::actorSlotSelectable(Actor *pActor, bool selectable) {
  auto it = std::find_if(_pImpl->_actorsIconSlots.begin(), _pImpl->_actorsIconSlots.end(),
                         [&pActor](auto &selectableActor) -> bool { return selectableActor.pActor == pActor; });
  if (it != _pImpl->_actorsIconSlots.end()) {
    it->selectable = selectable;
  }
}

void Engine::actorSlotSelectable(int index, bool selectable) {
  _pImpl->_actorsIconSlots.at(index - 1).selectable = selectable;
}

bool Engine::isActorSelectable(Actor *pActor) const {
  for (auto &&slot : _pImpl->_actorsIconSlots) {
    if (slot.pActor == pActor)
      return slot.selectable;
  }
  return false;
}

ActorSlotSelectableMode Engine::getActorSlotSelectable() const { return _pImpl->_actorIcons.getMode(); }

void Engine::setActorSlotSelectable(ActorSlotSelectableMode mode) { _pImpl->_actorIcons.setMode(mode); }

void Engine::setUseFlag(UseFlag flag, Entity *object) {
  _pImpl->_useFlag = flag;
  _pImpl->_pUseObject = object;
}

void Engine::cutsceneOverride() {
  if (!_pImpl->_pCutscene)
    return;
  _pImpl->_pCutscene->cutsceneOverride();
}

void Engine::cutscene(std::unique_ptr<Cutscene> function) {
  _pImpl->_pCutscene = function.get();
  addThread(std::move(function));
}

Cutscene *Engine::getCutscene() const { return _pImpl->_pCutscene; }

bool Engine::inCutscene() const { return _pImpl->_pCutscene && !_pImpl->_pCutscene->isElapsed(); }

HSQOBJECT &Engine::getDefaultObject() { return _pImpl->_pDefaultObject; }

void Engine::flashSelectableActor(bool on) { _pImpl->_actorIcons.flash(on); }

const Verb *Engine::getActiveVerb() const { return _pImpl->_hud.getCurrentVerb(); }

void Engine::setFade(float fade) { _pImpl->_fade = fade; }

float Engine::getFade() const { return _pImpl->_fade; }

void Engine::fadeTo(float destination, ngf::TimeSpan time, InterpolationMethod method) {
  auto get = [this]() -> float { return getFade(); };
  auto set = [this](const float &a) { setFade(a); };
  auto f = std::make_unique<ChangeProperty<float>>(get, set, destination, time, method);
  addFunction(std::move(f));
}

void Engine::pushSentence(int id, Entity *pObj1, Entity *pObj2) {
  const Verb *pVerb = _pImpl->_hud.getVerb(id);
  if (!pVerb)
    return;
  _pImpl->_pVerbExecute->execute(pVerb, pObj1, pObj2);
}

void Engine::setSentence(std::unique_ptr<Sentence> sentence) {
  _pImpl->_pSentence = std::move(sentence);
}

void Engine::stopSentence() {
  if (!_pImpl->_pSentence)
    return;
  _pImpl->_pSentence->stop();
  _pImpl->_pSentence.reset();
}

void Engine::keyDown(const Input &key) {
  _pImpl->_newKeyDowns.insert(key);
}

void Engine::keyUp(const Input &key) {
  auto it = _pImpl->_newKeyDowns.find(key);
  if (it == _pImpl->_newKeyDowns.end())
    return;
  _pImpl->_newKeyDowns.erase(it);
}

void Engine::sayLineAt(glm::ivec2 pos, ngf::Color color, ngf::TimeSpan duration, const std::string &text) {
  _pImpl->_talkingState.setTalkColor(color);
  auto size = getRoom()->getRoomSize();
  _pImpl->_talkingState.setPosition(toDefaultView(pos, size));
  _pImpl->_talkingState.setText(getText(text));
  _pImpl->_talkingState.setDuration(duration);
}

void Engine::sayLineAt(glm::ivec2 pos, Entity &entity, const std::string &text) {
  auto size = getRoom()->getRoomSize();
  _pImpl->_talkingState.setPosition(toDefaultView(pos, size));
  _pImpl->_talkingState.loadLip(text, &entity);
}

void Engine::showOptions(bool visible) {
  _pImpl->_state = visible ? EngineState::Options : EngineState::Game;
}

void Engine::quit() {
  _pImpl->_pApp->quit();
}

void Engine::run() {
  std::ifstream is("engge.nut");
  if (is.is_open()) {
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

Inventory &Engine::getInventory() { return _pImpl->_hud.getInventory(); }
Hud &Engine::getHud() { return _pImpl->_hud; }

void Engine::saveGame(int slot) {
  Impl::SaveGameSystem saveGameSystem(_pImpl.get());
  auto path = Impl::SaveGameSystem::getSlotPath(slot);
  std::filesystem::path screenshotPath(path);
  screenshotPath.replace_extension(".png");
  _pImpl->captureScreen(screenshotPath.string());
  saveGameSystem.saveGame(path);
}

void Engine::loadGame(int slot) {
  Impl::SaveGameSystem saveGameSystem(_pImpl.get());
  saveGameSystem.loadGame(Impl::SaveGameSystem::getSlotPath(slot));
}

void Engine::setAutoSave(bool autoSave) { _pImpl->_autoSave = autoSave; }

bool Engine::getAutoSave() const { return _pImpl->_autoSave; }

void Engine::allowSaveGames(bool allow) {
  _pImpl->_optionsDialog.setSaveEnabled(allow);
}

Entity *Engine::getEntity(const std::string &name) {
  if (name == "agent" || name == "player")
    return _pImpl->_pCurrentActor;

  Entity *pEntity = nullptr;
  ScriptEngine::get(name.data(), pEntity);
  return pEntity;
}

void Engine::getSlotSavegames(std::vector<SavegameSlot> &slots) {
  for (int i = 1; i <= 9; ++i) {
    auto path = Impl::SaveGameSystem::getSlotPath(i);

    SavegameSlot slot;
    slot.slot = i;
    slot.path = path;

    if (std::filesystem::exists(path)) {
      Impl::SaveGameSystem::getSlot(slot);
    }
    slots.push_back(slot);
  }
}

void Engine::stopTalking() const {
  _pImpl->stopTalking();
}

void Engine::stopTalkingExcept(Entity *pEntity) const {
  _pImpl->stopTalkingExcept(pEntity);
}

std::wstring SavegameSlot::getSaveTimeString() const {
  tm *ltm = localtime(&savetime);
  wchar_t buffer[120];
  // time format: "%b %d at %H:%M"
  auto format = Locator<TextDatabase>::get().getText(99944);
  wcsftime(buffer, 120, format.data(), ltm);
  std::wstring s(buffer);
  if (easyMode) {
    s.append(1, L' ');
    s.append(Locator<TextDatabase>::get().getText(99955));
  }
  return s;
}

std::wstring SavegameSlot::getGameTimeString() const {
  wchar_t buffer[120];
  auto min = static_cast<int>(gametime.getTotalSeconds() / 60.0);
  if (min < 2) {
    // "%d minute"
    auto format = Locator<TextDatabase>::get().getText(99945);
    swprintf(buffer, 120, format.data(), min);
  } else if (min < 60) {
    // "%d minutes"
    auto format = Locator<TextDatabase>::get().getText(99946);
    swprintf(buffer, 120, format.data(), min);
  } else {
    int format;
    int hour = min / 60;
    min = min % 60;
    if (hour < 2 && min < 2) {
      // "%d hour %d minute"
      format = 99947;
    } else if (hour < 2 && min >= 2) {
      // "%d hour %d minutes";
      format = 99948;
    } else if (hour >= 2 && min < 2) {
      // "%d hours %d minute";
      format = 99949;
    } else {
      // "%d hours %d minutes";
      format = 99950;
    }
    swprintf(buffer, 120, Locator<TextDatabase>::get().getText(format).data(), hour, min);
  }

  std::wstring s(buffer);
  return s;
}
} // namespace ng
