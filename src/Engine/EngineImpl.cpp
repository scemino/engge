#include "EngineImpl.hpp"

namespace ng {
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
    : _textureManager(Locator<ResourceManager>::get()),
      _preferences(Locator<Preferences>::get()),
      _soundManager(Locator<SoundManager>::get()),
      _actorIcons(_actorsIconSlots, _hud, _pCurrentActor) {
  _hud.setTextureManager(&_textureManager);
  sq_resetobject(&_pDefaultObject);

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
          {EngineCommands::SelectChoice1, [this] { _dialogManager.choose(1); }},
          {EngineCommands::SelectChoice2, [this] { _dialogManager.choose(2); }},
          {EngineCommands::SelectChoice3, [this] { _dialogManager.choose(3); }},
          {EngineCommands::SelectChoice4, [this] { _dialogManager.choose(4); }},
          {EngineCommands::SelectChoice5, [this] { _dialogManager.choose(5); }},
          {EngineCommands::SelectChoice6, [this] { _dialogManager.choose(6); }},
          {EngineCommands::ShowOptions, [this] { _pEngine->showOptions(true); }},
          {EngineCommands::ToggleHud, [this] {
            _hud.setVisible(!_cursorVisible);
            _actorIcons.setVisible(!_cursorVisible);
            _cursorVisible = !_cursorVisible;
          }}
      });
  Locator<CommandManager>::get().registerPressedCommand(EngineCommands::ShowHotspots, [this](bool down) {
    _preferences.setTempPreference(TempPreferenceNames::ShowHotspot, down);
  });

  _fadeShader.load(Shaders::vertexShader, Shaders::fadeFragmentShader);
  uint32_t pixels[4]{0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF};
  _blackTexture.loadFromMemory({2, 2}, pixels);
}

void Engine::Impl::pauseGame() {
  _state = _state == EngineState::Game ? EngineState::Paused : EngineState::Game;
  if (_state == EngineState::Paused) {
    // pause all pauseable threads
    for (auto &thread : _threads) {
      if (thread->isPauseable() && !thread->isStopped()) {
        thread->pause();
      }
    }
    _soundManager.pauseAllSounds();
  } else {
    // resume all pauseable threads
    for (auto &thread : _threads) {
      if (thread->isPauseable() && !thread->isStopped()) {
        thread->resume();
      }
    }
    _soundManager.resumeAllSounds();
  }
}

void Engine::Impl::selectActor(int index) {
  if (index <= 0 || index > static_cast<int>(_actorsIconSlots.size()))
    return;
  const auto &slot = _actorsIconSlots[index - 1];
  if (!slot.selectable)
    return;
  _pEngine->setCurrentActor(slot.pActor, true);
}

void Engine::Impl::selectPreviousActor() {
  auto currentActorIndex = getCurrentActorIndex();
  if (currentActorIndex == -1)
    return;
  auto size = static_cast<int>(_actorsIconSlots.size());
  for (auto i = 0; i < size; i++) {
    auto index = currentActorIndex - i - 1;
    if (index < 0)
      index += size - 1;
    if (index == currentActorIndex)
      return;
    const auto &slot = _actorsIconSlots[index];
    if (slot.selectable) {
      _pEngine->setCurrentActor(slot.pActor, true);
      return;
    }
  }
}

void Engine::Impl::selectNextActor() {
  auto currentActorIndex = getCurrentActorIndex();
  if (currentActorIndex == -1)
    return;
  auto size = static_cast<int>(_actorsIconSlots.size());
  for (auto i = 0; i < size; i++) {
    auto index = (currentActorIndex + i + 1) % size;
    if (index == currentActorIndex)
      return;
    const auto &slot = _actorsIconSlots[index];
    if (slot.selectable) {
      _pEngine->setCurrentActor(slot.pActor, true);
      return;
    }
  }
}

void Engine::Impl::skipCutscene() {
  if (_pEngine->inCutscene()) {
    if (_pCutscene && _pCutscene->hasCutsceneOverride()) {
      _pEngine->cutsceneOverride();
    } else {
      _noOverrideElapsed = ngf::TimeSpan::seconds(0);
    }
  }
}

