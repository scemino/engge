#pragma once
#include "SFML/Audio.hpp"
#include "Function.h"
#include "NGObject.h"
#include "NGEngineSettings.h"
#include "NGRoom.h"
#include "NGActor.h"
#include "TextureManager.h"
#include "NGTextDatabase.h"
#include "NGFont.h"
#include "SoundDefinition.h"
#include "Verb.h"
#include "SpriteSheet.h"
#include "NonCopyable.h"
#include "Dialog/DialogManager.h"

namespace ng
{
class VerbExecute
{
public:
  virtual ~VerbExecute() = default;
  virtual void execute(NGObject *pObject, const Verb *pVerb) = 0;
};

class ScriptExecute
{
public:
  virtual ~ScriptExecute() = default;
  virtual void execute(const std::string& code) = 0;
  virtual bool executeCondition(const std::string& code) = 0;
};

struct DialogSlot
{
int id;
std::string text;
std::string label;
};

class NGEngine : public NonCopyable
{
public:
  explicit NGEngine(const NGEngineSettings &settings);
  ~NGEngine();

  void setCameraAt(const sf::Vector2f &at);
  void moveCamera(const sf::Vector2f &offset);
  sf::Vector2f getCameraAt() const { return _cameraPos; }

  void setWindow(sf::RenderWindow &window) { _pWindow = &window; }

  TextureManager &getTextureManager() { return _textureManager; }
  const NGEngineSettings &getSettings() const { return _settings; }

  NGRoom &getRoom() { return *_pRoom; }
  void setRoom(NGRoom *room) { _pRoom = room; }
  NGFont &getFont() { return _font; }
  std::string getText(int id) { return _textDb.getText(id); }
  void setFadeAlpha(float fade) { _fadeAlpha = static_cast<uint8_t>(fade * 255); }
  float getFadeAlpha() const { return _fadeAlpha / 255.f; }

  void addActor(std::unique_ptr<NGActor> actor) { _actors.push_back(std::move(actor)); }
  void addRoom(std::unique_ptr<NGRoom> room) { _rooms.push_back(std::move(room)); }
  void addFunction(std::unique_ptr<Function> function) { _newFunctions.push_back(std::move(function)); }

  std::vector<std::unique_ptr<NGActor>> &getActors() { return _actors; }

  std::shared_ptr<SoundDefinition> defineSound(const std::string &name);
  std::shared_ptr<SoundId> playSound(SoundDefinition &soundDefinition, bool loop = false);
  std::shared_ptr<SoundId> loopMusic(SoundDefinition &soundDefinition);
  void stopSound(SoundId &sound);

  void update(const sf::Time &elapsed);
  void draw(sf::RenderWindow &window) const;

  void setCurrentActor(NGActor *pCurrentActor) { _pCurrentActor = pCurrentActor; }
  NGActor *getCurrentActor() { return _pCurrentActor; }

  void setVerb(int characterSlot, int verbSlot, const Verb &verb) { _verbSlots[characterSlot].setVerb(verbSlot, verb); }
  void setVerbUiColors(int characterSlot, VerbUiColors colors) { _verbUiColors[characterSlot] = colors; }

  void setInputActive(bool active)
  {
    _inputActive = active;
    _showCursor = active;
  }
  void inputSilentOff() { _inputActive = false; }
  bool getInputActive() const { return _inputActive; }

  void follow(NGActor *pActor) { _pFollowActor = pActor; }
  void setVerbExecute(std::unique_ptr<VerbExecute> verbExecute) { _pVerbExecute = std::move(verbExecute); }
  void setScriptExecute(std::unique_ptr<ScriptExecute> scriptExecute) { _pScriptExecute = std::move(scriptExecute); }
  const Verb *getVerb(const std::string &id) const;

  void addThread(HSQUIRRELVM thread) { _threads.push_back(thread); }
  void stopThread(HSQUIRRELVM thread);
  bool isThreadAlive(HSQUIRRELVM thread) const;

  void startDialog(const std::string& dialog);
  std::array<DialogSlot,8>& getDialog() {return _dialog;}
  void execute(const std::string& code);
  bool executeCondition(const std::string& code);

private:
  sf::IntRect getVerbRect(const std::string &name, std::string lang = "en", bool isRetro = false) const;
  void drawVerbs(sf::RenderWindow &window) const;
  void drawInventory(sf::RenderWindow &window) const;
  void drawCursor(sf::RenderWindow &window) const;
  bool drawDialog(sf::RenderWindow &window) const;

private:
  const NGEngineSettings &_settings;
  TextureManager _textureManager;
  NGRoom *_pRoom;
  std::vector<std::unique_ptr<NGActor>> _actors;
  std::vector<std::unique_ptr<NGRoom>> _rooms;
  std::vector<std::unique_ptr<Function>> _newFunctions;
  std::vector<std::unique_ptr<Function>> _functions;
  std::vector<std::shared_ptr<SoundDefinition>> _sounds;
  std::vector<std::shared_ptr<SoundId>> _soundIds;
  sf::Music _music;
  sf::Uint8 _fadeAlpha;
  sf::RenderWindow *_pWindow;
  sf::Vector2f _cameraPos;
  NGTextDatabase _textDb;
  NGFont _font;
  NGActor *_pCurrentActor;
  std::array<VerbSlot, 6> _verbSlots;
  std::array<VerbUiColors, 6> _verbUiColors;
  bool _inputActive;
  bool _showCursor;
  SpriteSheet _verbSheet, _gameSheet, _inventoryItems;
  nlohmann::json _jsonInventoryItems;
  NGActor *_pFollowActor;
  sf::IntRect _cursorRect;
  sf::IntRect _cursorLeftRect;
  sf::IntRect _cursorRightRect;
  sf::IntRect _cursorFrontRect;
  sf::IntRect _cursorBackRect;
  sf::IntRect _hotspotCursorRect;
  sf::IntRect _hotspotCursorLeftRect;
  sf::IntRect _hotspotCursorRightRect;
  sf::IntRect _hotspotCursorFrontRect;
  sf::IntRect _hotspotCursorBackRect;
  sf::IntRect _verbRects[9];
  NGObject *_pCurrentObject;
  sf::Vector2f _mousePos;
  std::unique_ptr<VerbExecute> _pVerbExecute;
  std::unique_ptr<ScriptExecute> _pScriptExecute;
  const Verb *_pVerb;
  std::vector<HSQUIRRELVM> _threads;
  std::array<DialogSlot,8> _dialog;
  DialogManager _dialogManager;
};
} // namespace ng
