#pragma once
#include "Function.h"
#include "Object.h"
#include "EngineSettings.h"
#include "Room.h"
#include "Actor.h"
#include "TextureManager.h"
#include "TextDatabase.h"
#include "Font.h"
#include "Verb.h"
#include "SpriteSheet.h"
#include "NonCopyable.h"
#include "Dialog/DialogManager.h"
#include "Preferences.h"
#include "SoundManager.h"
#include "FntFont.h"
#include "ActorIconSlot.h"
#include "ActorIcons.h"
#include "Inventory.h"

namespace ng
{
class Cutscene;
class VerbExecute
{
public:
  virtual ~VerbExecute() = default;
  virtual void use(const InventoryObject *pObjectSource, Object *pObjectTarget) = 0;
  virtual void execute(Object *pObject, const Verb *pVerb) = 0;
  virtual void execute(const InventoryObject *pObject, const Verb *pVerb) = 0;
};

class ScriptExecute
{
public:
  virtual ~ScriptExecute() = default;
  virtual void execute(const std::string &code) = 0;
  virtual bool executeCondition(const std::string &code) = 0;
  virtual SoundDefinition *getSoundDefinition(const std::string &name) = 0;
};

enum class CursorDirection
{
  None = 0,
  Left = 1,
  Right = 1 << 1,
  Up = 1 << 2,
  Down = 1 << 3,
  Hotspot = 1 << 4
};

enum UseFlag
{
  None = 0,
  UseWith,
  UseOn,
  UseIn
};

class Engine : public NonCopyable
{
public:
  explicit Engine(EngineSettings &settings);
  ~Engine();

  void setCameraAt(const sf::Vector2f &at);
  void moveCamera(const sf::Vector2f &offset);
  sf::Vector2f getCameraAt() const { return _cameraPos; }

  void setWindow(sf::RenderWindow &window) { _pWindow = &window; }

  TextureManager &getTextureManager() { return _textureManager; }
  EngineSettings &getSettings() { return _settings; }

  Room &getRoom() { return *_pRoom; }
  void setRoom(Room *room);
  std::string getText(int id) const { return _textDb.getText(id); }
  void setFadeAlpha(float fade) { _fadeColor.a = static_cast<uint8_t>(fade * 255); }
  float getFadeAlpha() const { return _fadeColor.a / 255.f; }
  void setFadeColor(sf::Color color) { _fadeColor = color; }
  sf::Color getFadeColor() const { return _fadeColor; }

  void addActor(std::unique_ptr<Actor> actor) { _actors.push_back(std::move(actor)); }
  void addRoom(std::unique_ptr<Room> room) { _rooms.push_back(std::move(room)); }
  const std::vector<std::unique_ptr<Room>> &getRooms() const { return _rooms; }
  void addFunction(std::unique_ptr<Function> function) { _newFunctions.push_back(std::move(function)); }
  void cutscene(std::unique_ptr<Cutscene> function);
  bool inCutscene() const;
  void cutsceneOverride();

  std::vector<std::unique_ptr<Actor>> &getActors() { return _actors; }

  void update(const sf::Time &elapsed);
  void draw(sf::RenderWindow &window) const;

  void setCurrentActor(Actor *pCurrentActor);
  Actor *getCurrentActor() { return _pCurrentActor; }

  void setVerb(int characterSlot, int verbSlot, const Verb &verb) { _verbSlots[characterSlot].setVerb(verbSlot, verb); }
  void setVerbUiColors(int characterSlot, VerbUiColors colors) { _verbUiColors[characterSlot] = colors; }
  VerbUiColors &getVerbUiColors(int characterSlot) { return _verbUiColors[characterSlot]; }

  void setInputActive(bool active);
  void inputSilentOff();
  void setInputVerbs(bool on);
  bool getInputActive() const { return _inputActive; }
  bool getInputVerbs() const { return _inputVerbsActive; }

