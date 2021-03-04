#ifdef _WIN32
// for Windows you'll need this to have M_PI defined
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <ctime>
#include <cwchar>
#include <filesystem>
#include <iomanip>
#include <memory>
#include <set>
#include <string>
#include <unordered_set>
#include <squirrel.h>
#include <ngf/Application.h>
#include <ngf/Graphics/Colors.h>
#include <ngf/System/Mouse.h>
#include <engge/Util/RandomNumberGenerator.hpp>
#include <engge/EnggeApplication.hpp>
#include <engge/Engine/Engine.hpp>
#include <engge/Engine/ActorIcons.hpp>
#include <engge/Engine/Camera.hpp>
#include <engge/Engine/Cutscene.hpp>
#include <engge/Engine/Hud.hpp>
#include <engge/Input/InputConstants.hpp>
#include <engge/Dialog/DialogManager.hpp>
#include <engge/Engine/Inventory.hpp>
#include <engge/Engine/Preferences.hpp>
#include <engge/Room/Room.hpp>
#include <engge/Room/RoomScaling.hpp>
#include <engge/Scripting/ScriptEngine.hpp>
#include <engge/Scripting/ScriptExecute.hpp>
#include <engge/Engine/Sentence.hpp>
#include <engge/Audio/SoundDefinition.hpp>
#include <engge/Audio/SoundManager.hpp>
#include <engge/Graphics/SpriteSheet.hpp>
#include <engge/Engine/TextDatabase.hpp>
#include <engge/Engine/Verb.hpp>
#include <engge/Scripting/VerbExecute.hpp>
#include <engge/System/Logger.hpp>
#include <engge/Engine/InputStateConstants.hpp>
#include "EngineImpl.hpp"

namespace fs = std::filesystem;

