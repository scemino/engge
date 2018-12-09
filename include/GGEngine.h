#pragma once
#include "SFML/Audio.hpp"
#include "Function.h"
#include "GGObject.h"
#include "GGEngineSettings.h"
#include "GGRoom.h"
#include "GGActor.h"
#include "TextureManager.h"
#include "GGTextDatabase.h"
#include "GGFont.h"
#include "SoundDefinition.h"
#include "Verb.h"
#include "SpriteSheet.h"
#include "NonCopyable.h"

namespace gg
{
class VerbExecute
{
public:
  virtual ~VerbExecute() = default;
  virtual void execute(GGObject *pObject, const Verb *pVerb) = 0;
};

class GGEngine : public NonCopyable
{
public:
  explicit GGEngine(const GGEngineSettings &settings);
  ~GGEngine();

  void setCameraAt(const sf::Vector2f &at);
  void moveCamera(const sf::Vector2f &offset);
  sf::Vector2f getCameraAt() const { return _cameraPos; }

  void setWindow(sf::RenderWindow &window) { _pWindow = &window; }

  TextureManager &getTextureManager() { return _textureManager; }
  const GGEngineSettings &getSettings() const { return _settings; }

  GGRoom &getRoom() { return *_pRoom; }
  void setRoom(GGRoom *room) { _pRoom = room; }
  GGFont &getFont() { return _font; }
  std::string getText(int id) { return _textDb.getText(id); }
  void setFadeAlpha(float fade) { _fadeAlpha = static_cast<uint8_t>(fade * 255); }
  float getFadeAlpha() const { return _fadeAlpha / 255.f; }

  void addActor(std::unique_ptr<GGActor> actor) { _actors.push_back(std::move(actor)); }
  void addRoom(std::unique_ptr<GGRoom> room) { _rooms.push_back(std::move(room)); }
  void addFunction(std::unique_ptr<Function> function) { _newFunctions.push_back(std::move(function)); }

  std::vector<std::unique_ptr<GGActor>> &getActors() { return _actors; }

  void loopMusic(const std::string &name);
  std::shared_ptr<SoundDefinition> defineSound(const std::string &name);
  std::shared_ptr<SoundId> playSound(SoundDefinition &soundDefinition, bool loop = false);
  void stopSound(SoundId &sound);

  void update(const sf::Time &elapsed);
  void draw(sf::RenderWindow &window) const;

  void setCurrentActor(GGActor *pCurrentActor) { _pCurrentActor = pCurrentActor; }
  GGActor *getCurrentActor() { return _pCurrentActor; }

  void setVerb(int characterSlot, int verbSlot, const Verb &verb) { _verbSlots[characterSlot].setVerb(verbSlot, verb); }
  void setVerbUiColors(int characterSlot, VerbUiColors colors) { _verbUiColors[characterSlot] = colors; }

  void setInputActive(bool active)
  {
    _inputActive = active;
    _showCursor = active;
  }
  void inputSilentOff() { _inputActive = false; }
  bool getInputActive() const { return _inputActive; }

  void follow(GGActor *pActor) { _pFollowActor = pActor; }
  void setVerbExecute(std::unique_ptr<VerbExecute> verbExecute) { _pVerbExecute = std::move(verbExecute); }
  const Verb *getVerb(const std::string &id) const;

  void addThread(HSQUIRRELVM thread) { _threads.push_back(thread); }
  void stopThread(HSQUIRRELVM thread);
  bool isThreadAlive(HSQUIRRELVM thread) const;

private:
  sf::IntRect getVerbRect(const std::string &name, std::string lang = "en", bool isRetro = false) const;
  void drawVerbs(sf::RenderWindow &window) const;
  void drawInventory(sf::RenderWindow &window) const;
  void drawCursor(sf::RenderWindow &window) const;

private:
  const GGEngineSettings &_settings;
  TextureManager _textureManager;
  GGRoom *_pRoom;
  std::vector<std::unique_ptr<GGActor>> _actors;
  std::vector<std::unique_ptr<GGRoom>> _rooms;
  std::vector<std::unique_ptr<Function>> _newFunctions;
  std::vector<std::unique_ptr<Function>> _functions;
  std::vector<std::shared_ptr<SoundDefinition>> _sounds;
  std::vector<std::shared_ptr<SoundId>> _soundIds;
  sf::Music _music;
  sf::Uint8 _fadeAlpha;
  sf::RenderWindow *_pWindow;
  sf::Vector2f _cameraPos;
  GGTextDatabase _textDb;
  GGFont _font;
  GGActor *_pCurrentActor;
  std::array<VerbSlot, 6> _verbSlots;
  std::array<VerbUiColors, 6> _verbUiColors;
  bool _inputActive;
  bool _showCursor;
  SpriteSheet _verbSheet, _gameSheet, _inventoryItems;
  nlohmann::json _jsonInventoryItems;
  GGActor *_pFollowActor;
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
  GGObject *_pCurrentObject;
  sf::Vector2f _mousePos;
  std::unique_ptr<VerbExecute> _pVerbExecute;
  const Verb *_pVerb;
  std::vector<HSQUIRRELVM> _threads;
};
} // namespace gg