  void follow(Actor *pActor) { _pFollowActor = pActor; }
  void setVerbExecute(std::unique_ptr<VerbExecute> verbExecute) { _pVerbExecute = std::move(verbExecute); }
  void setScriptExecute(std::unique_ptr<ScriptExecute> scriptExecute) { _pScriptExecute = std::move(scriptExecute); }
  const Verb *getVerb(int id) const;

  void addThread(HSQUIRRELVM thread) { _threads.push_back(thread); }
  void stopThread(HSQUIRRELVM thread);
  bool isThreadAlive(HSQUIRRELVM thread) const;

  void startDialog(const std::string &dialog, const std::string &node);
  void execute(const std::string &code);
  SoundDefinition *getSoundDefinition(const std::string &name);
  bool executeCondition(const std::string &code);

  sf::Vector2f getMousePos() const { return _mousePos; }

  Preferences &getPreferences() { return _preferences; }
  SoundManager &getSoundManager() { return _soundManager; }
  DialogManager &getDialogManager() { return _dialogManager; }

  void addSelectableActor(int index, Actor *pActor);
  void actorSlotSelectable(Actor *pActor, bool selectable);
  void actorSlotSelectable(int index, bool selectable);
  void setUseFlag(UseFlag flag, const InventoryObject *object)
  {
    _useFlag = flag;
    _pUseObject = object;
  }
  UseFlag getUseFlag(UseFlag flag) const { return _useFlag; }
  sf::Time getTime() const { return _time; }

  void setVm(HSQUIRRELVM vm) { _vm = vm; }
  HSQUIRRELVM getVm() const { return _vm; }

private:
  sf::IntRect getVerbRect(int id, std::string lang = "en", bool isRetro = false) const;
  void drawVerbs(sf::RenderWindow &window) const;
  void drawCursor(sf::RenderWindow &window) const;
  void drawCursorText(sf::RenderWindow &window) const;
  void drawFade(sf::RenderWindow &window) const;
  void clampCamera();
  int getCurrentActorIndex() const;
  sf::IntRect getCursorRect() const;
  void appendUseFlag(std::string &sentence) const;
  bool clickedAt(const sf::Vector2f &pos);

private:
  EngineSettings &_settings;
  TextureManager _textureManager;
  Room *_pRoom;
  std::vector<std::unique_ptr<Actor>> _actors;
  std::vector<std::unique_ptr<Room>> _rooms;
  std::vector<std::unique_ptr<Function>> _newFunctions;
  std::vector<std::unique_ptr<Function>> _functions;
  std::unique_ptr<Cutscene> _pCutscene;
  sf::Color _fadeColor;
  sf::RenderWindow *_pWindow;
  sf::Vector2f _cameraPos;
  TextDatabase _textDb;
  FntFont _fntFont;
  Actor *_pCurrentActor;
  std::array<VerbSlot, 6> _verbSlots;
  std::array<VerbUiColors, 6> _verbUiColors;
  bool _inputActive;
  bool _showCursor;
  bool _inputVerbsActive;
  SpriteSheet _verbSheet, _gameSheet;
  nlohmann::json _jsonInventoryItems;
  Actor *_pFollowActor;
  sf::IntRect _verbRects[9];
  Object *_pCurrentObject;
  const InventoryObject *_pUseObject;
  sf::Vector2f _mousePos;
  std::unique_ptr<VerbExecute> _pVerbExecute;
  std::unique_ptr<ScriptExecute> _pScriptExecute;
  const Verb *_pVerb;
  std::vector<HSQUIRRELVM> _threads;
  DialogManager _dialogManager;
  Preferences _preferences;
  SoundManager _soundManager;
  CursorDirection _cursorDirection;
  std::array<ActorIconSlot, 6> _actorsIconSlots;
  UseFlag _useFlag;
  ActorIcons _actorIcons;
  Inventory _inventory;
  HSQUIRRELVM _vm;
  sf::Time _time;
};
} // namespace ng
