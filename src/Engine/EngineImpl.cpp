#include "EngineImpl.hpp"
#include <engge/EnggeApplication.hpp>
#include <engge/Graphics/Text.hpp>
#include <engge/Graphics/AnimDrawable.hpp>
#include "../Graphics/PathDrawable.hpp"

namespace ng {
ngf::Color toColor(const ObjectType &type) {
  ngf::Color color;
  switch (type) {
  case ObjectType::Object:color = ngf::Colors::Red;
    break;
  case ObjectType::Spot:color = ngf::Colors::Green;
    break;
  case ObjectType::Trigger:color = ngf::Colors::Magenta;
    break;
  case ObjectType::Prop:color = ngf::Colors::Blue;
    break;
  }
  return color;
}

CursorDirection operator|=(CursorDirection &lhs, CursorDirection rhs) {
  lhs = static_cast<CursorDirection>(static_cast<std::underlying_type<CursorDirection>::type>(lhs) |
      static_cast<std::underlying_type<CursorDirection>::type>(rhs));
  return lhs;
}

bool operator&(CursorDirection lhs, CursorDirection rhs) {
  return static_cast<CursorDirection>(static_cast<std::underlying_type<CursorDirection>::type>(lhs) &
      static_cast<std::underlying_type<CursorDirection>::type>(rhs)) >
      CursorDirection::None;
}

Engine::Impl::Impl()
    : m_resourceManager(Locator<ResourceManager>::get()),
      m_preferences(Locator<Preferences>::get()),
      m_soundManager(Locator<SoundManager>::get()),
      m_actorIcons(m_actorsIconSlots, m_hud, m_pCurrentActor) {
  m_hud.setTextureManager(&m_resourceManager);
  sq_resetobject(&m_pDefaultObject);

  Locator<CommandManager>::get().registerCommands(
      {
          {EngineCommands::SkipText, [this]() { skipText(); }},
          {EngineCommands::SkipCutscene, [this] { skipCutscene(); }},
          {EngineCommands::PauseGame, [this] { pauseGame(); }},
          {EngineCommands::SelectActor1, [this] { selectActor(1); }},
          {EngineCommands::SelectActor2, [this] { selectActor(2); }},
          {EngineCommands::SelectActor3, [this] { selectActor(3); }},
          {EngineCommands::SelectActor4, [this] { selectActor(4); }},
          {EngineCommands::SelectActor5, [this] { selectActor(5); }},
          {EngineCommands::SelectActor6, [this] { selectActor(6); }},
          {EngineCommands::SelectPreviousActor, [this] { selectPreviousActor(); }},
          {EngineCommands::SelectNextActor, [this] { selectNextActor(); }},
          {EngineCommands::SelectChoice1, [this] { m_dialogManager.choose(1); }},
          {EngineCommands::SelectChoice2, [this] { m_dialogManager.choose(2); }},
          {EngineCommands::SelectChoice3, [this] { m_dialogManager.choose(3); }},
          {EngineCommands::SelectChoice4, [this] { m_dialogManager.choose(4); }},
          {EngineCommands::SelectChoice5, [this] { m_dialogManager.choose(5); }},
          {EngineCommands::SelectChoice6, [this] { m_dialogManager.choose(6); }},
          {EngineCommands::ShowOptions, [this] { m_pEngine->showOptions(true); }},
          {EngineCommands::ToggleHud, [this] {
            m_hud.setVisible(!m_cursorVisible);
            m_actorIcons.setVisible(!m_cursorVisible);
            m_cursorVisible = !m_cursorVisible;
          }}
      });
  Locator<CommandManager>::get().registerPressedCommand(EngineCommands::ShowHotspots, [this](bool down) {
    m_preferences.setTempPreference(TempPreferenceNames::ShowHotspot, down);
  });

  m_fadeShader.load(Shaders::vertexShader, Shaders::fadeFragmentShader);
  uint32_t pixels[4]{0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF};
  m_blackTexture.loadFromMemory({2, 2}, pixels);
}

void Engine::Impl::pauseGame() {
  m_state = m_state == EngineState::Game ? EngineState::Paused : EngineState::Game;
  if (m_state == EngineState::Paused) {
    // pause all pauseable threads
    for (auto &thread : m_threads) {
      if (thread->isPauseable() && !thread->isStopped()) {
        thread->pause();
      }
    }
    m_soundManager.pauseAllSounds();
  } else {
    // resume all pauseable threads
    for (auto &thread : m_threads) {
      if (thread->isPauseable() && !thread->isStopped()) {
        thread->resume();
      }
    }
    m_soundManager.resumeAllSounds();
  }
}

void Engine::Impl::selectActor(int index) {
  if (index <= 0 || index > static_cast<int>(m_actorsIconSlots.size()))
    return;
  const auto &slot = m_actorsIconSlots[index - 1];
  if (!slot.selectable)
    return;
  m_pEngine->setCurrentActor(slot.pActor, true);
}

void Engine::Impl::selectPreviousActor() {
  auto currentActorIndex = getCurrentActorIndex();
  if (currentActorIndex == -1)
    return;
  auto size = static_cast<int>(m_actorsIconSlots.size());
  for (auto i = 0; i < size; i++) {
    auto index = currentActorIndex - i - 1;
    if (index < 0)
      index += size - 1;
    if (index == currentActorIndex)
      return;
    const auto &slot = m_actorsIconSlots[index];
    if (slot.selectable) {
      m_pEngine->setCurrentActor(slot.pActor, true);
      return;
    }
  }
}

void Engine::Impl::selectNextActor() {
  auto currentActorIndex = getCurrentActorIndex();
  if (currentActorIndex == -1)
    return;
  auto size = static_cast<int>(m_actorsIconSlots.size());
  for (auto i = 0; i < size; i++) {
    auto index = (currentActorIndex + i + 1) % size;
    if (index == currentActorIndex)
      return;
    const auto &slot = m_actorsIconSlots[index];
    if (slot.selectable) {
      m_pEngine->setCurrentActor(slot.pActor, true);
      return;
    }
  }
}

void Engine::Impl::skipCutscene() {
  if (m_pEngine->inCutscene()) {
    if (m_pCutscene && m_pCutscene->hasCutsceneOverride()) {
      m_pEngine->cutsceneOverride();
    } else {
      m_noOverrideElapsed = ngf::TimeSpan::seconds(0);
    }
  }
}

void Engine::Impl::skipText() const {
  if (m_dialogManager.getState() == DialogManagerState::Active) {
    stopTalking();
  }
}

void Engine::Impl::onLanguageChange(const std::string &lang) {
  std::stringstream ss;
  ss << "ThimbleweedText_" << lang << ".tsv";
  Locator<TextDatabase>::get().load(ss.str());

  ScriptEngine::call("onLanguageChange");
}

SQInteger Engine::Impl::exitRoom(Object *pObject) {
  m_pEngine->setDefaultVerb();
  m_talkingState.stop();

  if (!m_pRoom)
    return 0;

  auto pOldRoom = m_pRoom;

  actorExit();

  // call exit room function
  auto nparams = ScriptEngine::getParameterCount(pOldRoom, "exit");
  trace("call exit room function of {} ({} params)", pOldRoom->getName(), nparams);

  if (nparams == 2) {
    auto pRoom = pObject ? pObject->getRoom() : nullptr;
    ScriptEngine::rawCall(pOldRoom, "exit", pRoom);
  } else {
    ScriptEngine::rawCall(pOldRoom, "exit");
  }

  pOldRoom->exit();

  ScriptEngine::rawCall("exitedRoom", pOldRoom);

  // stop all local threads
  std::for_each(m_threads.begin(), m_threads.end(), [](auto &pThread) {
    if (!pThread->isGlobal())
      pThread->stop();
  });

  return 0;
}

void Engine::Impl::actorEnter() const {
  if (!m_pCurrentActor)
    return;

  m_pCurrentActor->stopWalking();
  ScriptEngine::rawCall("actorEnter", m_pCurrentActor);

  if (!m_pRoom)
    return;

  if (ScriptEngine::rawExists(m_pRoom, "actorEnter")) {
    ScriptEngine::rawCall(m_pRoom, "actorEnter", m_pCurrentActor);
  }
}

void Engine::Impl::actorExit() const {
  if (!m_pCurrentActor || !m_pRoom)
    return;

  if (ScriptEngine::rawExists(m_pRoom, "actorExit")) {
    ScriptEngine::rawCall(m_pRoom, "actorExit", m_pCurrentActor);
  }
}

SQInteger Engine::Impl::enterRoom(Room *pRoom, Object *pObject) const {
  // call enter room function
  trace("call enter room function of {}", pRoom->getName());
  auto nparams = ScriptEngine::getParameterCount(pRoom, "enter");
  if (nparams == 2) {
    ScriptEngine::rawCall(pRoom, "enter", pObject);
  } else {
    ScriptEngine::rawCall(pRoom, "enter");
  }

  actorEnter();

  auto lang = Locator<Preferences>::get().getUserPreference<std::string>(PreferenceNames::Language,
                                                                         PreferenceDefaultValues::Language);
  const auto &spriteSheet = pRoom->getSpriteSheet();
  auto &objects = pRoom->getObjects();
  for (auto &obj : objects) {
    for (auto &anim : obj->getAnims()) {
      for (size_t i = 0; i < anim.frames.size(); ++i) {
        auto &frame = anim.frames.at(i);
        auto name = frame.name;
        if (!endsWith(name, "_en"))
          continue;

        checkLanguage(name);
        anim.frames[i] = spriteSheet.getItem(name);
      }
    }
    if (obj->getId() == 0 || obj->isTemporary())
      continue;

    if (ScriptEngine::rawExists(obj.get(), "enter")) {
      ScriptEngine::rawCall(obj.get(), "enter");
    }
  }

  ScriptEngine::rawCall("enteredRoom", pRoom);

  return 0;
}

void Engine::Impl::run(bool state) {
  if (m_run != state) {
    m_run = state;
    if (m_pCurrentActor) {
      ScriptEngine::objCall(m_pCurrentActor, "run", state);
    }
  }
}

void Engine::Impl::setCurrentRoom(Room *pRoom) {
  // reset fade effect if we change the room except for wobble effect
  if (m_fadeEffect.effect != FadeEffect::Wobble) {
    m_fadeEffect.effect = FadeEffect::None;
  }

  if (pRoom) {
    ScriptEngine::set("currentRoom", pRoom);
  }
  m_camera.resetBounds();
  m_pRoom = pRoom;
  m_camera.at(glm::vec2(0, 0));
}

void Engine::Impl::updateCutscene(const ngf::TimeSpan &elapsed) {
  if (m_pCutscene) {
    (*m_pCutscene)(elapsed);
    if (m_pCutscene->isElapsed()) {
      m_pCutscene = nullptr;
    }
  }
}

void Engine::Impl::updateSentence(const ngf::TimeSpan &elapsed) const {
  if (!m_pSentence)
    return;
  (*m_pSentence)(elapsed);
  if (!m_pSentence->isElapsed())
    return;
  m_pEngine->stopSentence();
}

void Engine::Impl::updateFunctions(const ngf::TimeSpan &elapsed) {
  for (auto &function : m_newFunctions) {
    m_functions.push_back(std::move(function));
  }
  m_newFunctions.clear();
  for (auto &function : m_functions) {
    (*function)(elapsed);
  }
  m_functions.erase(std::remove_if(m_functions.begin(), m_functions.end(),
                                   [](std::unique_ptr<Function> &f) { return f->isElapsed(); }),
                    m_functions.end());

  std::vector<std::unique_ptr<Callback>> callbacks;
  std::move(m_callbacks.begin(), m_callbacks.end(), std::back_inserter(callbacks));
  m_callbacks.clear();
  for (auto &callback : callbacks) {
    (*callback)(elapsed);
  }
  callbacks.erase(std::remove_if(callbacks.begin(),
                                 callbacks.end(),
                                 [](auto &f) { return f->isElapsed(); }),
                  callbacks.end());
  std::move(callbacks.begin(), callbacks.end(), std::back_inserter(m_callbacks));
}

void Engine::Impl::updateActorIcons(const ngf::TimeSpan &elapsed) {
  auto screenSize = m_pRoom->getScreenSize();
  auto screenMouse = toDefaultView((glm::ivec2) m_mousePos, screenSize);
  m_actorIcons.setMousePosition(screenMouse);
  m_actorIcons.update(elapsed);
}

void Engine::Impl::updateMouseCursor() {
  auto flags = getFlags(m_objId1);
  auto screen = m_pApp->getRenderTarget()->getView().getSize();
  m_cursorDirection = CursorDirection::None;
  if ((m_mousePos.x < 20) || (flags & ObjectFlagConstants::DOOR_LEFT) == ObjectFlagConstants::DOOR_LEFT)
    m_cursorDirection |= CursorDirection::Left;
  else if ((m_mousePos.x > screen.x - 20) ||
      (flags & ObjectFlagConstants::DOOR_RIGHT) == ObjectFlagConstants::DOOR_RIGHT)
    m_cursorDirection |= CursorDirection::Right;
  if ((flags & ObjectFlagConstants::DOOR_FRONT) == ObjectFlagConstants::DOOR_FRONT)
    m_cursorDirection |= CursorDirection::Down;
  else if ((flags & ObjectFlagConstants::DOOR_BACK) == ObjectFlagConstants::DOOR_BACK)
    m_cursorDirection |= CursorDirection::Up;
  if ((m_cursorDirection == CursorDirection::None) && m_objId1)
    m_cursorDirection |= CursorDirection::Hotspot;
}

Entity *Engine::Impl::getHoveredEntity(const glm::vec2 &mousPos) {
  Entity *pCurrentObject = nullptr;

  // mouse on actor ?
  for (auto &&actor : m_actors) {
    if (actor.get() == m_pCurrentActor)
      continue;
    if (actor->getRoom() != m_pRoom)
      continue;

    if (actor->contains(mousPos)) {
      if (!pCurrentObject || actor->getZOrder() < pCurrentObject->getZOrder()) {
        pCurrentObject = actor.get();
      }
    }
  }

  // mouse on object ?
  const auto &objects = m_pRoom->getObjects();
  std::for_each(objects.cbegin(), objects.cend(), [mousPos, &pCurrentObject](const auto &pObj) {
    if (!pObj->isTouchable())
      return;
    auto rect = pObj->getRealHotspot();
    if (!rect.contains((glm::ivec2) mousPos))
      return;
    if (!pCurrentObject || pObj->getZOrder() <= pCurrentObject->getZOrder())
      pCurrentObject = pObj.get();
  });

  if (!pCurrentObject && m_pRoom && m_pRoom->getFullscreen() != 1) {
    // mouse on inventory object ?
    pCurrentObject = m_hud.getInventory().getCurrentInventoryObject();
  }

  return pCurrentObject;
}

void Engine::Impl::updateHoveredEntity(bool isRightClick) {
  m_hud.setVerbOverride(nullptr);
  if (!m_hud.getCurrentVerb()) {
    m_hud.setCurrentVerb(m_hud.getVerb(VerbConstants::VERB_WALKTO));
  }

  if (m_pUseObject) {
    m_objId1 = m_pUseObject ? m_pUseObject->getId() : 0;
    m_pObj2 = m_hud.getHoveredEntity();
  } else {
    m_objId1 = m_hud.getHoveredEntity() ? m_hud.getHoveredEntity()->getId() : 0;
    m_pObj2 = nullptr;
  }

  // abort some invalid actions
  if (!m_objId1 || !m_hud.getCurrentVerb()) {
    return;
  }

  if (m_pObj2 && m_pObj2->getId() == m_objId1) {
    m_pObj2 = nullptr;
  }

  if (m_objId1 && isRightClick) {
    m_hud.setVerbOverride(m_hud.getVerb(EntityManager::getScriptObjectFromId<Entity>(m_objId1)->getDefaultVerb(
        VerbConstants::VERB_LOOKAT)));
  }

  auto verbId = m_hud.getCurrentVerb()->id;
  switch (verbId) {
  case VerbConstants::VERB_WALKTO: {
    auto pObj1 = EntityManager::getScriptObjectFromId<Entity>(m_objId1);
    if (pObj1 && pObj1->isInventoryObject()) {
      m_hud.setVerbOverride(m_hud.getVerb(EntityManager::getScriptObjectFromId<Entity>(m_objId1)->getDefaultVerb(
          VerbConstants::VERB_LOOKAT)));
    }
    break;
  }
  case VerbConstants::VERB_TALKTO:
    // select actor/object only if talkable flag is set
    if (!hasFlag(m_objId1, ObjectFlagConstants::TALKABLE)) {
      m_objId1 = 0;
    }
    break;
  case VerbConstants::VERB_GIVE: {
    auto pObj1 = EntityManager::getScriptObjectFromId<Entity>(m_objId1);
    if (!pObj1->isInventoryObject())
      m_objId1 = 0;

    // select actor/object only if giveable flag is set
    if (m_pObj2 && !hasFlag(m_pObj2->getId(), ObjectFlagConstants::GIVEABLE))
      m_pObj2 = nullptr;
    break;
  }
  default: {
    auto pActor = EntityManager::getScriptObjectFromId<Actor>(m_objId1);
    if (pActor) {
      m_objId1 = 0;
    }
    break;
  }
  }
}

Entity *Engine::Impl::getEntity(Entity *pEntity) const {
  if (!pEntity)
    return nullptr;

  // if an actor has the same name then get its flags
  auto itActor = std::find_if(m_actors.begin(), m_actors.end(), [pEntity](const auto &pActor) -> bool {
    return pActor->getName() == pEntity->getName();
  });
  if (itActor != m_actors.end()) {
    return itActor->get();
  }
  return pEntity;
}

bool Engine::Impl::hasFlag(int id, uint32_t flagToTest) const {
  auto pObj = EntityManager::getScriptObjectFromId<Entity>(id);
  auto flags = getFlags(pObj);
  if (flags & flagToTest)
    return true;
  auto pActor = getEntity(pObj);
  flags = getFlags(pActor);
  return flags & flagToTest;
}

uint32_t Engine::Impl::getFlags(int id) const {
  auto pEntity = EntityManager::getScriptObjectFromId<Entity>(id);
  return getFlags(pEntity);
}

uint32_t Engine::Impl::getFlags(Entity *pEntity) const {
  if (pEntity)
    return pEntity->getFlags();
  return 0;
}

void Engine::Impl::updateRoomScalings() const {
  auto actor = m_pCurrentActor;
  if (!actor)
    return;

  auto &scalings = m_pRoom->getScalings();
  auto &objects = m_pRoom->getObjects();
  for (auto &&object : objects) {
    if (object->getType() != ObjectType::Trigger)
      continue;
    if (object->getRealHotspot().contains((glm::ivec2) actor->getPosition())) {
      auto it = std::find_if(scalings.begin(), scalings.end(), [&object](const auto &s) -> bool {
        return s.getName() == object->getName();
      });
      if (it != scalings.end()) {
        m_pRoom->setRoomScaling(*it);
        return;
      }
    }
  }
  if (!scalings.empty()) {
    m_pRoom->setRoomScaling(scalings[0]);
  }
}

const Verb *Engine::Impl::getHoveredVerb() const {
  if (!m_hud.getActive())
    return nullptr;
  if (m_pRoom && m_pRoom->getFullscreen() == 1)
    return nullptr;

  return m_hud.getHoveredVerb();
}

void Engine::Impl::stopTalking() const {
  for (auto &&a : m_pEngine->getActors()) {
    a->stopTalking();
  }
  for (auto &&a : m_pEngine->getRoom()->getObjects()) {
    a->stopTalking();
  }
}

void Engine::Impl::stopTalkingExcept(Entity *pEntity) const {
  for (auto &&a : m_pEngine->getActors()) {
    if (a.get() == pEntity)
      continue;
    a->stopTalking();
  }

  for (auto &&a : m_pEngine->getRoom()->getObjects()) {
    if (a.get() == pEntity)
      continue;
    a->stopTalking();
  }
}

void Engine::Impl::updateKeys() {
  ImGuiIO &io = ImGui::GetIO();
  if (io.WantTextInput)
    return;

  const auto &cmdMgr = Locator<CommandManager>::get();
  for (auto &key : m_oldKeyDowns) {
    if (isKeyPressed(key)) {
      cmdMgr.execute(key);
      cmdMgr.execute(key, false);
    }
  }

  for (auto &key : m_newKeyDowns) {
    if (m_oldKeyDowns.find(key) != m_oldKeyDowns.end()) {
      cmdMgr.execute(key, true);
    }
  }

  m_oldKeyDowns.clear();
  for (auto key : m_newKeyDowns) {
    m_oldKeyDowns.insert(key);
  }
}

bool Engine::Impl::isKeyPressed(const Input &key) {
  auto wasDown = m_oldKeyDowns.find(key) != m_oldKeyDowns.end();
  auto isDown = m_newKeyDowns.find(key) != m_newKeyDowns.end();
  return wasDown && !isDown;
}

InputConstants Engine::Impl::toKey(const std::string &keyText) {
  if (keyText.length() == 1) {
    return static_cast<InputConstants>(keyText[0]);
  }
  return InputConstants::NONE;
}

void Engine::Impl::updateKeyboard() {
  if (m_oldKeyDowns.empty())
    return;

  if (m_pRoom) {
    for (auto key : m_oldKeyDowns) {
      if (isKeyPressed(key) && ScriptEngine::rawExists(m_pRoom, "pressedKey")) {
        ScriptEngine::rawCall(m_pRoom, "pressedKey", static_cast<int>(key.input));
      }
    }
  }

  int currentActorIndex = getCurrentActorIndex();
  if (currentActorIndex == -1)
    return;

  const auto &verbSlot = m_hud.getVerbSlot(currentActorIndex);
  for (auto i = 0; i < 10; i++) {
    const auto &verb = verbSlot.getVerb(i);
    if (verb.key.length() == 0)
      continue;
    auto id = std::strtol(verb.key.substr(1, verb.key.length() - 1).c_str(), nullptr, 10);
    auto key = toKey(tostring(ng::Engine::getText(id)));
    if (isKeyPressed(key)) {
      onVerbClick(&verb);
    }
  }
}

void Engine::Impl::onVerbClick(const Verb *pVerb) {
  m_hud.setCurrentVerb(pVerb);
  m_useFlag = UseFlag::None;
  m_pUseObject = nullptr;
  m_objId1 = 0;
  m_pObj2 = nullptr;

  ScriptEngine::rawCall("onVerbClick");
}

bool Engine::Impl::clickedAt(const glm::vec2 &pos) const {
  if (!m_pRoom)
    return false;

  bool handled = false;
  if (ScriptEngine::rawExists(m_pRoom, clickedAtCallback)) {
    ScriptEngine::rawCallFunc(handled, m_pRoom, clickedAtCallback, pos.x, pos.y);
    if (handled)
      return true;
  }

  if (!m_pCurrentActor)
    return false;

  if (!ScriptEngine::rawExists(m_pCurrentActor, clickedAtCallback))
    return false;

  ScriptEngine::rawCallFunc(handled, m_pCurrentActor, clickedAtCallback, pos.x, pos.y);
  return handled;
}

void Engine::Impl::drawActorHotspot(ngf::RenderTarget &target) const {
  if (!m_pCurrentActor)
    return;

  if (!m_pCurrentActor->isHotspotVisible())
    return;

  auto at = m_camera.getRect().getTopLeft();
  auto rect = m_pCurrentActor->getHotspot();
  auto pos = m_pCurrentActor->getPosition();
  pos = {pos.x - at.x, at.y + m_pRoom->getScreenSize().y - pos.y};

  ngf::Transform t;
  t.setPosition(pos);

  ngf::RenderStates s;
  s.transform = t.getTransform();

  ngf::RectangleShape shape(rect.getSize());
  shape.getTransform().setPosition(rect.getTopLeft());
  shape.setOutlineThickness(1);
  shape.setOutlineColor(ngf::Colors::Red);
  shape.setColor(ngf::Colors::Transparent);
  shape.draw(target, s);

  // draw actor position
  ngf::RectangleShape rectangle;
  rectangle.setColor(ngf::Colors::Red);
  rectangle.setSize(glm::vec2(2, 2));
  rectangle.getTransform().setOrigin(glm::vec2(1, 1));
  rectangle.draw(target, s);
}

glm::vec2 Engine::Impl::roomToScreen(const glm::vec2 &pos) const {
  auto at = m_camera.getRect().getTopLeft();
  return toDefaultView((glm::ivec2) (pos - at), m_pRoom->getScreenSize());
}

ngf::irect Engine::Impl::roomToScreen(const ngf::irect &rect) const {
  auto min = toDefaultView((glm::ivec2) rect.getSize(), m_pRoom->getScreenSize());
  auto max = toDefaultView((glm::ivec2) rect.max, m_pRoom->getScreenSize());
  return ngf::irect::fromMinMax(min, max);
}

void Engine::Impl::drawObjectHotspot(const Object &object, ngf::RenderTarget &target) const {
  if (!object.isTouchable())
    return;

  const auto showHotspot =
      Locator<Preferences>::get().getTempPreference(TempPreferenceNames::ShowHotspot,
                                                    TempPreferenceDefaultValues::ShowHotspot);
  if (!showHotspot)
    return;

  auto pos = object.getPosition();
  pos = roomToScreen(pos);
  pos = {pos.x, Screen::Height - pos.y};

  ngf::Transform t;
  t.setPosition(pos);

  ngf::RenderStates s;
  s.transform = t.getTransform();

  const auto &view = target.getView();
  target.setView(ngf::View{ngf::frect::fromPositionSize({0, 0}, {Screen::Width, Screen::Height})});

  auto &gameSheet = Locator<ResourceManager>::get().getSpriteSheet("GameSheet");
  ngf::Sprite sprite(*gameSheet.getTexture(), gameSheet.getRect("hotspot_marker"));
  sprite.setColor(ngf::Color(255, 165, 0));
  sprite.getTransform().setOrigin({15.f, 15.f});
  sprite.draw(target, s);

  target.setView(view);
}

class ArrowShape final : public ngf::Drawable {
public:
  ArrowShape(UseDirection dir, const ngf::Color &color) : m_dir(dir), m_color(color) {}