void Engine::Impl::skipText() const {
  if (_dialogManager.getState() == DialogManagerState::Active) {
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
  _pEngine->setDefaultVerb();
  _talkingState.stop();

  if (!_pRoom)
    return 0;

  auto pOldRoom = _pRoom;

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
  std::for_each(_threads.begin(), _threads.end(), [](auto &pThread) {
    if (!pThread->isGlobal())
      pThread->stop();
  });

  return 0;
}

void Engine::Impl::actorEnter() const {
  if (!_pCurrentActor)
    return;

  _pCurrentActor->stopWalking();
  ScriptEngine::rawCall("actorEnter", _pCurrentActor);

  if (!_pRoom)
    return;

  if (ScriptEngine::rawExists(_pRoom, "actorEnter")) {
    ScriptEngine::rawCall(_pRoom, "actorEnter", _pCurrentActor);
  }
}

void Engine::Impl::actorExit() const {
  if (!_pCurrentActor || !_pRoom)
    return;

  if (ScriptEngine::rawExists(_pRoom, "actorExit")) {
    ScriptEngine::rawCall(_pRoom, "actorExit", _pCurrentActor);
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
  if (_run != state) {
    _run = state;
    if (_pCurrentActor) {
      ScriptEngine::objCall(_pCurrentActor, "run", state);
    }
  }
}

void Engine::Impl::setCurrentRoom(Room *pRoom) {
  // reset fade effect if we change the room except for wobble effect
  if (_fadeEffect.effect != FadeEffect::Wobble) {
    _fadeEffect.effect = FadeEffect::None;
  }

  if (pRoom) {
    ScriptEngine::set("currentRoom", pRoom);
  }
  _camera.resetBounds();
  _pRoom = pRoom;
  _camera.at(glm::vec2(0, 0));
}

void Engine::Impl::updateCutscene(const ngf::TimeSpan &elapsed) {
  if (_pCutscene) {
    (*_pCutscene)(elapsed);
    if (_pCutscene->isElapsed()) {
      _pCutscene = nullptr;
    }
  }
}

void Engine::Impl::updateSentence(const ngf::TimeSpan &elapsed) const {
  if (!_pSentence)
    return;
  (*_pSentence)(elapsed);
  if (!_pSentence->isElapsed())
    return;
  _pEngine->stopSentence();
}

void Engine::Impl::updateFunctions(const ngf::TimeSpan &elapsed) {
  for (auto &function : _newFunctions) {
    _functions.push_back(std::move(function));
  }
  _newFunctions.clear();
  for (auto &function : _functions) {
    (*function)(elapsed);
  }
  _functions.erase(std::remove_if(_functions.begin(), _functions.end(),
                                  [](std::unique_ptr<Function> &f) { return f->isElapsed(); }),
                   _functions.end());

  std::vector<std::unique_ptr<Callback>> callbacks;
  std::move(_callbacks.begin(), _callbacks.end(), std::back_inserter(callbacks));
  _callbacks.clear();
  for (auto &callback : callbacks) {
    (*callback)(elapsed);
  }
  callbacks.erase(std::remove_if(callbacks.begin(),
                                 callbacks.end(),
                                 [](auto &f) { return f->isElapsed(); }),
                  callbacks.end());
  std::move(callbacks.begin(), callbacks.end(), std::back_inserter(_callbacks));
}

void Engine::Impl::updateActorIcons(const ngf::TimeSpan &elapsed) {
  auto screenSize = _pRoom->getScreenSize();
  auto screenMouse = toDefaultView((glm::ivec2) _mousePos, screenSize);
  _actorIcons.setMousePosition(screenMouse);
  _actorIcons.update(elapsed);
}

void Engine::Impl::updateMouseCursor() {
  auto flags = getFlags(_objId1);
  auto screen = _pApp->getRenderTarget()->getView().getSize();
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
  if ((_cursorDirection == CursorDirection::None) && _objId1)
    _cursorDirection |= CursorDirection::Hotspot;
}

Entity *Engine::Impl::getHoveredEntity(const glm::vec2 &mousPos) {
  Entity *pCurrentObject = nullptr;

  // mouse on actor ?
  for (auto &&actor : _actors) {
    if (actor.get() == _pCurrentActor)
      continue;
    if (actor->getRoom() != _pRoom)
      continue;

    if (actor->contains(mousPos)) {
      if (!pCurrentObject || actor->getZOrder() < pCurrentObject->getZOrder()) {
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
    if (!rect.contains((glm::ivec2) mousPos))
      return;
    if (!pCurrentObject || pObj->getZOrder() <= pCurrentObject->getZOrder())
      pCurrentObject = pObj.get();
  });

  if (!pCurrentObject && _pRoom && _pRoom->getFullscreen() != 1) {
    // mouse on inventory object ?
    pCurrentObject = _hud.getInventory().getCurrentInventoryObject();
  }

  return pCurrentObject;
}

void Engine::Impl::updateHoveredEntity(bool isRightClick) {
  _hud.setVerbOverride(nullptr);
  if (!_hud.getCurrentVerb()) {
    _hud.setCurrentVerb(_hud.getVerb(VerbConstants::VERB_WALKTO));
  }

  if (_pUseObject) {
    _objId1 = _pUseObject ? _pUseObject->getId() : 0;
    _pObj2 = _hud.getHoveredEntity();
  } else {
    _objId1 = _hud.getHoveredEntity() ? _hud.getHoveredEntity()->getId() : 0;
    _pObj2 = nullptr;
  }

  // abort some invalid actions
  if (!_objId1 || !_hud.getCurrentVerb()) {
    return;
  }

  if (_pObj2 && _pObj2->getId() == _objId1) {
    _pObj2 = nullptr;
  }

  if (_objId1 && isRightClick) {
    _hud.setVerbOverride(_hud.getVerb(EntityManager::getScriptObjectFromId<Entity>(_objId1)->getDefaultVerb(
        VerbConstants::VERB_LOOKAT)));
  }

  auto verbId = _hud.getCurrentVerb()->id;
  switch (verbId) {
  case VerbConstants::VERB_WALKTO: {
    auto pObj1 = EntityManager::getScriptObjectFromId<Entity>(_objId1);
    if (pObj1 && pObj1->isInventoryObject()) {
      _hud.setVerbOverride(_hud.getVerb(EntityManager::getScriptObjectFromId<Entity>(_objId1)->getDefaultVerb(
          VerbConstants::VERB_LOOKAT)));
    }
    break;
  }
  case VerbConstants::VERB_TALKTO:
    // select actor/object only if talkable flag is set
    if (!hasFlag(_objId1, ObjectFlagConstants::TALKABLE)) {
      _objId1 = 0;
    }
    break;
  case VerbConstants::VERB_GIVE: {
    auto pObj1 = EntityManager::getScriptObjectFromId<Entity>(_objId1);
    if (!pObj1->isInventoryObject())
      _objId1 = 0;

    // select actor/object only if giveable flag is set
    if (_pObj2 && !hasFlag(_pObj2->getId(), ObjectFlagConstants::GIVEABLE))
      _pObj2 = nullptr;
    break;
  }
  default: {
    auto pActor = EntityManager::getScriptObjectFromId<Actor>(_objId1);
    if (pActor) {
      _objId1 = 0;
    }
    break;
  }
  }
}

Entity *Engine::Impl::getEntity(Entity *pEntity) const {
  if (!pEntity)
    return nullptr;

  // if an actor has the same name then get its flags
  auto itActor = std::find_if(_actors.begin(), _actors.end(), [pEntity](const auto &pActor) -> bool {
    return pActor->getName() == pEntity->getName();
  });
  if (itActor != _actors.end()) {
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
  auto actor = _pCurrentActor;
  if (!actor)
    return;

  auto &scalings = _pRoom->getScalings();
  auto &objects = _pRoom->getObjects();
  for (auto &&object : objects) {
    if (object->getType() != ObjectType::Trigger)
      continue;
    if (object->getRealHotspot().contains((glm::ivec2) actor->getPosition())) {
      auto it = std::find_if(scalings.begin(), scalings.end(), [&object](const auto &s) -> bool {
        return s.getName() == object->getName();
      });
      if (it != scalings.end()) {
        _pRoom->setRoomScaling(*it);
        return;
      }
    }
  }
  if (!scalings.empty()) {
    _pRoom->setRoomScaling(scalings[0]);
  }
}

const Verb *Engine::Impl::getHoveredVerb() const {
  if (!_hud.getActive())
    return nullptr;
  if (_pRoom && _pRoom->getFullscreen() == 1)
    return nullptr;

  return _hud.getHoveredVerb();
}

void Engine::Impl::stopTalking() const {
  for (auto &&a : _pEngine->getActors()) {
    a->stopTalking();
  }
  for (auto &&a : _pEngine->getRoom()->getObjects()) {
    a->stopTalking();
  }
}

void Engine::Impl::stopTalkingExcept(Entity *pEntity) const {
  for (auto &&a : _pEngine->getActors()) {
    if (a.get() == pEntity)
      continue;
    a->stopTalking();
  }

  for (auto &&a : _pEngine->getRoom()->getObjects()) {
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
  for (auto &key : _oldKeyDowns) {
    if (isKeyPressed(key)) {
      cmdMgr.execute(key);
      cmdMgr.execute(key, false);
    }
  }

  for (auto &key : _newKeyDowns) {
    if (_oldKeyDowns.find(key) != _oldKeyDowns.end()) {
      cmdMgr.execute(key, true);
    }
  }

  _oldKeyDowns.clear();
  for (auto key : _newKeyDowns) {
    _oldKeyDowns.insert(key);
  }
}

bool Engine::Impl::isKeyPressed(const Input &key) {
  auto wasDown = _oldKeyDowns.find(key) != _oldKeyDowns.end();
  auto isDown = _newKeyDowns.find(key) != _newKeyDowns.end();
  return wasDown && !isDown;
}

InputConstants Engine::Impl::toKey(const std::string &keyText) {
  if (keyText.length() == 1) {
    return static_cast<InputConstants>(keyText[0]);
  }
  return InputConstants::NONE;
}

void Engine::Impl::updateKeyboard() {
  if (_oldKeyDowns.empty())
    return;

  if (_pRoom) {
    for (auto key : _oldKeyDowns) {
      if (isKeyPressed(key) && ScriptEngine::rawExists(_pRoom, "pressedKey")) {
        ScriptEngine::rawCall(_pRoom, "pressedKey", static_cast<int>(key.input));
      }
    }
  }

  int currentActorIndex = getCurrentActorIndex();
  if (currentActorIndex == -1)
    return;

  const auto &verbSlot = _hud.getVerbSlot(currentActorIndex);
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
  _hud.setCurrentVerb(pVerb);
  _useFlag = UseFlag::None;
  _pUseObject = nullptr;
  _objId1 = 0;
  _pObj2 = nullptr;

  ScriptEngine::rawCall("onVerbClick");
}

bool Engine::Impl::clickedAt(const glm::vec2 &pos) const {
  if (!_pRoom)
    return false;

  bool handled = false;
  if (ScriptEngine::rawExists(_pRoom, _clickedAtCallback)) {
    ScriptEngine::rawCallFunc(handled, _pRoom, _clickedAtCallback, pos.x, pos.y);
    if (handled)
      return true;
  }

  if (!_pCurrentActor)
    return false;

  if (!ScriptEngine::rawExists(_pCurrentActor, _clickedAtCallback))
    return false;

  ScriptEngine::rawCallFunc(handled, _pCurrentActor, _clickedAtCallback, pos.x, pos.y);
  return handled;
}

void Engine::Impl::drawWalkboxes(ngf::RenderTarget &target) const {
  if (!_pRoom || _showDrawWalkboxes == 0)
    return;

  auto at = _camera.getRect().getTopLeft();
  ngf::Transform t;
  t.setPosition(-at);
  ngf::RenderStates states;
  states.transform = t.getTransform();

  if (_showDrawWalkboxes & 4) {
    for (const auto &walkbox : _pRoom->getWalkboxes()) {
      WalkboxDrawable wd(walkbox, _pRoom->getScreenSize().y);
      wd.draw(target, states);
    }
  }

  if (_showDrawWalkboxes & 1) {
    for (const auto &walkbox : _pRoom->getGraphWalkboxes()) {
      WalkboxDrawable wd(walkbox, _pRoom->getScreenSize().y);
      wd.draw(target, states);
    }
  }

  if (_showDrawWalkboxes & 2) {
    const auto *pGraph = _pRoom->getGraph();
    if (pGraph) {
      auto height = _pRoom->getRoomSize().y;
      ng::GraphDrawable d(*pGraph, height);
      d.draw(target, states);
    }
  }
}

void Engine::Impl::drawPause(ngf::RenderTarget &target) const {
  if (_state != EngineState::Paused)
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
      _pEngine->getPreferences().getUserPreference(PreferenceNames::RetroFonts, PreferenceDefaultValues::RetroFonts);
  auto &font = _pEngine->getResourceManager().getFont(retroFonts ? "FontRetroSheet" : "FontModernSheet");

  Text text;
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
  _threads.erase(std::remove_if(_threads.begin(), _threads.end(), [](const auto &t) -> bool {
    return !t || t->isStopped();
  }), _threads.end());
}

void Engine::Impl::drawCursor(ngf::RenderTarget &target) const {
  if (!_cursorVisible)
    return;
  if (!_showCursor && _dialogManager.getState() != DialogManagerState::WaitingForChoice)
    return;

  auto cursorSize = glm::vec2(68.f, 68.f);
  const auto &gameSheet = Locator<ResourceManager>::get().getSpriteSheet("GameSheet");

  const auto view = target.getView();
  target.setView(ngf::View(ngf::frect::fromPositionSize({0, 0}, {Screen::Width, Screen::Height})));

  auto screenSize = _pRoom->getScreenSize();
  auto pos = toDefaultView((glm::ivec2) _mousePos, screenSize);

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
  if (_state == EngineState::Paused)
    return gameSheet.getRect("cursor_pause");

  if (_state == EngineState::Options)
    return gameSheet.getRect("cursor");

  if (_dialogManager.getState() != DialogManagerState::None)
    return gameSheet.getRect("cursor");

  if (_cursorDirection & CursorDirection::Left) {
    return _cursorDirection & CursorDirection::Hotspot ? gameSheet.getRect("hotspot_cursor_left")
                                                       : gameSheet.getRect("cursor_left");
  }
  if (_cursorDirection & CursorDirection::Right) {
    return _cursorDirection & CursorDirection::Hotspot ? gameSheet.getRect("hotspot_cursor_right")
                                                       : gameSheet.getRect("cursor_right");
  }
  if (_cursorDirection & CursorDirection::Up) {
    return _cursorDirection & CursorDirection::Hotspot ? gameSheet.getRect("hotspot_cursor_back")
                                                       : gameSheet.getRect("cursor_back");
  }
  if (_cursorDirection & CursorDirection::Down) {
    return (_cursorDirection & CursorDirection::Hotspot) ? gameSheet.getRect("hotspot_cursor_front")
                                                         : gameSheet.getRect("cursor_front");
  }
  return (_cursorDirection & CursorDirection::Hotspot) ? gameSheet.getRect("hotspot_cursor")
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

  auto pObj1 = EntityManager::getScriptObjectFromId<Entity>(_objId1);
  if (!pObj1)
    return pVerb;
  return _hud.getVerb(pObj1->getDefaultVerb(VerbConstants::VERB_WALKTO));
}

void Engine::Impl::drawCursorText(ngf::RenderTarget &target) const {
  if (!_cursorVisible)
    return;
  if (!_showCursor || _state != EngineState::Game)
    return;

  if (_dialogManager.getState() != DialogManagerState::None)
    return;

  auto pVerb = _hud.getVerbOverride();
  if (!pVerb)
    pVerb = _hud.getCurrentVerb();
  if (!pVerb)
    return;

  pVerb = overrideVerb(pVerb);

  auto currentActorIndex = getCurrentActorIndex();
  if (currentActorIndex == -1)
    return;

  auto classicSentence = _pEngine->getPreferences().getUserPreference(PreferenceNames::ClassicSentence,
                                                                      PreferenceDefaultValues::ClassicSentence);

  const auto view = target.getView();
  target.setView(ngf::View(ngf::frect::fromPositionSize({0, 0}, {Screen::Width, Screen::Height})));

  auto retroFonts =
      _pEngine->getPreferences().getUserPreference(PreferenceNames::RetroFonts, PreferenceDefaultValues::RetroFonts);
  auto &font = _pEngine->getResourceManager().getFont(retroFonts ? "FontRetroSheet" : "FontModernSheet");

  std::wstring s;
  if (pVerb->id != VerbConstants::VERB_WALKTO || _hud.getHoveredEntity()) {
    auto id = std::strtol(pVerb->text.substr(1).data(), nullptr, 10);
    s.append(ng::Engine::getText(id));
  }
  auto pObj1 = EntityManager::getScriptObjectFromId<Entity>(_objId1);
  if (pObj1) {
    s.append(L" ").append(getDisplayName(ng::Engine::getText(pObj1->getName())));
    if (DebugFeatures::showHoveredObject) {
      if (pObj1) {
        s.append(L"(").append(towstring(pObj1->getKey())).append(L")");
      }
    }
  }
  appendUseFlag(s);
  if (_pObj2) {
    s.append(L" ").append(getDisplayName(ng::Engine::getText(_pObj2->getName())));
  }

  Text text;
  text.setFont(font);
  text.setColor(_hud.getVerbUiColors(currentActorIndex).sentence);
  text.setWideString(s);

  // do display cursor position:
  if (DebugFeatures::showCursorPosition) {
    std::wstringstream ss;
    std::wstring txt = text.getWideString();
    ss << txt << L" (" << std::fixed << std::setprecision(0) << _mousePosInRoom.x << L"," << _mousePosInRoom.y
       << L")";
    text.setWideString(ss.str());
  }

  auto screenSize = _pRoom->getScreenSize();
  auto pos = toDefaultView((glm::ivec2) _mousePos, screenSize);

  auto bounds = getGlobalBounds(text);
  if (classicSentence) {
    auto y = Screen::Height - 210.f;
    auto x = Screen::HalfWidth - bounds.getWidth() / 2.f;
    text.getTransform().setPosition({x, y});
  } else {
    auto y = pos.y - 20 < 40 ? pos.y + 80 : pos.y - 40;
    auto x = std::clamp<float>(pos.x - bounds.getWidth() / 2.f, 20.f, Screen::Width - 20.f - bounds.getWidth());
    text.getTransform().setPosition({x, y - bounds.getHeight()});
  }
  text.draw(target, {});
  target.setView(view);
}

void Engine::Impl::drawNoOverride(ngf::RenderTarget &target) const {
  if (_noOverrideElapsed > ngf::TimeSpan::seconds(2))
    return;

  auto &gameSheet = Locator<ResourceManager>::get().getSpriteSheet("GameSheet");
  const auto view = target.getView();
  target.setView(ngf::View(ngf::frect::fromPositionSize({0, 0}, {Screen::Width, Screen::Height})));

  ngf::Color c(ngf::Colors::White);
  c.a = static_cast<sf::Uint8>((2.f - _noOverrideElapsed.getTotalSeconds() / 2.f) * 255);
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
  switch (_useFlag) {
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
  for (int i = 0; i < static_cast<int>(_actorsIconSlots.size()); i++) {
    const auto &selectableActor = _actorsIconSlots.at(i);
    if (selectableActor.pActor == _pCurrentActor) {
      return i;
    }
  }
  return -1;
}

void Engine::Impl::drawHud(ngf::RenderTarget &target) const {
  if (_state != EngineState::Game)
    return;

  _hud.draw(target, {});
}

void Engine::Impl::captureScreen(const std::string &path) const {
  ngf::RenderTexture target({320, 180});
  _pEngine->draw(target, true);
  target.display();

  auto screenshot = target.capture();
  screenshot.saveToFile(path);
}
}
