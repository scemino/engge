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
  m_pImpl->m_pEngine = this;
  m_pImpl->m_soundManager.setEngine(this);
  m_pImpl->m_dialogManager.setEngine(this);
  m_pImpl->m_actorIcons.setEngine(this);
  m_pImpl->m_camera.setEngine(this);
  m_pImpl->m_talkingState.setEngine(this);

  // load all messages
  std::stringstream s;
  auto lang =
      m_pImpl->m_preferences.getUserPreference<std::string>(PreferenceNames::Language,
                                                            PreferenceDefaultValues::Language);
  s << "ThimbleweedText_" << lang << ".tsv";
  Locator<TextDatabase>::get().load(s.str());

  m_pImpl->m_optionsDialog.setSaveEnabled(true);
  m_pImpl->m_optionsDialog.setEngine(this);
  m_pImpl->m_optionsDialog.setCallback([this]() {
    showOptions(false);
  });
  m_pImpl->m_startScreenDialog.setEngine(this);
  m_pImpl->m_startScreenDialog.setNewGameCallback([this]() {
    m_pImpl->m_state = EngineState::Game;
    m_pImpl->exitRoom(nullptr);
    ScriptEngine::call("start", true);
  });
  m_pImpl->m_startScreenDialog.setSlotCallback([this](int slot) {
    m_pImpl->m_state = EngineState::Game;
    loadGame(slot);
  });

  m_pImpl->m_preferences.subscribe([this](const std::string &name) {
    if (name == PreferenceNames::Language) {
      auto newLang = m_pImpl->m_preferences.getUserPreference<std::string>(PreferenceNames::Language,
                                                                           PreferenceDefaultValues::Language);
      m_pImpl->onLanguageChange(newLang);
    } else if (name == PreferenceNames::Fullscreen) {
      auto fullscreen = m_pImpl->m_preferences.getUserPreference(PreferenceNames::Fullscreen,
                                                                 PreferenceDefaultValues::Fullscreen);
      m_pImpl->m_pApp->getWindow().setFullscreen(fullscreen);
    }
  });
}

Engine::~Engine() = default;

int Engine::getFrameCounter() const { return m_pImpl->m_frameCounter; }

void Engine::setApplication(ng::EnggeApplication *app) { m_pImpl->m_pApp = app; }

const ng::EnggeApplication *Engine::getApplication() const { return m_pImpl->m_pApp; }
ng::EnggeApplication *Engine::getApplication() { return m_pImpl->m_pApp; }

ResourceManager &Engine::getResourceManager() { return m_pImpl->m_resourceManager; }

Room *Engine::getRoom() { return m_pImpl->m_pRoom; }

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

void Engine::addActor(std::unique_ptr<Actor> actor) { m_pImpl->m_actors.push_back(std::move(actor)); }

void Engine::addRoom(std::unique_ptr<Room> room) { m_pImpl->m_rooms.push_back(std::move(room)); }

std::vector<std::unique_ptr<Room>> &Engine::getRooms() { return m_pImpl->m_rooms; }

void Engine::addFunction(std::unique_ptr<Function> function) { m_pImpl->m_newFunctions.push_back(std::move(function)); }

void Engine::addCallback(std::unique_ptr<Callback> callback) { m_pImpl->m_callbacks.push_back(std::move(callback)); }

void Engine::removeCallback(int id) {
  auto it = std::find_if(m_pImpl->m_callbacks.begin(), m_pImpl->m_callbacks.end(),
                         [id](auto &callback) -> bool { return callback->getId() == id; });
  if (it != m_pImpl->m_callbacks.end()) {
    m_pImpl->m_callbacks.erase(it);
  }
}

std::vector<std::unique_ptr<Actor>> &Engine::getActors() { return m_pImpl->m_actors; }

Actor *Engine::getCurrentActor() { return m_pImpl->m_pCurrentActor; }

const VerbUiColors *Engine::getVerbUiColors(const std::string &name) const {
  if (name.empty()) {
    auto index = m_pImpl->getCurrentActorIndex();
    if (index == -1)
      return nullptr;
    return &m_pImpl->m_hud.getVerbUiColors(index);
  }
  for (int i = 0; i < static_cast<int>(m_pImpl->m_actorsIconSlots.size()); i++) {
    const auto &selectableActor = m_pImpl->m_actorsIconSlots.at(i);
    if (selectableActor.pActor && selectableActor.pActor->getKey() == name) {
      return &m_pImpl->m_hud.getVerbUiColors(i);
    }
  }
  return nullptr;
}