  void draw(ngf::RenderTarget &target, ngf::RenderStates s) const final {
    const auto headPos = size / 3.f;
    switch (m_dir) {
    case UseDirection::Front: {
      ngf::RectangleShape dirShape({size, 1});
      dirShape.getTransform().setPosition({-headPos, size - headPos});
      dirShape.setColor(m_color);
      dirShape.draw(target, s);
    }
      break;
    case UseDirection::Back: {
      ngf::RectangleShape dirShape(glm::vec2(size, 1));
      dirShape.getTransform().setPosition({-headPos, headPos - size});
      dirShape.setColor(m_color);
      dirShape.draw(target, s);
    }
      break;
    case UseDirection::Left: {
      ngf::RectangleShape dirShape(glm::vec2(1, size));
      dirShape.getTransform().setPosition({headPos - size, -headPos});
      dirShape.setColor(m_color);
      dirShape.draw(target, s);
    }
      break;
    case UseDirection::Right: {
      ngf::RectangleShape dirShape(glm::vec2(1, size));
      dirShape.getTransform().setPosition({size - headPos, -headPos});
      dirShape.setColor(m_color);
      dirShape.draw(target, s);
    }
      break;
    }
  }

private:
  const float size = 3.f;
  UseDirection m_dir;
  ngf::Color m_color;
};

class CrossShape final : public ngf::Drawable {
public:
  explicit CrossShape(const ngf::Color &color) : m_color(color) {}