namespace ng {

Engine::Engine() : m_pImpl(std::make_unique<Impl>()) {
  m_pImpl->_pEngine = this;
  m_pImpl->_soundManager.setEngine(this);
  m_pImpl->_dialogManager.setEngine(this);
  m_pImpl->_actorIcons.setEngine(this);
  m_pImpl->_camera.setEngine(this);
  m_pImpl->_talkingState.setEngine(this);

  // load all messages
  std::stringstream s;
  auto lang =
      m_pImpl->_preferences.getUserPreference<std::string>(PreferenceNames::Language,
                                                           PreferenceDefaultValues::Language);
  s << "ThimbleweedText_" << lang << ".tsv";
  Locator<TextDatabase>::get().load(s.str());

  m_pImpl->_optionsDialog.setSaveEnabled(true);
  m_pImpl->_optionsDialog.setEngine(this);
  m_pImpl->_optionsDialog.setCallback([this]() {
    showOptions(false);
  });
  m_pImpl->_startScreenDialog.setEngine(this);
  m_pImpl->_startScreenDialog.setNewGameCallback([this]() {
    m_pImpl->_state = EngineState::Game;
    m_pImpl->exitRoom(nullptr);
    ScriptEngine::call("start", true);
  });
  m_pImpl->_startScreenDialog.setSlotCallback([this](int slot) {
    m_pImpl->_state = EngineState::Game;
    loadGame(slot);
  });

  m_pImpl->_preferences.subscribe([this](const std::string &name) {
    if (name == PreferenceNames::Language) {
      auto newLang = m_pImpl->_preferences.getUserPreference<std::string>(PreferenceNames::Language,
                                                                          PreferenceDefaultValues::Language);
      m_pImpl->onLanguageChange(newLang);
    } else if (name == PreferenceNames::Fullscreen) {
      auto fullscreen = m_pImpl->_preferences.getUserPreference(PreferenceNames::Fullscreen,
                                                                PreferenceDefaultValues::Fullscreen);
      m_pImpl->_pApp->getWindow().setFullscreen(fullscreen);
    }
  });
}

Engine::~Engine() = default;

int Engine::getFrameCounter() const { return m_pImpl->_frameCounter; }

void Engine::setApplication(ng::EnggeApplication *app) { m_pImpl->_pApp = app; }

const ng::EnggeApplication *Engine::getApplication() const { return m_pImpl->_pApp; }
ng::EnggeApplication *Engine::getApplication() { return m_pImpl->_pApp; }

ResourceManager &Engine::getResourceManager() { return m_pImpl->_textureManager; }

Room *Engine::getRoom() { return m_pImpl->_pRoom; }

std::wstring Engine::getText(int id) {
  auto text = Locator<TextDatabase>::get().getText(id);
  removeFirstParenthesis(text);
  return text;
}

std::wstring Engine::getText(const std::string &text) {
  auto text2 = Locator<TextDatabase>::get().getText(text);
  removeFirstParenthesis(text2);
  return text2;
}

void Engine::addActor(std::unique_ptr<Actor> actor) { m_pImpl->_actors.push_back(std::move(actor)); }

void Engine::addRoom(std::unique_ptr<Room> room) { m_pImpl->_rooms.push_back(std::move(room)); }

std::vector<std::unique_ptr<Room>> &Engine::getRooms() { return m_pImpl->_rooms; }

void Engine::addFunction(std::unique_ptr<Function> function) { m_pImpl->_newFunctions.push_back(std::move(function)); }

void Engine::addCallback(std::unique_ptr<Callback> callback) { m_pImpl->_callbacks.push_back(std::move(callback)); }

void Engine::removeCallback(int id) {
  auto it = std::find_if(m_pImpl->_callbacks.begin(), m_pImpl->_callbacks.end(),
                         [id](auto &callback) -> bool { return callback->getId() == id; });
  if (it != m_pImpl->_callbacks.end()) {
    m_pImpl->_callbacks.erase(it);
  }
}

std::vector<std::unique_ptr<Actor>> &Engine::getActors() { return m_pImpl->_actors; }

Actor *Engine::getCurrentActor() { return m_pImpl->_pCurrentActor; }

const VerbUiColors *Engine::getVerbUiColors(const std::string &name) const {
  if (name.empty()) {
    auto index = m_pImpl->getCurrentActorIndex();
    if (index == -1)
      return nullptr;
    return &m_pImpl->_hud.getVerbUiColors(index);
  }
  for (int i = 0; i < static_cast<int>(m_pImpl->_actorsIconSlots.size()); i++) {
    const auto &selectableActor = m_pImpl->_actorsIconSlots.at(i);
    if (selectableActor.pActor && selectableActor.pActor->getKey() == name) {
      return &m_pImpl->_hud.getVerbUiColors(i);
    }
  }
  return nullptr;
}

bool Engine::getInputActive() const { return m_pImpl->_inputActive; }

void Engine::setInputState(int state) {
  if ((state & InputStateConstants::UI_INPUT_ON) == InputStateConstants::UI_INPUT_ON) {
    m_pImpl->_inputActive = true;
  }
  if ((state & InputStateConstants::UI_INPUT_OFF) == InputStateConstants::UI_INPUT_OFF) {
    m_pImpl->_inputActive = false;
  }
  if ((state & InputStateConstants::UI_VERBS_ON) == InputStateConstants::UI_VERBS_ON) {
    m_pImpl->_inputVerbsActive = true;
  }
  if ((state & InputStateConstants::UI_VERBS_OFF) == InputStateConstants::UI_VERBS_OFF) {
    m_pImpl->_inputVerbsActive = false;
  }
  if ((state & InputStateConstants::UI_CURSOR_ON) == InputStateConstants::UI_CURSOR_ON) {
    m_pImpl->_showCursor = true;
  }
  if ((state & InputStateConstants::UI_CURSOR_OFF) == InputStateConstants::UI_CURSOR_OFF) {
    m_pImpl->_showCursor = false;
  }
  if ((state & InputStateConstants::UI_HUDOBJECTS_ON) == InputStateConstants::UI_HUDOBJECTS_ON) {
    m_pImpl->_inputHUD = true;
  }
  if ((state & InputStateConstants::UI_HUDOBJECTS_OFF) == InputStateConstants::UI_HUDOBJECTS_OFF) {
    m_pImpl->_inputHUD = false;
  }
}

int Engine::getInputState() const {
  int inputState = 0;
  inputState |= (m_pImpl->_inputActive ? InputStateConstants::UI_INPUT_ON : InputStateConstants::UI_INPUT_OFF);
  inputState |= (m_pImpl->_inputVerbsActive ? InputStateConstants::UI_VERBS_ON : InputStateConstants::UI_VERBS_OFF);
  inputState |= (m_pImpl->_showCursor ? InputStateConstants::UI_CURSOR_ON : InputStateConstants::UI_CURSOR_OFF);
  inputState |= (m_pImpl->_inputHUD ? InputStateConstants::UI_HUDOBJECTS_ON : InputStateConstants::UI_HUDOBJECTS_OFF);
  return inputState;
}

void Engine::follow(Actor *pActor) {
  auto panCamera =
      (m_pImpl->_pFollowActor && pActor && m_pImpl->_pFollowActor != pActor && m_pImpl->_pFollowActor->getRoom() &&
          pActor->getRoom() && m_pImpl->_pFollowActor->getRoom()->getId() == pActor->getRoom()->getId());
  m_pImpl->_pFollowActor = pActor;
  if (!pActor)
    return;

  auto pos = pActor->getPosition();
  setRoom(pActor->getRoom());
  if (panCamera) {
    m_pImpl->_camera.panTo(pos, ngf::TimeSpan::seconds(4), InterpolationMethod::EaseOut);
    return;
  }
  m_pImpl->_camera.at(pos);
}

void Engine::setVerbExecute(std::unique_ptr<VerbExecute> verbExecute) {
  m_pImpl->_pVerbExecute = std::move(verbExecute);
}

void Engine::setDefaultVerb() {
  m_pImpl->_hud.setHoveredEntity(nullptr);
  auto index = m_pImpl->getCurrentActorIndex();
  if (index == -1)
    return;

  const auto &verbSlot = m_pImpl->_hud.getVerbSlot(index);
  m_pImpl->_hud.setCurrentVerb(&verbSlot.getVerb(0));
  m_pImpl->_useFlag = UseFlag::None;
  m_pImpl->_pUseObject = nullptr;
  m_pImpl->_objId1 = 0;
  m_pImpl->_pObj2 = nullptr;
}

void Engine::setScriptExecute(std::unique_ptr<ScriptExecute> scriptExecute) {
  m_pImpl->_pScriptExecute = std::move(scriptExecute);
}

void Engine::addThread(std::unique_ptr<ThreadBase> thread) { m_pImpl->_threads.push_back(std::move(thread)); }

std::vector<std::unique_ptr<ThreadBase>> &Engine::getThreads() { return m_pImpl->_threads; }

glm::vec2 Engine::getMousePositionInRoom() const { return m_pImpl->_mousePosInRoom; }

Preferences &Engine::getPreferences() { return m_pImpl->_preferences; }

SoundManager &Engine::getSoundManager() { return m_pImpl->_soundManager; }

DialogManager &Engine::getDialogManager() { return m_pImpl->_dialogManager; }

Camera &Engine::getCamera() { return m_pImpl->_camera; }

ngf::TimeSpan Engine::getTime() const { return m_pImpl->_time; }

SQInteger Engine::setRoom(Room *pRoom) {
  if (!pRoom)
    return 0;

  auto pOldRoom = m_pImpl->_pRoom;
  if (pRoom == pOldRoom)
    return 0;

  auto result = m_pImpl->exitRoom(nullptr);
  if (SQ_FAILED(result))
    return result;

  m_pImpl->setCurrentRoom(pRoom);

  result = m_pImpl->enterRoom(pRoom, nullptr);
  if (SQ_FAILED(result))
    return result;

  return 0;
}

SQInteger Engine::enterRoomFromDoor(Object *pDoor) {
  auto dir = pDoor->getUseDirection();
  auto facing = toFacing(dir);
  auto pRoom = pDoor->getRoom();

  // exit current room
  auto result = m_pImpl->exitRoom(nullptr);
  if (SQ_FAILED(result))
    return result;

  // change current room
  m_pImpl->setCurrentRoom(pRoom);

  // move current actor to the new room
  auto actor = getCurrentActor();
  actor->getCostume().setFacing(facing);
  actor->setRoom(pRoom);
  auto pos = pDoor->getPosition();
  auto usePos = pDoor->getUsePosition().value_or(glm::vec2());
  pos += usePos;
  actor->setPosition(pos);

  // move camera to the actor if not closeup room
  if (pRoom->getFullscreen() != 1) {
    m_pImpl->_camera.at(pos);
  }

  // enter current room
  return m_pImpl->enterRoom(pRoom, pDoor);
}

void Engine::setInputHUD(bool on) { m_pImpl->_inputHUD = on; }

void Engine::setInputActive(bool active) {
  if (inCutscene())
    return;
  m_pImpl->_inputActive = active;
  m_pImpl->_showCursor = active;
}

void Engine::inputSilentOff() { m_pImpl->_inputActive = false; }

void Engine::setInputVerbs(bool on) { m_pImpl->_inputVerbsActive = on; }

void Engine::update(const ngf::TimeSpan &el) {
  if(m_pImpl->_state == EngineState::Quit) return;

  roomEffect.RandomValue[0] = Locator<RandomNumberGenerator>::get().generateFloat(0, 1.f);
  roomEffect.iGlobalTime = fmod(m_pImpl->_time.getTotalSeconds(), 1000.f);
  roomEffect.TimeLapse = roomEffect.iGlobalTime;

  auto gameSpeedFactor =
      getPreferences().getUserPreference(PreferenceNames::EnggeGameSpeedFactor,
                                         PreferenceDefaultValues::EnggeGameSpeedFactor);
  const ngf::TimeSpan elapsed(ngf::TimeSpan::seconds(el.getTotalSeconds() * gameSpeedFactor));
  m_pImpl->stopThreads();
  auto screenSize = m_pImpl->_pRoom->getScreenSize();
  auto view = ngf::View{ngf::frect::fromPositionSize({0, 0}, screenSize)};
  m_pImpl->_mousePos = m_pImpl->_pApp->getRenderTarget()->mapPixelToCoords(ngf::Mouse::getPosition(), view);
  if (m_pImpl->_pRoom && m_pImpl->_pRoom->getName() != "Void") {
    auto screenMouse = toDefaultView((glm::ivec2) m_pImpl->_mousePos, screenSize);
    m_pImpl->_hud.setMousePosition(screenMouse);
    m_pImpl->_dialogManager.setMousePosition(screenMouse);
  }
  if (m_pImpl->_state == EngineState::Options) {
    m_pImpl->_optionsDialog.update(elapsed);
  } else if (m_pImpl->_state == EngineState::StartScreen) {
    m_pImpl->_startScreenDialog.update(elapsed);
    if(m_pImpl->_state == EngineState::Quit)
      return;
  }

  if (m_pImpl->_state == EngineState::Paused) {
    m_pImpl->updateKeys();
    return;
  }

  // update fade effect
  m_pImpl->_fadeEffect.elapsed += elapsed;
  m_pImpl->_talkingState.update(elapsed);

  m_pImpl->_frameCounter++;
  auto &io = ImGui::GetIO();
  auto wasMouseDown = m_pImpl->_isMouseDown && !io.WantCaptureMouse;
  auto wasMouseRightDown = m_pImpl->_isMouseRightDown;
  m_pImpl->_isMouseDown =
      ngf::Mouse::isButtonPressed(ngf::Mouse::Button::Left) && !io.WantCaptureMouse;
  if (!wasMouseDown || !m_pImpl->_isMouseDown) {
    m_pImpl->_mouseDownTime = ngf::TimeSpan::seconds(0);
    m_pImpl->run(false);
  } else {
    m_pImpl->_mouseDownTime += elapsed;
    if (m_pImpl->_mouseDownTime > ngf::TimeSpan::seconds(0.5f)) {
      m_pImpl->run(true);
    }
  }
  m_pImpl->_isMouseRightDown = ngf::Mouse::isButtonPressed(ngf::Mouse::Button::Right) && !io.WantCaptureMouse;
  bool isRightClick = wasMouseRightDown != m_pImpl->_isMouseRightDown && !m_pImpl->_isMouseRightDown;
  auto isMouseClick = wasMouseDown != m_pImpl->_isMouseDown && !m_pImpl->_isMouseDown;

  m_pImpl->_time += elapsed;
  m_pImpl->_noOverrideElapsed += elapsed;

  m_pImpl->_camera.update(elapsed);
  m_pImpl->_soundManager.update(elapsed);
  m_pImpl->updateCutscene(elapsed);
  m_pImpl->updateFunctions(elapsed);
  m_pImpl->updateSentence(elapsed);
  m_pImpl->updateKeys();

  if (!m_pImpl->_pRoom || m_pImpl->_pRoom->getName() == "Void")
    return;

  m_pImpl->updateRoomScalings();

  m_pImpl->_pRoom->update(elapsed);
  for (auto &pActor : m_pImpl->_actors) {
    if (!pActor || pActor->getRoom() == m_pImpl->_pRoom)
      continue;
    pActor->update(elapsed);
  }

  if (m_pImpl->_pFollowActor && m_pImpl->_pFollowActor->isVisible() && m_pImpl->_pFollowActor->getRoom() == getRoom()) {
    auto screen = m_pImpl->_pRoom->getScreenSize();
    auto pos = m_pImpl->_pFollowActor->getPosition();
    auto margin = glm::vec2(screen.x / 4, screen.y / 4);
    auto cameraPos = m_pImpl->_camera.getAt();
    if (m_pImpl->_camera.isMoving() || (cameraPos.x > pos.x + margin.x) || (cameraPos.x < pos.x - margin.x) ||
        (cameraPos.y > pos.y + margin.y) || (cameraPos.y < pos.y - margin.y)) {
      m_pImpl->_camera.panTo(pos, ngf::TimeSpan::seconds(4), InterpolationMethod::EaseOut);
    }
  }

  m_pImpl->updateActorIcons(elapsed);

  if (m_pImpl->_state == EngineState::Options)
    return;

  m_pImpl->_cursorDirection = CursorDirection::None;
  m_pImpl->updateMouseCursor();

  auto mousePos =
      glm::vec2(m_pImpl->_mousePos.x, m_pImpl->_pRoom->getScreenSize().y - m_pImpl->_mousePos.y);
  m_pImpl->_mousePosInRoom = mousePos + m_pImpl->_camera.getRect().getTopLeft();

  m_pImpl->_dialogManager.update(elapsed);

  m_pImpl->_hud.setActive(m_pImpl->_inputVerbsActive && m_pImpl->_dialogManager.getState() == DialogManagerState::None
                              && m_pImpl->_pRoom->getFullscreen() != 1);
  m_pImpl->_hud.setHoveredEntity(m_pImpl->getHoveredEntity(m_pImpl->_mousePosInRoom));
  m_pImpl->updateHoveredEntity(isRightClick);

  if (m_pImpl->_pCurrentActor) {
    auto &objects = m_pImpl->_pCurrentActor->getObjects();
    for (auto &object : objects) {
      object->update(elapsed);
    }
  }

  m_pImpl->_hud.update(elapsed);

  if (m_pImpl->_actorIcons.isMouseOver())
    return;

  if (isMouseClick && m_pImpl->clickedAt(m_pImpl->_mousePosInRoom))
    return;

  if (!m_pImpl->_inputActive)
    return;

  m_pImpl->updateKeyboard();

  if (m_pImpl->_dialogManager.getState() != DialogManagerState::None) {
    auto rightClickSkipsDialog = getPreferences().getUserPreference(PreferenceNames::RightClickSkipsDialog,
                                                                    PreferenceDefaultValues::RightClickSkipsDialog);
    if (rightClickSkipsDialog && isRightClick) {
      m_pImpl->skipText();
    }
    return;
  }

  if (!m_pImpl->_pCurrentActor)
    return;

  if (!isMouseClick && !isRightClick && !m_pImpl->_isMouseDown)
    return;

  m_pImpl->_hud.setVisible(true);
  m_pImpl->_actorIcons.setVisible(true);
  m_pImpl->_cursorVisible = true;
  stopSentence();

  const auto *pVerb = m_pImpl->getHoveredVerb();
  // input click on a verb ?
  if (m_pImpl->_hud.getActive() && pVerb) {
    m_pImpl->onVerbClick(pVerb);
    return;
  }

  if (!isMouseClick && !isRightClick) {
    if (!pVerb && !m_pImpl->_hud.getHoveredEntity())
      m_pImpl->_pCurrentActor->walkTo(m_pImpl->_mousePosInRoom);
    return;
  }

  if (m_pImpl->_hud.getHoveredEntity()) {
    ScriptEngine::rawCall("onObjectClick", m_pImpl->_hud.getHoveredEntity());
    auto pVerbOverride = m_pImpl->_hud.getVerbOverride();
    if (!pVerbOverride) {
      pVerbOverride = m_pImpl->_hud.getCurrentVerb();
    }
    pVerbOverride = m_pImpl->overrideVerb(pVerbOverride);
    auto pObj1 = EntityManager::getScriptObjectFromId<Entity>(m_pImpl->_objId1);
    pObj1 = pVerbOverride->id == VerbConstants::VERB_TALKTO ? m_pImpl->getEntity(pObj1) : pObj1;
    auto pObj2 = pVerbOverride->id == VerbConstants::VERB_GIVE ? m_pImpl->getEntity(m_pImpl->_pObj2) : m_pImpl->_pObj2;
    if (pObj1) {
      m_pImpl->_pVerbExecute->execute(pVerbOverride, pObj1, pObj2);
    }
    return;
  }

  if (m_pImpl->_hud.isMouseOver())
    return;

  m_pImpl->_pCurrentActor->walkTo(m_pImpl->_mousePosInRoom);
  setDefaultVerb();
}

void Engine::setCurrentActor(Actor *pCurrentActor, bool userSelected) {
  m_pImpl->_pCurrentActor = pCurrentActor;

  int currentActorIndex = m_pImpl->getCurrentActorIndex();
  m_pImpl->_hud.setCurrentActorIndex(currentActorIndex);
  m_pImpl->_hud.setCurrentActor(m_pImpl->_pCurrentActor);

  ScriptEngine::rawCall("onActorSelected", pCurrentActor, userSelected);
  auto pRoom = pCurrentActor ? pCurrentActor->getRoom() : nullptr;
  if (pRoom) {
    if (ScriptEngine::rawExists(pRoom, "onActorSelected")) {
      ScriptEngine::rawCall(pRoom, "onActorSelected", pCurrentActor, userSelected);
    }
  }

  if (m_pImpl->_pCurrentActor) {
    follow(m_pImpl->_pCurrentActor);
  }
}

void Engine::draw(ngf::RenderTarget &target, bool screenshot) const {
  if (!m_pImpl->_pRoom)
    return;

  // update room shader if necessary
  ngf::RenderStates states;
  auto effect = m_pImpl->_pRoom->getEffect();
  if (m_pImpl->_roomEffect != effect) {
    if (effect == RoomEffectConstants::EFFECT_BLACKANDWHITE) {
      m_pImpl->_roomShader.load(Shaders::vertexShader, Shaders::bwFragmentShader);
    } else if (effect == RoomEffectConstants::EFFECT_EGA) {
      m_pImpl->_roomShader.load(Shaders::vertexShader, Shaders::egaFragmenShader);
    } else if (effect == RoomEffectConstants::EFFECT_GHOST) {
      m_pImpl->_roomShader.load(Shaders::vertexShader, Shaders::ghostFragmentShader);
    } else if (effect == RoomEffectConstants::EFFECT_SEPIA) {
      m_pImpl->_roomShader.load(Shaders::vertexShader, Shaders::sepiaFragmentShader);
    } else if (effect == RoomEffectConstants::EFFECT_VHS) {
      m_pImpl->_roomShader.load(Shaders::vertexShader, Shaders::vhsFragmentShader);
    }
    m_pImpl->_roomEffect = effect;
  }
  states.shader = &m_pImpl->_roomShader;
  if (effect == RoomEffectConstants::EFFECT_GHOST) {
    // don't remove the fmod function or you will have float overflow with the shader and the effect will look strange
    m_pImpl->_roomShader.setUniform("iGlobalTime", roomEffect.iGlobalTime);
    m_pImpl->_roomShader.setUniform("iFade", roomEffect.iFade);
    m_pImpl->_roomShader.setUniform("wobbleIntensity", roomEffect.wobbleIntensity);
    m_pImpl->_roomShader.setUniform("shadows", roomEffect.shadows);
    m_pImpl->_roomShader.setUniform("midtones", roomEffect.midtones);
    m_pImpl->_roomShader.setUniform("highlights", roomEffect.highlights);
  } else if (effect == RoomEffectConstants::EFFECT_SEPIA) {
    m_pImpl->_roomShader.setUniform("sepiaFlicker", roomEffect.sepiaFlicker);
    m_pImpl->_roomShader.setUniformArray("RandomValue", roomEffect.RandomValue.data(), 5);
    m_pImpl->_roomShader.setUniform("TimeLapse", roomEffect.TimeLapse);
  } else if (effect == RoomEffectConstants::EFFECT_VHS) {
    m_pImpl->_roomShader.setUniform("iGlobalTime", roomEffect.iGlobalTime);
    m_pImpl->_roomShader.setUniform("iNoiseThreshold", roomEffect.iNoiseThreshold);
  } else if (effect == RoomEffectConstants::EFFECT_NONE) {
    states.shader = nullptr;
  }

  // render the room to a texture, this allows to create a post process effect: room effect
  ngf::RenderTexture roomTexture(target.getSize());
  auto screenSize = m_pImpl->_pRoom->getScreenSize();
  ngf::View view(ngf::frect::fromPositionSize({0, 0}, screenSize));
  roomTexture.setView(view);
  roomTexture.clear();
  m_pImpl->_pRoom->draw(roomTexture, m_pImpl->_camera.getRect().getTopLeft());
  roomTexture.display();

  // then render a sprite with this texture and apply the room effect
  ngf::RenderTexture roomWithEffectTexture(target.getSize());
  roomWithEffectTexture.clear();
  ngf::Sprite sprite(roomTexture.getTexture());
  sprite.draw(roomWithEffectTexture, states);

  // and render overlay
  ngf::RectangleShape fadeShape;
  fadeShape.setSize(roomWithEffectTexture.getSize());
  fadeShape.setColor(m_pImpl->_pRoom->getOverlayColor());
  fadeShape.draw(roomWithEffectTexture, {});
  roomWithEffectTexture.display();

  // render fade
  ngf::Sprite fadeSprite;
  float fade = m_pImpl->_fadeEffect.effect == FadeEffect::None ? 0.f :
               std::clamp(
                   m_pImpl->_fadeEffect.elapsed.getTotalSeconds() / m_pImpl->_fadeEffect.duration.getTotalSeconds(),
                   0.f, 1.f);
  ngf::RenderTexture roomTexture2(target.getSize());
  roomTexture2.setView(view);
  roomTexture2.clear();
  if (m_pImpl->_fadeEffect.effect == FadeEffect::Wobble) {
    m_pImpl->_fadeEffect.room->draw(roomTexture2, m_pImpl->_fadeEffect.cameraTopLeft);
  }
  roomTexture2.display();

  ngf::RenderTexture roomTexture3(target.getSize());
  ngf::Sprite sprite2(roomTexture2.getTexture());
  sprite2.draw(roomTexture3, {});
  roomTexture3.display();

  const ngf::Texture *texture1{nullptr};
  const ngf::Texture *texture2{nullptr};
  switch (m_pImpl->_fadeEffect.effect) {
  case FadeEffect::Wobble:
  case FadeEffect::In:texture1 = &roomTexture3.getTexture();
    texture2 = &roomWithEffectTexture.getTexture();
    break;
  case FadeEffect::Out:texture1 = &roomWithEffectTexture.getTexture();
    texture2 = &roomTexture3.getTexture();
    break;
  default:texture1 = &roomWithEffectTexture.getTexture();
    texture2 = &roomWithEffectTexture.getTexture();
    break;
  }
  fadeSprite.setTexture(*texture1);
  m_pImpl->_fadeShader.setUniform("u_texture2", *texture2);
  m_pImpl->_fadeShader.setUniform("u_fade", fade); // fade value between [0.f,1.f]
  m_pImpl->_fadeShader.setUniform("u_fadeToSep", m_pImpl->_fadeEffect.fadeToSepia ? 1 : 0);  // 1 to fade to sepia
  m_pImpl->_fadeShader.setUniform("u_movement",
                                  sinf(M_PI * fade) * m_pImpl->_fadeEffect.movement); // movement for wobble effect
  m_pImpl->_fadeShader.setUniform("u_timer", m_pImpl->_fadeEffect.elapsed.getTotalSeconds());
  states.shader = &m_pImpl->_fadeShader;

  // apply the room rotation
  auto pos = target.getView().getSize() / 2.f;
  fadeSprite.getTransform().setOrigin(pos);
  fadeSprite.getTransform().setPosition(pos);
  fadeSprite.getTransform().setRotation(m_pImpl->_pRoom->getRotation());
  fadeSprite.draw(target, states);

  // if we take a screenshot (for savegame) then stop drawing
  if (screenshot)
    return;

  // draw dialogs, hud
  m_pImpl->_dialogManager.draw(target, {});
  m_pImpl->drawHud(target);

  // draw walkboxes, actor texts
  auto orgView = target.getView();
  target.setView(view);
  m_pImpl->drawWalkboxes(target);
  m_pImpl->drawActorHotspot(target);
  m_pImpl->_talkingState.draw(target, {});
  m_pImpl->_pRoom->drawForeground(target, m_pImpl->_camera.getAt());
  target.setView(orgView);

  // draw actor icons
  if ((m_pImpl->_dialogManager.getState() == DialogManagerState::None)
      && m_pImpl->_inputActive) {
    m_pImpl->_actorIcons.draw(target, {});
  }

  // draw options or startscreen if necessary
  if (m_pImpl->_state == EngineState::Options) {
    m_pImpl->_optionsDialog.draw(target, {});
  } else if (m_pImpl->_state == EngineState::StartScreen) {
    m_pImpl->_startScreenDialog.draw(target, {});
  }

  // draw pause, cursor and no override icon
  m_pImpl->drawPause(target);
  m_pImpl->drawCursor(target);
  m_pImpl->drawCursorText(target);
  m_pImpl->drawNoOverride(target);
}

void Engine::setWalkboxesFlags(WalkboxesFlags show) { m_pImpl->_showDrawWalkboxes = show; }

WalkboxesFlags Engine::getWalkboxesFlags() const { return m_pImpl->_showDrawWalkboxes; }

void Engine::startDialog(const std::string &dialog, const std::string &node) {
  std::string actor;
  if (m_pImpl->_pCurrentActor)
    actor = m_pImpl->_pCurrentActor->getKey();
  m_pImpl->_dialogManager.start(actor, dialog, node);
}

void Engine::execute(const std::string &code) { m_pImpl->_pScriptExecute->execute(code); }

SoundDefinition *Engine::getSoundDefinition(const std::string &name) {
  return m_pImpl->_pScriptExecute->getSoundDefinition(name);
}

bool Engine::executeCondition(const std::string &code) { return m_pImpl->_pScriptExecute->executeCondition(code); }

std::string Engine::executeDollar(const std::string &code) { return m_pImpl->_pScriptExecute->executeDollar(code); }

void Engine::addSelectableActor(int index, Actor *pActor) {
  m_pImpl->_actorsIconSlots.at(index - 1).selectable = true;
  m_pImpl->_actorsIconSlots.at(index - 1).pActor = pActor;
}

void Engine::actorSlotSelectable(Actor *pActor, bool selectable) {
  auto it = std::find_if(m_pImpl->_actorsIconSlots.begin(), m_pImpl->_actorsIconSlots.end(),
                         [&pActor](auto &selectableActor) -> bool { return selectableActor.pActor == pActor; });
  if (it != m_pImpl->_actorsIconSlots.end()) {
    it->selectable = selectable;
  }
}

void Engine::actorSlotSelectable(int index, bool selectable) {
  m_pImpl->_actorsIconSlots.at(index - 1).selectable = selectable;
}

bool Engine::isActorSelectable(Actor *pActor) const {
  for (auto &&slot : m_pImpl->_actorsIconSlots) {
    if (slot.pActor == pActor)
      return slot.selectable;
  }
  return false;
}

ActorSlotSelectableMode Engine::getActorSlotSelectable() const { return m_pImpl->_actorIcons.getMode(); }

void Engine::setActorSlotSelectable(ActorSlotSelectableMode mode) { m_pImpl->_actorIcons.setMode(mode); }

void Engine::setUseFlag(UseFlag flag, Entity *object) {
  m_pImpl->_useFlag = flag;
  m_pImpl->_pUseObject = object;
}

void Engine::cutsceneOverride() {
  if (!m_pImpl->_pCutscene)
    return;
  m_pImpl->_pCutscene->cutsceneOverride();
}

void Engine::cutscene(std::unique_ptr<Cutscene> function) {
  m_pImpl->_pCutscene = function.get();
  addThread(std::move(function));
}

Cutscene *Engine::getCutscene() const { return m_pImpl->_pCutscene; }

bool Engine::inCutscene() const { return m_pImpl->_pCutscene && !m_pImpl->_pCutscene->isElapsed(); }

HSQOBJECT &Engine::getDefaultObject() { return m_pImpl->_pDefaultObject; }

void Engine::flashSelectableActor(bool on) { m_pImpl->_actorIcons.flash(on); }

const Verb *Engine::getActiveVerb() const { return m_pImpl->_hud.getCurrentVerb(); }

void Engine::fadeTo(FadeEffect effect, const ngf::TimeSpan &duration) {
  m_pImpl->_fadeEffect.effect = effect;
  m_pImpl->_fadeEffect.room = getRoom();
  m_pImpl->_fadeEffect.cameraTopLeft = m_pImpl->_camera.getRect().getTopLeft();
  m_pImpl->_fadeEffect.duration = duration;
  m_pImpl->_fadeEffect.movement = effect == FadeEffect::Wobble ? 0.005f : 0.f;
  m_pImpl->_fadeEffect.elapsed = ngf::TimeSpan::seconds(0);
}

FadeEffectParameters &Engine::getFadeParameters() {
  return m_pImpl->_fadeEffect;
}

void Engine::pushSentence(int id, Entity *pObj1, Entity *pObj2) {
  const Verb *pVerb = m_pImpl->_hud.getVerb(id);
  if (!pVerb)
    return;
  m_pImpl->_pVerbExecute->execute(pVerb, pObj1, pObj2);
}

void Engine::setSentence(std::unique_ptr<Sentence> sentence) {
  m_pImpl->_pSentence = std::move(sentence);
}

void Engine::stopSentence() {
  if (!m_pImpl->_pSentence)
    return;
  m_pImpl->_pSentence->stop();
  m_pImpl->_pSentence.reset();
}

void Engine::keyDown(const Input &key) {
  m_pImpl->_newKeyDowns.insert(key);
}

void Engine::keyUp(const Input &key) {
  auto it = m_pImpl->_newKeyDowns.find(key);
  if (it == m_pImpl->_newKeyDowns.end())
    return;
  m_pImpl->_newKeyDowns.erase(it);
}

void Engine::sayLineAt(glm::ivec2 pos, ngf::Color color, ngf::TimeSpan duration, const std::string &text) {
  m_pImpl->_talkingState.setTalkColor(color);
  auto size = getRoom()->getRoomSize();
  m_pImpl->_talkingState.setPosition(toDefaultView(pos, size));
  m_pImpl->_talkingState.setText(getText(text));
  m_pImpl->_talkingState.setDuration(duration);
}

void Engine::sayLineAt(glm::ivec2 pos, Entity &entity, const std::string &text) {
  auto size = getRoom()->getRoomSize();
  m_pImpl->_talkingState.setPosition(toDefaultView(pos, size));
  m_pImpl->_talkingState.loadLip(text, &entity);
}

void Engine::showOptions(bool visible) {
  m_pImpl->_state = visible ? EngineState::Options : EngineState::Game;
}

void Engine::quit() {
  m_pImpl->_pApp->quit();
  m_pImpl->_state = EngineState::Quit;
}

void Engine::run() {
  std::ifstream is("engge.nut");
  if (is.is_open()) {
    info("execute engge.nut");
    m_pImpl->_state = EngineState::Game;
    ScriptEngine::executeScript("engge.nut");
    return;
  }

  ng::info("execute boot script");
  ScriptEngine::executeNutScript("Defines.nut");
  ScriptEngine::executeNutScript("Boot.nut");
  execute("cameraInRoom(StartScreen)");
}

Inventory &Engine::getInventory() { return m_pImpl->_hud.getInventory(); }
Hud &Engine::getHud() { return m_pImpl->_hud; }

void Engine::saveGame(int slot) {
  Impl::SaveGameSystem saveGameSystem(m_pImpl.get());
  auto path = Impl::SaveGameSystem::getSlotPath(slot);
  std::filesystem::path screenshotPath(path);
  screenshotPath.replace_extension(".png");
  m_pImpl->captureScreen(screenshotPath.string());
  saveGameSystem.saveGame(path);
}

void Engine::loadGame(int slot) {
  Impl::SaveGameSystem saveGameSystem(m_pImpl.get());
  saveGameSystem.loadGame(Impl::SaveGameSystem::getSlotPath(slot).string());
}

void Engine::setAutoSave(bool autoSave) { m_pImpl->_autoSave = autoSave; }

bool Engine::getAutoSave() const { return m_pImpl->_autoSave; }

void Engine::allowSaveGames(bool allow) {
  m_pImpl->_optionsDialog.setSaveEnabled(allow);
}

Entity *Engine::getEntity(const std::string &name) {
  if (name == "agent" || name == "player")
    return m_pImpl->_pCurrentActor;

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
  m_pImpl->stopTalking();
}

void Engine::stopTalkingExcept(Entity *pEntity) const {
  m_pImpl->stopTalkingExcept(pEntity);
}

std::wstring SavegameSlot::getSaveTimeString() const {
  tm *ltm = localtime(&savetime);
  wchar_t buffer[120];
  // time format: "%b %d at %H:%M"
  auto format = Locator<TextDatabase>::get().getText(99944);
  wcsftime(buffer, 120, format.data(), ltm);
  return buffer;
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