bool Engine::getInputActive() const { return m_pImpl->m_inputActive; }

void Engine::setInputState(int state) {
  if ((state & InputStateConstants::UI_INPUT_ON) == InputStateConstants::UI_INPUT_ON) {
    m_pImpl->m_inputActive = true;
  }
  if ((state & InputStateConstants::UI_INPUT_OFF) == InputStateConstants::UI_INPUT_OFF) {
    m_pImpl->m_inputActive = false;
  }
  if ((state & InputStateConstants::UI_VERBS_ON) == InputStateConstants::UI_VERBS_ON) {
    m_pImpl->m_inputVerbsActive = true;
  }
  if ((state & InputStateConstants::UI_VERBS_OFF) == InputStateConstants::UI_VERBS_OFF) {
    m_pImpl->m_inputVerbsActive = false;
  }
  if ((state & InputStateConstants::UI_CURSOR_ON) == InputStateConstants::UI_CURSOR_ON) {
    m_pImpl->m_showCursor = true;
  }
  if ((state & InputStateConstants::UI_CURSOR_OFF) == InputStateConstants::UI_CURSOR_OFF) {
    m_pImpl->m_showCursor = false;
  }
  if ((state & InputStateConstants::UI_HUDOBJECTS_ON) == InputStateConstants::UI_HUDOBJECTS_ON) {
    m_pImpl->m_inputHUD = true;
  }
  if ((state & InputStateConstants::UI_HUDOBJECTS_OFF) == InputStateConstants::UI_HUDOBJECTS_OFF) {
    m_pImpl->m_inputHUD = false;
  }
}

int Engine::getInputState() const {
  int inputState = 0;
  inputState |= (m_pImpl->m_inputActive ? InputStateConstants::UI_INPUT_ON : InputStateConstants::UI_INPUT_OFF);
  inputState |= (m_pImpl->m_inputVerbsActive ? InputStateConstants::UI_VERBS_ON : InputStateConstants::UI_VERBS_OFF);
  inputState |= (m_pImpl->m_showCursor ? InputStateConstants::UI_CURSOR_ON : InputStateConstants::UI_CURSOR_OFF);
  inputState |= (m_pImpl->m_inputHUD ? InputStateConstants::UI_HUDOBJECTS_ON : InputStateConstants::UI_HUDOBJECTS_OFF);
  return inputState;
}

void Engine::follow(Actor *pActor) {
  m_pImpl->m_pFollowActor = pActor;
  if (!pActor)
    return;

  auto pos = pActor->getPosition();
  auto pOldRoom = getRoom();
  setRoom(pActor->getRoom());
  if (pOldRoom != pActor->getRoom()) {
    m_pImpl->m_camera.at(pos);
  }
}

const Actor *Engine::getFollowActor() const {
  return m_pImpl->m_pFollowActor;
}

void Engine::setVerbExecute(std::unique_ptr<VerbExecute> verbExecute) {
  m_pImpl->m_pVerbExecute = std::move(verbExecute);
}

void Engine::setDefaultVerb() {
  m_pImpl->m_hud.setHoveredEntity(nullptr);
  auto index = m_pImpl->getCurrentActorIndex();
  if (index == -1)
    return;

  const auto &verbSlot = m_pImpl->m_hud.getVerbSlot(index);
  m_pImpl->m_hud.setCurrentVerb(&verbSlot.getVerb(0));
  m_pImpl->m_useFlag = UseFlag::None;
  m_pImpl->m_pUseObject = nullptr;
  m_pImpl->m_objId1 = 0;
  m_pImpl->m_pObj2 = nullptr;
}

void Engine::setScriptExecute(std::unique_ptr<ScriptExecute> scriptExecute) {
  m_pImpl->m_pScriptExecute = std::move(scriptExecute);
}

void Engine::addThread(std::unique_ptr<ThreadBase> thread) { m_pImpl->m_threads.push_back(std::move(thread)); }

std::vector<std::unique_ptr<ThreadBase>> &Engine::getThreads() { return m_pImpl->m_threads; }

glm::vec2 Engine::getMousePositionInRoom() const { return m_pImpl->m_mousePosInRoom; }