  void draw(ngf::RenderTarget &target, ngf::RenderStates s) const final {
    ngf::RectangleShape vl(glm::vec2(1, 7));
    vl.getTransform().setPosition({0, -3});
    vl.setColor(m_color);
    vl.draw(target, s);

    ngf::RectangleShape hl(glm::vec2(7, 1));
    hl.getTransform().setPosition({-3, 0});
    hl.setColor(m_color);
    hl.draw(target, s);
  }

private:
  ngf::Color m_color;
};

void Engine::Impl::drawDebugHotspot(const Object &object, ngf::RenderTarget &target) const {
  if (!object.isHotspotVisible())
    return;

  auto pos = object.getPosition();
  auto at = m_camera.getRect().getTopLeft();
  pos = {pos.x - at.x, m_pRoom->getScreenSize().y - pos.y + at.y};

  ngf::Transform t;
  t.setPosition(pos);
  ngf::RenderStates s;
  s.transform = t.getTransform();

  // fix hotspot
  auto rect = object.getHotspot();
  auto min = rect.min;
  auto max = rect.max;
  min.y = -min.y;
  max.y = -max.y;
  rect = ngf::irect::fromMinMax(min, max);

  auto color = toColor(object.getType());
  std::array<ngf::Vertex, 4> hotspotVertices = {
      ngf::Vertex{rect.getTopLeft(), color},
      ngf::Vertex{rect.getBottomLeft(), color},
      ngf::Vertex{rect.getBottomRight(), color},
      ngf::Vertex{rect.getTopRight(), color}};

  // draw a rectangle
  target.draw(ngf::PrimitiveType::LineLoop, hotspotVertices, s);

  // draw a cross at the use position
  auto usePos = object.getUsePosition().value_or(glm::vec2());
  usePos.y = -usePos.y;
  t.setPosition(usePos);
  s.transform = t.getTransform() * s.transform;
  CrossShape cross(color);
  cross.draw(target, s);

  // draw direction
  const auto useDir = object.getUseDirection().value_or(UseDirection::Front);
  ArrowShape arrow(useDir, color);
  arrow.draw(target, s);
}

void Engine::Impl::drawScreenSpace(const Object &object, ngf::RenderTarget &target, ngf::RenderStates states) {
  if (object.getScreenSpace() != ScreenSpace::Object)
    return;

  const auto *anim = object.getAnimation();
  if (!anim)
    return;

  const auto view = target.getView();
  target.setView(ngf::View(ngf::frect::fromPositionSize({0, 0}, {Screen::Width, Screen::Height})));

  auto t = object.getTransform();
  auto pos = t.getPosition();
  ngf::RenderStates s;

  t.setPosition({pos.x, Screen::Height - pos.y});
  s.transform = t.getTransform();
  s.shader = states.shader;

  AnimDrawable animDrawable;
  animDrawable.setAnim(anim);
  animDrawable.setColor(object.getColor());
  animDrawable.draw(pos, target, s);

  target.setView(view);
}

void Engine::Impl::drawWalkboxes(ngf::RenderTarget &target) const {
  if (!m_pRoom || m_showDrawWalkboxes == WalkboxesFlags::None)
    return;

  auto at = m_camera.getRect().getTopLeft();
  ngf::Transform t;
  t.setPosition({-at.x, at.y});
  ngf::RenderStates states;
  states.transform = t.getTransform();

  if ((m_showDrawWalkboxes & WalkboxesFlags::Walkboxes) == WalkboxesFlags::Walkboxes) {
    // draw walkboxes
    for (const auto &walkbox : m_pRoom->getWalkboxes()) {
      WalkboxDrawable wd(walkbox, m_pRoom->getScreenSize().y);
      wd.draw(target, states);
    }
  }

  if ((m_showDrawWalkboxes & WalkboxesFlags::Merged) == WalkboxesFlags::Merged) {
    // draw merged walkboxes
    for (const auto &walkbox : m_pRoom->getGraphWalkboxes()) {
      WalkboxDrawable wd(walkbox, m_pRoom->getScreenSize().y);
      wd.draw(target, states);
    }
  }

  if ((m_showDrawWalkboxes & WalkboxesFlags::Graph) == WalkboxesFlags::Graph) {
    // draw walkbox graph
    const auto *pGraph = m_pRoom->getGraph();
    if (pGraph) {
      auto height = m_pRoom->getRoomSize().y;
      ng::GraphDrawable d(*pGraph, height);
      d.draw(target, states);
    }
  }

  if (!m_pCurrentActor)
    return;
  auto path = m_pCurrentActor->getPath();
  if (!path)
    return;
  path->draw(target, states);
}

void Engine::Impl::drawPause(ngf::RenderTarget &target) const {
  if (m_state != EngineState::Paused)
    return;

  const auto view = target.getView();
  auto viewRect = ngf::frect::fromPositionSize({0, 0}, {320, 176});
  target.setView(ngf::View(viewRect));

  auto &saveLoadSheet = Locator<ResourceManager>::get().getSpriteSheet("SaveLoadSheet");
  auto viewCenter = glm::vec2(viewRect.getWidth() / 2, viewRect.getHeight() / 2);
  auto rect = saveLoadSheet.getRect("pause_dialog");

  ngf::Sprite sprite;
  sprite.getTransform().setPosition(viewCenter);
  sprite.setTexture(*saveLoadSheet.getTexture());
  sprite.getTransform().setOrigin({rect.getWidth() / 2.f, rect.getHeight() / 2.f});
  sprite.setTextureRect(rect);
  sprite.draw(target, {});

  viewRect = ngf::frect::fromPositionSize({0, 0}, {Screen::Width, Screen::Height});
  viewCenter = glm::vec2(viewRect.getWidth() / 2, viewRect.getHeight() / 2);
  target.setView(ngf::View(viewRect));

  auto retroFonts =
      m_pEngine->getPreferences().getUserPreference(PreferenceNames::RetroFonts, PreferenceDefaultValues::RetroFonts);
  auto &font = m_pEngine->getResourceManager().getFont(retroFonts ? "FontRetroSheet" : "FontModernSheet");

  ng::Text text;
  auto screen = target.getView().getSize();
  auto scale = screen.y / 512.f;
  text.getTransform().setScale({scale, scale});
  text.getTransform().setPosition(viewCenter);
  text.setFont(font);
  text.setColor(ngf::Colors::White);
  text.setWideString(Engine::getText(99951));
  auto bounds = getGlobalBounds(text);
  text.getTransform().move({-bounds.getWidth() / 2.f, -scale * bounds.getHeight() / 2.f});
  text.draw(target, {});

  target.setView(view);
}

void Engine::Impl::stopThreads() {
  m_threads.erase(std::remove_if(m_threads.begin(), m_threads.end(), [](const auto &t) -> bool {
    return !t || t->isStopped();
  }), m_threads.end());
}

void Engine::Impl::drawCursor(ngf::RenderTarget &target) const {
  if (!m_cursorVisible)
    return;
  if (!m_showCursor && m_dialogManager.getState() != DialogManagerState::WaitingForChoice)
    return;

  auto cursorSize = glm::vec2(68.f, 68.f);
  const auto &gameSheet = Locator<ResourceManager>::get().getSpriteSheet("GameSheet");

  const auto view = target.getView();
  target.setView(ngf::View(ngf::frect::fromPositionSize({0, 0}, {Screen::Width, Screen::Height})));

  auto screenSize = m_pRoom->getScreenSize();
  auto pos = toDefaultView((glm::ivec2) m_mousePos, screenSize);

  ngf::RectangleShape shape;
  shape.getTransform().setPosition(pos);
  shape.getTransform().setOrigin(cursorSize / 2.f);
  shape.setSize(cursorSize);
  shape.setTexture(*gameSheet.getTexture(), false);
  shape.setTextureRect(getCursorRect());
  shape.draw(target, {});

  target.setView(view);
}

ngf::irect Engine::Impl::getCursorRect() const {
  auto &gameSheet = Locator<ResourceManager>::get().getSpriteSheet("GameSheet");
  if (m_state == EngineState::Paused)
    return gameSheet.getRect("cursor_pause");

  if (m_state == EngineState::Options)
    return gameSheet.getRect("cursor");

  if (m_dialogManager.getState() != DialogManagerState::None)
    return gameSheet.getRect("cursor");

  if (m_pRoom->getFullscreen() == 1)
    return gameSheet.getRect("cursor");

  if (m_cursorDirection & CursorDirection::Left) {
    return m_cursorDirection & CursorDirection::Hotspot ? gameSheet.getRect("hotspot_cursor_left")
                                                        : gameSheet.getRect("cursor_left");
  }
  if (m_cursorDirection & CursorDirection::Right) {
    return m_cursorDirection & CursorDirection::Hotspot ? gameSheet.getRect("hotspot_cursor_right")
                                                        : gameSheet.getRect("cursor_right");
  }
  if (m_cursorDirection & CursorDirection::Up) {
    return m_cursorDirection & CursorDirection::Hotspot ? gameSheet.getRect("hotspot_cursor_back")
                                                        : gameSheet.getRect("cursor_back");
  }
  if (m_cursorDirection & CursorDirection::Down) {
    return (m_cursorDirection & CursorDirection::Hotspot) ? gameSheet.getRect("hotspot_cursor_front")
                                                          : gameSheet.getRect("cursor_front");
  }
  return (m_cursorDirection & CursorDirection::Hotspot) ? gameSheet.getRect("hotspot_cursor")
                                                        : gameSheet.getRect("cursor");
}

std::wstring Engine::Impl::getDisplayName(const std::wstring &name) {
  std::wstring displayName(name);
  auto len = displayName.length();
  if (len > 1 && displayName[0] == '^') {
    displayName = name.substr(1, len - 1);
  }
  if (len > 2 && displayName[len - 2] == '#') {
    displayName = name.substr(0, len - 2);
  }
  return displayName;
}

const Verb *Engine::Impl::overrideVerb(const Verb *pVerb) const {
  if (!pVerb || pVerb->id != VerbConstants::VERB_WALKTO)
    return pVerb;

  auto pObj1 = EntityManager::getScriptObjectFromId<Entity>(m_objId1);
  if (!pObj1)
    return pVerb;
  return m_hud.getVerb(pObj1->getDefaultVerb(VerbConstants::VERB_WALKTO));
}

void Engine::Impl::drawCursorText(ngf::RenderTarget &target) const {
  if (!m_cursorVisible)
    return;

  if (!m_showCursor || m_state != EngineState::Game)
    return;

  if (m_dialogManager.getState() != DialogManagerState::None)
    return;

  auto pVerb = m_hud.getVerbOverride();
  if (!pVerb)
    pVerb = m_hud.getCurrentVerb();
  if (!pVerb)
    return;

  pVerb = overrideVerb(pVerb);

  auto currentActorIndex = getCurrentActorIndex();
  if (currentActorIndex == -1)
    return;

  auto textColor = m_hud.getVerbUiColors(currentActorIndex).sentence;
  auto classicSentence = m_pEngine->getPreferences().getUserPreference(PreferenceNames::ClassicSentence,
                                                                       PreferenceDefaultValues::ClassicSentence);

  const auto view = target.getView();
  target.setView(ngf::View(ngf::frect::fromPositionSize({0, 0}, {Screen::Width, Screen::Height})));

  auto retroFonts =
      m_pEngine->getPreferences().getUserPreference(PreferenceNames::RetroFonts, PreferenceDefaultValues::RetroFonts);
  auto &font = m_pEngine->getResourceManager().getFont(retroFonts ? "FontRetroSheet" : "FontModernSheet");

  std::wstring s;
  // draw verb
  if ((m_pRoom->getFullscreen() != 1) && (pVerb->id != VerbConstants::VERB_WALKTO || m_hud.getHoveredEntity())) {
    auto id = std::strtol(pVerb->text.substr(1).data(), nullptr, 10);
    s.append(ng::Engine::getText(id));
  }
  auto pObj1 = EntityManager::getScriptObjectFromId<Entity>(m_objId1);
  // draw object 1 name
  if (pObj1) {
    if (!s.empty()) {
      s.append(L" ");
    }
    s.append(getDisplayName(ng::Engine::getText(pObj1->getName())));
    if (DebugFeatures::showHoveredObject) {
      if (pObj1) {
        s.append(L"(").append(towstring(pObj1->getKey())).append(L")");
      }
    }
  }
  // draw use flags if any
  appendUseFlag(s);
  // draw object 2 name
  if (m_pObj2) {
    s.append(L" ").append(getDisplayName(ng::Engine::getText(m_pObj2->getName())));
  }

  ng::Text text;
  text.setFont(font);
  text.setColor(textColor);
  text.setWideString(s);

  // do display cursor position:
  if (DebugFeatures::showCursorPosition) {
    std::wstringstream ss;
    std::wstring txt = text.getWideString();
    ss << txt << L" (" << std::fixed << std::setprecision(0) << m_mousePosInRoom.x << L"," << m_mousePosInRoom.y
       << L")";
    text.setWideString(ss.str());
  }

  // gets the position where to draw the cursor text
  auto bounds = getGlobalBounds(text);
  if (classicSentence) {
    auto y = Screen::Height - 210.f;
    auto x = Screen::HalfWidth - bounds.getWidth() / 2.f;
    text.getTransform().setPosition({x, y});
  } else {
    auto screenSize = m_pRoom->getScreenSize();
    auto pos = toDefaultView((glm::ivec2) m_mousePos, screenSize);
    auto y = pos.y - 20 < 40 ? pos.y + 80 : pos.y - 40;
    auto x = std::clamp<float>(pos.x - bounds.getWidth() / 2.f, 20.f, Screen::Width - 20.f - bounds.getWidth());
    text.getTransform().setPosition({x, y - bounds.getHeight()});
  }

  text.draw(target, {});
  target.setView(view);
}

void Engine::Impl::drawNoOverride(ngf::RenderTarget &target) const {
  if (m_noOverrideElapsed > ngf::TimeSpan::seconds(2))
    return;

  auto &gameSheet = Locator<ResourceManager>::get().getSpriteSheet("GameSheet");
  const auto view = target.getView();
  target.setView(ngf::View(ngf::frect::fromPositionSize({0, 0}, {Screen::Width, Screen::Height})));

  ngf::Color c(ngf::Colors::White);
  c.a = (2.f - m_noOverrideElapsed.getTotalSeconds() / 2.f);
  ngf::Sprite spriteNo;
  spriteNo.setColor(c);
  spriteNo.getTransform().setPosition({8.f, 8.f});
  spriteNo.getTransform().setScale({2.f, 2.f});
  spriteNo.setTexture(*gameSheet.getTexture());
  spriteNo.setTextureRect(gameSheet.getRect("icon_no"));
  spriteNo.draw(target, {});

  target.setView(view);
}

void Engine::Impl::appendUseFlag(std::wstring &sentence) const {
  switch (m_useFlag) {
  case UseFlag::UseWith:sentence.append(L" ").append(ng::Engine::getText(10000));
    break;
  case UseFlag::UseOn:sentence.append(L" ").append(ng::Engine::getText(10001));
    break;
  case UseFlag::UseIn:sentence.append(L" ").append(ng::Engine::getText(10002));
    break;
  case UseFlag::GiveTo:sentence.append(L" ").append(ng::Engine::getText(10003));
    break;
  case UseFlag::None:break;
  }
}

int Engine::Impl::getCurrentActorIndex() const {
  for (int i = 0; i < static_cast<int>(m_actorsIconSlots.size()); i++) {
    const auto &selectableActor = m_actorsIconSlots.at(i);
    if (selectableActor.pActor == m_pCurrentActor) {
      return i;
    }
  }
  return -1;
}

void Engine::Impl::drawHud(ngf::RenderTarget &target) const {
  if (m_state != EngineState::Game)
    return;

  m_hud.draw(target, {});
}

void Engine::Impl::captureScreen(const std::string &path) const {
  ngf::RenderTexture target({320, 180});
  m_pEngine->draw(target, true);
  target.display();

  auto screenshot = target.capture();
  screenshot.saveToFile(path);
}
}