Preferences &Engine::getPreferences() { return m_pImpl->m_preferences; }

SoundManager &Engine::getSoundManager() { return m_pImpl->m_soundManager; }

DialogManager &Engine::getDialogManager() { return m_pImpl->m_dialogManager; }

Camera &Engine::getCamera() { return m_pImpl->m_camera; }

ngf::TimeSpan Engine::getTime() const { return m_pImpl->m_time; }

SQInteger Engine::setRoom(Room *pRoom) {
  if (!pRoom)
    return 0;

  auto pOldRoom = m_pImpl->m_pRoom;
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
    m_pImpl->m_camera.at(pos);
  }

  // enter current room
  return m_pImpl->enterRoom(pRoom, pDoor);
}

void Engine::setInputHUD(bool on) { m_pImpl->m_inputHUD = on; }

void Engine::setInputActive(bool active) {
  if (inCutscene())
    return;
  m_pImpl->m_inputActive = active;
  m_pImpl->m_showCursor = active;
}

void Engine::inputSilentOff() { m_pImpl->m_inputActive = false; }

void Engine::setInputVerbs(bool on) { m_pImpl->m_inputVerbsActive = on; }

void Engine::update(const ngf::TimeSpan &el) {
  if (m_pImpl->m_state == EngineState::Quit)
    return;

  roomEffect.RandomValue[0] = Locator<RandomNumberGenerator>::get().generateFloat(0, 1.f);
  roomEffect.iGlobalTime = fmod(m_pImpl->m_time.getTotalSeconds(), 1000.f);
  roomEffect.TimeLapse = roomEffect.iGlobalTime;

  auto gameSpeedFactor =
      getPreferences().getUserPreference(PreferenceNames::EnggeGameSpeedFactor,
                                         PreferenceDefaultValues::EnggeGameSpeedFactor);
  const ngf::TimeSpan elapsed(ngf::TimeSpan::seconds(el.getTotalSeconds() * gameSpeedFactor));
  m_pImpl->stopThreads();
  auto screenSize = m_pImpl->m_pRoom->getScreenSize();
  auto view = ngf::View{ngf::frect::fromPositionSize({0, 0}, screenSize)};
  m_pImpl->m_mousePos = m_pImpl->m_pApp->getRenderTarget()->mapPixelToCoords(ngf::Mouse::getPosition(), view);
  if (m_pImpl->m_pRoom && m_pImpl->m_pRoom->getName() != "Void") {
    auto screenMouse = toDefaultView((glm::ivec2) m_pImpl->m_mousePos, screenSize);
    m_pImpl->m_hud.setMousePosition(screenMouse);
    m_pImpl->m_dialogManager.setMousePosition(screenMouse);
  }
  if (m_pImpl->m_state == EngineState::Options) {
    m_pImpl->m_optionsDialog.update(elapsed);
  } else if (m_pImpl->m_state == EngineState::StartScreen) {
    m_pImpl->m_startScreenDialog.update(elapsed);
    if (m_pImpl->m_state == EngineState::Quit)
      return;
  }

  if (m_pImpl->m_state == EngineState::Paused) {
    m_pImpl->updateKeys();
    return;
  }

  // update fade effect
  m_pImpl->m_fadeEffect.elapsed += elapsed;
  m_pImpl->m_talkingState.update(elapsed);

  m_pImpl->m_frameCounter++;
  auto &io = ImGui::GetIO();
  auto wasMouseDown = m_pImpl->m_isMouseDown && !io.WantCaptureMouse;
  auto wasMouseRightDown = m_pImpl->m_isMouseRightDown;
  m_pImpl->m_isMouseDown =
      ngf::Mouse::isButtonPressed(ngf::Mouse::Button::Left) && !io.WantCaptureMouse;
  if (!wasMouseDown || !m_pImpl->m_isMouseDown) {
    m_pImpl->m_mouseDownTime = ngf::TimeSpan::seconds(0);
    m_pImpl->run(false);
  } else {
    m_pImpl->m_mouseDownTime += elapsed;
    if (m_pImpl->m_mouseDownTime > ngf::TimeSpan::seconds(0.5f)) {
      m_pImpl->run(true);
    }
  }
  m_pImpl->m_isMouseRightDown = ngf::Mouse::isButtonPressed(ngf::Mouse::Button::Right) && !io.WantCaptureMouse;
  bool isRightClick = wasMouseRightDown != m_pImpl->m_isMouseRightDown && !m_pImpl->m_isMouseRightDown;
  auto isMouseClick = wasMouseDown != m_pImpl->m_isMouseDown && !m_pImpl->m_isMouseDown;

  m_pImpl->m_time += elapsed;
  m_pImpl->m_noOverrideElapsed += elapsed;

  m_pImpl->m_camera.update(elapsed);
  m_pImpl->m_soundManager.update(elapsed);
  m_pImpl->updateCutscene(elapsed);
  m_pImpl->updateFunctions(elapsed);
  m_pImpl->updateSentence(elapsed);
  m_pImpl->updateKeys();

  if (!m_pImpl->m_pRoom || m_pImpl->m_pRoom->getName() == "Void")
    return;

  m_pImpl->updateRoomScalings();

  m_pImpl->m_pRoom->update(elapsed);
  for (auto &pActor : m_pImpl->m_actors) {
    if (!pActor || pActor->getRoom() == m_pImpl->m_pRoom)
      continue;
    pActor->update(elapsed);
  }

  m_pImpl->updateActorIcons(elapsed);

  if (m_pImpl->m_state == EngineState::Options)
    return;

  m_pImpl->m_cursorDirection = CursorDirection::None;
  m_pImpl->updateMouseCursor();

  auto mousePos =
      glm::vec2(m_pImpl->m_mousePos.x, m_pImpl->m_pRoom->getScreenSize().y - m_pImpl->m_mousePos.y);
  m_pImpl->m_mousePosInRoom = mousePos + m_pImpl->m_camera.getRect().getTopLeft();

  m_pImpl->m_dialogManager.update(elapsed);

  m_pImpl->m_hud.setActive(
      m_pImpl->m_inputVerbsActive && m_pImpl->m_dialogManager.getState() == DialogManagerState::None
          && m_pImpl->m_pRoom->getFullscreen() != 1);
  m_pImpl->m_hud.setHoveredEntity(m_pImpl->getHoveredEntity(m_pImpl->m_mousePosInRoom));
  m_pImpl->updateHoveredEntity(isRightClick);

  if (m_pImpl->m_pCurrentActor) {
    auto &objects = m_pImpl->m_pCurrentActor->getObjects();
    for (auto &object : objects) {
      object->update(elapsed);
    }
  }

  m_pImpl->m_hud.update(elapsed);

  if (m_pImpl->m_actorIcons.isMouseOver())
    return;

  if (isMouseClick && m_pImpl->clickedAt(m_pImpl->m_mousePosInRoom))
    return;

  if (!m_pImpl->m_inputActive)
    return;

  m_pImpl->updateKeyboard();

  if (m_pImpl->m_dialogManager.getState() != DialogManagerState::None) {
    auto rightClickSkipsDialog = getPreferences().getUserPreference(PreferenceNames::RightClickSkipsDialog,
                                                                    PreferenceDefaultValues::RightClickSkipsDialog);
    if (rightClickSkipsDialog && isRightClick) {
      m_pImpl->skipText();
    }
    return;
  }

  if (!m_pImpl->m_pCurrentActor)
    return;

  if (!isMouseClick && !isRightClick && !m_pImpl->m_isMouseDown)
    return;

  m_pImpl->m_hud.setVisible(true);
  m_pImpl->m_actorIcons.setVisible(true);
  m_pImpl->m_cursorVisible = true;
  stopSentence();

  const auto *pVerb = m_pImpl->getHoveredVerb();
  // input click on a verb ?
  if (m_pImpl->m_hud.getActive() && pVerb) {
    m_pImpl->onVerbClick(pVerb);
    return;
  }

  if (!isMouseClick && !isRightClick) {
    if (!pVerb && !m_pImpl->m_hud.getHoveredEntity())
      m_pImpl->m_pCurrentActor->walkTo(m_pImpl->m_mousePosInRoom);
    return;
  }

  if (m_pImpl->m_hud.getHoveredEntity()) {
    ScriptEngine::rawCall("onObjectClick", m_pImpl->m_hud.getHoveredEntity());
    auto pVerbOverride = m_pImpl->m_hud.getVerbOverride();
    if (!pVerbOverride) {
      pVerbOverride = m_pImpl->m_hud.getCurrentVerb();
    }
    pVerbOverride = m_pImpl->overrideVerb(pVerbOverride);
    auto pObj1 = EntityManager::getScriptObjectFromId<Entity>(m_pImpl->m_objId1);
    pObj1 = pVerbOverride->id == VerbConstants::VERB_TALKTO ? m_pImpl->getEntity(pObj1) : pObj1;
    auto
        pObj2 = pVerbOverride->id == VerbConstants::VERB_GIVE ? m_pImpl->getEntity(m_pImpl->m_pObj2) : m_pImpl->m_pObj2;
    if (pObj1) {
      m_pImpl->m_pVerbExecute->execute(pVerbOverride, pObj1, pObj2);
    }
    return;
  }

  if (m_pImpl->m_hud.isMouseOver())
    return;

  m_pImpl->m_pCurrentActor->walkTo(m_pImpl->m_mousePosInRoom);
  setDefaultVerb();
}

void Engine::setCurrentActor(Actor *pCurrentActor, bool userSelected) {
  m_pImpl->m_pCurrentActor = pCurrentActor;

  int currentActorIndex = m_pImpl->getCurrentActorIndex();
  m_pImpl->m_hud.setCurrentActorIndex(currentActorIndex);
  m_pImpl->m_hud.setCurrentActor(m_pImpl->m_pCurrentActor);

  ScriptEngine::rawCall("onActorSelected", pCurrentActor, userSelected);
  auto pRoom = pCurrentActor ? pCurrentActor->getRoom() : nullptr;
  if (pRoom) {
    if (ScriptEngine::rawExists(pRoom, "onActorSelected")) {
      ScriptEngine::rawCall(pRoom, "onActorSelected", pCurrentActor, userSelected);
    }
  }

  if (m_pImpl->m_pCurrentActor) {
    follow(m_pImpl->m_pCurrentActor);
  }
}

void Engine::draw(ngf::RenderTarget &target, bool screenshot) const {
  if (!m_pImpl->m_pRoom)
    return;

  // update room shader if necessary
  ngf::RenderStates states;
  auto effect = m_pImpl->m_pRoom->getEffect();
  if (m_pImpl->m_roomEffect != effect) {
    if (effect == RoomEffectConstants::EFFECT_BLACKANDWHITE) {
      m_pImpl->m_roomShader.load(Shaders::vertexShader, Shaders::bwFragmentShader);
    } else if (effect == RoomEffectConstants::EFFECT_EGA) {
      m_pImpl->m_roomShader.load(Shaders::vertexShader, Shaders::egaFragmenShader);
    } else if (effect == RoomEffectConstants::EFFECT_GHOST) {
      m_pImpl->m_roomShader.load(Shaders::vertexShader, Shaders::ghostFragmentShader);
    } else if (effect == RoomEffectConstants::EFFECT_SEPIA) {
      m_pImpl->m_roomShader.load(Shaders::vertexShader, Shaders::sepiaFragmentShader);
    } else if (effect == RoomEffectConstants::EFFECT_VHS) {
      m_pImpl->m_roomShader.load(Shaders::vertexShader, Shaders::vhsFragmentShader);
    }
    m_pImpl->m_roomEffect = effect;
  }
  states.shader = &m_pImpl->m_roomShader;
  if (effect == RoomEffectConstants::EFFECT_GHOST) {
    // don't remove the fmod function or you will have float overflow with the shader and the effect will look strange
    m_pImpl->m_roomShader.setUniform("iGlobalTime", roomEffect.iGlobalTime);
    m_pImpl->m_roomShader.setUniform("iFade", roomEffect.iFade);
    m_pImpl->m_roomShader.setUniform("wobbleIntensity", roomEffect.wobbleIntensity);
    m_pImpl->m_roomShader.setUniform("shadows", roomEffect.shadows);
    m_pImpl->m_roomShader.setUniform("midtones", roomEffect.midtones);
    m_pImpl->m_roomShader.setUniform("highlights", roomEffect.highlights);
  } else if (effect == RoomEffectConstants::EFFECT_SEPIA) {
    m_pImpl->m_roomShader.setUniform("sepiaFlicker", roomEffect.sepiaFlicker);
    m_pImpl->m_roomShader.setUniformArray("RandomValue", roomEffect.RandomValue.data(), 5);
    m_pImpl->m_roomShader.setUniform("TimeLapse", roomEffect.TimeLapse);
  } else if (effect == RoomEffectConstants::EFFECT_VHS) {
    m_pImpl->m_roomShader.setUniform("iGlobalTime", roomEffect.iGlobalTime);
    m_pImpl->m_roomShader.setUniform("iNoiseThreshold", roomEffect.iNoiseThreshold);
  } else if (effect == RoomEffectConstants::EFFECT_NONE) {
    states.shader = nullptr;
  }

  // render the room to a texture, this allows to create a post process effect: room effect
  ngf::RenderTexture roomTexture(target.getSize());
  auto screenSize = m_pImpl->m_pRoom->getScreenSize();
  ngf::View view(ngf::frect::fromPositionSize({0, 0}, screenSize));
  roomTexture.setView(view);
  roomTexture.clear();
  m_pImpl->m_pRoom->draw(roomTexture, m_pImpl->m_camera.getRect().getTopLeft());
  roomTexture.display();

  // then render a sprite with this texture and apply the room effect
  ngf::RenderTexture roomWithEffectTexture(target.getSize());
  roomWithEffectTexture.clear();
  ngf::Sprite sprite(roomTexture.getTexture());
  sprite.draw(roomWithEffectTexture, states);

  // and render overlay
  ngf::RectangleShape fadeShape;
  fadeShape.setSize(roomWithEffectTexture.getSize());
  fadeShape.setColor(m_pImpl->m_pRoom->getOverlayColor());
  fadeShape.draw(roomWithEffectTexture, {});
  roomWithEffectTexture.display();

  // render fade
  ngf::Sprite fadeSprite;
  float fade = m_pImpl->m_fadeEffect.effect == FadeEffect::None ? 0.f :
               std::clamp(
                   m_pImpl->m_fadeEffect.elapsed.getTotalSeconds() / m_pImpl->m_fadeEffect.duration.getTotalSeconds(),
                   0.f, 1.f);
  ngf::RenderTexture roomTexture2(target.getSize());
  roomTexture2.setView(view);
  roomTexture2.clear();
  if (m_pImpl->m_fadeEffect.effect == FadeEffect::Wobble) {
    m_pImpl->m_fadeEffect.room->draw(roomTexture2, m_pImpl->m_fadeEffect.cameraTopLeft);
  }
  roomTexture2.display();

  ngf::RenderTexture roomTexture3(target.getSize());
  ngf::Sprite sprite2(roomTexture2.getTexture());
  sprite2.draw(roomTexture3, {});
  roomTexture3.display();

  const ngf::Texture *texture1{nullptr};
  const ngf::Texture *texture2{nullptr};
  switch (m_pImpl->m_fadeEffect.effect) {
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
  m_pImpl->m_fadeShader.setUniform("u_texture2", *texture2);
  m_pImpl->m_fadeShader.setUniform("u_fade", fade); // fade value between [0.f,1.f]
  m_pImpl->m_fadeShader.setUniform("u_fadeToSep", m_pImpl->m_fadeEffect.fadeToSepia ? 1 : 0);  // 1 to fade to sepia
  m_pImpl->m_fadeShader.setUniform("u_movement",
                                   sinf(M_PI * fade) * m_pImpl->m_fadeEffect.movement); // movement for wobble effect
  m_pImpl->m_fadeShader.setUniform("u_timer", m_pImpl->m_fadeEffect.elapsed.getTotalSeconds());
  states.shader = &m_pImpl->m_fadeShader;

  // apply the room rotation
  auto pos = target.getView().getSize() / 2.f;
  fadeSprite.getTransform().setOrigin(pos);
  fadeSprite.getTransform().setPosition(pos);
  fadeSprite.getTransform().setRotation(m_pImpl->m_pRoom->getRotation());
  fadeSprite.draw(target, states);

  // if we take a screenshot (for savegame) then stop drawing
  if (screenshot)
    return;

  // draw dialogs, hud
  m_pImpl->m_dialogManager.draw(target, {});
  m_pImpl->drawHud(target);

  // draw walkboxes, actor texts
  auto orgView = target.getView();
  target.setView(view);
  m_pImpl->drawWalkboxes(target);
  m_pImpl->drawActorHotspot(target);
  const auto &objects = m_pImpl->m_pRoom->getObjects();
  std::for_each(objects.begin(), objects.end(), [this, &target](const auto &pObj) {
    m_pImpl->drawObjectHotspot(*pObj, target);
    m_pImpl->drawDebugHotspot(*pObj, target);
  });

  ngf::RenderStates statesObjects;
  auto &lightingShader = m_pImpl->m_pRoom->getLightingShader();
  statesObjects.shader = &lightingShader;
  lightingShader.setNumberLights(0);
  lightingShader.setAmbientColor(ngf::Colors::White);
  std::for_each(objects.begin(), objects.end(), [this, &target, statesObjects](const auto &pObj) {
    m_pImpl->drawScreenSpace(*pObj, target, statesObjects);
  });

  m_pImpl->m_talkingState.draw(target, {});
  m_pImpl->m_pRoom->drawForeground(target, m_pImpl->m_camera.getAt());
  target.setView(orgView);

  // draw actor icons
  if ((m_pImpl->m_dialogManager.getState() == DialogManagerState::None)
      && m_pImpl->m_inputActive) {
    m_pImpl->m_actorIcons.draw(target, {});
  }

  // draw options or startscreen if necessary
  if (m_pImpl->m_state == EngineState::Options) {
    m_pImpl->m_optionsDialog.draw(target, {});
  } else if (m_pImpl->m_state == EngineState::StartScreen) {
    m_pImpl->m_startScreenDialog.draw(target, {});
  }

  // draw pause, cursor and no override icon
  m_pImpl->drawPause(target);
  m_pImpl->drawCursor(target);
  m_pImpl->drawCursorText(target);
  m_pImpl->drawNoOverride(target);
}

void Engine::setWalkboxesFlags(WalkboxesFlags show) { m_pImpl->m_showDrawWalkboxes = show; }

WalkboxesFlags Engine::getWalkboxesFlags() const { return m_pImpl->m_showDrawWalkboxes; }

void Engine::startDialog(const std::string &dialog, const std::string &node) {
  std::string actor;
  if (m_pImpl->m_pCurrentActor)
    actor = m_pImpl->m_pCurrentActor->getKey();
  m_pImpl->m_dialogManager.start(actor, dialog, node);
}

void Engine::execute(const std::string &code) { m_pImpl->m_pScriptExecute->execute(code); }

SoundDefinition *Engine::getSoundDefinition(const std::string &name) {
  return m_pImpl->m_pScriptExecute->getSoundDefinition(name);
}

bool Engine::executeCondition(const std::string &code) { return m_pImpl->m_pScriptExecute->executeCondition(code); }

std::string Engine::executeDollar(const std::string &code) { return m_pImpl->m_pScriptExecute->executeDollar(code); }

void Engine::addSelectableActor(int index, Actor *pActor) {
  m_pImpl->m_actorsIconSlots.at(index - 1).selectable = true;
  m_pImpl->m_actorsIconSlots.at(index - 1).pActor = pActor;
}

void Engine::actorSlotSelectable(Actor *pActor, bool selectable) {
  auto it = std::find_if(m_pImpl->m_actorsIconSlots.begin(), m_pImpl->m_actorsIconSlots.end(),
                         [&pActor](auto &selectableActor) -> bool { return selectableActor.pActor == pActor; });
  if (it != m_pImpl->m_actorsIconSlots.end()) {
    it->selectable = selectable;
  }
}

void Engine::actorSlotSelectable(int index, bool selectable) {
  m_pImpl->m_actorsIconSlots.at(index - 1).selectable = selectable;
}

bool Engine::isActorSelectable(Actor *pActor) const {
  for (auto &&slot : m_pImpl->m_actorsIconSlots) {
    if (slot.pActor == pActor)
      return slot.selectable;
  }
  return false;
}

ActorSlotSelectableMode Engine::getActorSlotSelectable() const { return m_pImpl->m_actorIcons.getMode(); }

void Engine::setActorSlotSelectable(ActorSlotSelectableMode mode) { m_pImpl->m_actorIcons.setMode(mode); }

void Engine::setUseFlag(UseFlag flag, Entity *object) {
  m_pImpl->m_useFlag = flag;
  m_pImpl->m_pUseObject = object;
}

void Engine::cutsceneOverride() {
  if (!m_pImpl->m_pCutscene)
    return;
  m_pImpl->m_pCutscene->cutsceneOverride();
}

void Engine::cutscene(std::unique_ptr<Cutscene> function) {
  m_pImpl->m_pCutscene = function.get();
  addThread(std::move(function));
}

Cutscene *Engine::getCutscene() const { return m_pImpl->m_pCutscene; }

bool Engine::inCutscene() const { return m_pImpl->m_pCutscene && !m_pImpl->m_pCutscene->isElapsed(); }

HSQOBJECT &Engine::getDefaultObject() { return m_pImpl->m_pDefaultObject; }

void Engine::flashSelectableActor(bool on) { m_pImpl->m_actorIcons.flash(on); }

const Verb *Engine::getActiveVerb() const { return m_pImpl->m_hud.getCurrentVerb(); }

void Engine::fadeTo(FadeEffect effect, const ngf::TimeSpan &duration) {
  m_pImpl->m_fadeEffect.effect = effect;
  m_pImpl->m_fadeEffect.room = getRoom();
  m_pImpl->m_fadeEffect.cameraTopLeft = m_pImpl->m_camera.getRect().getTopLeft();
  m_pImpl->m_fadeEffect.duration = duration;
  m_pImpl->m_fadeEffect.movement = effect == FadeEffect::Wobble ? 0.005f : 0.f;
  m_pImpl->m_fadeEffect.elapsed = ngf::TimeSpan::seconds(0);
}

FadeEffectParameters &Engine::getFadeParameters() {
  return m_pImpl->m_fadeEffect;
}

void Engine::pushSentence(int id, Entity *pObj1, Entity *pObj2) {
  const Verb *pVerb = m_pImpl->m_hud.getVerb(id);
  if (!pVerb)
    return;
  m_pImpl->m_pVerbExecute->execute(pVerb, pObj1, pObj2);
}

void Engine::setSentence(std::unique_ptr<Sentence> sentence) {
  m_pImpl->m_pSentence = std::move(sentence);
}

void Engine::stopSentence() {
  if (!m_pImpl->m_pSentence)
    return;
  m_pImpl->m_pSentence->stop();
  m_pImpl->m_pSentence.reset();
}

void Engine::keyDown(const Input &key) {
  m_pImpl->m_newKeyDowns.insert(key);
}

void Engine::keyUp(const Input &key) {
  auto it = m_pImpl->m_newKeyDowns.find(key);
  if (it == m_pImpl->m_newKeyDowns.end())
    return;
  m_pImpl->m_newKeyDowns.erase(it);
}

void Engine::sayLineAt(glm::ivec2 pos, ngf::Color color, ngf::TimeSpan duration, const std::string &text) {
  m_pImpl->m_talkingState.setTalkColor(color);
  auto size = getRoom()->getRoomSize();
  m_pImpl->m_talkingState.setPosition(toDefaultView(pos, size));
  m_pImpl->m_talkingState.setText(getText(text));
  m_pImpl->m_talkingState.setDuration(duration);
}

void Engine::sayLineAt(glm::ivec2 pos, Entity &entity, const std::string &text) {
  auto size = getRoom()->getRoomSize();
  m_pImpl->m_talkingState.setPosition(toDefaultView(pos, size));
  m_pImpl->m_talkingState.loadLip(text, &entity);
}

void Engine::showOptions(bool visible) {
  m_pImpl->m_state = visible ? EngineState::Options : EngineState::Game;
}

void Engine::quit() {
  m_pImpl->m_pApp->quit();
  m_pImpl->m_state = EngineState::Quit;
}

void Engine::run() {
  std::ifstream is("engge.nut");
  if (is.is_open()) {
    info("execute engge.nut");
    m_pImpl->m_state = EngineState::Game;
    ScriptEngine::executeScript("engge.nut");
    return;
  }

  ng::info("execute boot script");
  ScriptEngine::executeNutScript("Defines.nut");
  ScriptEngine::executeNutScript("Boot.nut");
  execute("cameraInRoom(StartScreen)");
}

Inventory &Engine::getInventory() { return m_pImpl->m_hud.getInventory(); }
Hud &Engine::getHud() { return m_pImpl->m_hud; }

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

void Engine::setAutoSave(bool autoSave) { m_pImpl->m_autoSave = autoSave; }

bool Engine::getAutoSave() const { return m_pImpl->m_autoSave; }

void Engine::allowSaveGames(bool allow) {
  m_pImpl->m_optionsDialog.setSaveEnabled(allow);
}

Entity *Engine::getEntity(const std::string &name) {
  if (name == "agent" || name == "player")
    return m_pImpl->m_pCurrentActor;

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
