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
#include "NonCopyable.h"

namespace gg
{
class SoundId;

class SoundDefinition
{
  friend class SoundId;

public:
  SoundDefinition(const std::string &path)
      : _path(path), _isLoaded(false)
  {
  }

  const std::string &getPath() const { return _path; };

  void load()
  {
    if (_isLoaded)
      return;
    _isLoaded = _buffer.loadFromFile(_path);
    if (!_isLoaded)
    {
      std::cerr << "Can't load the sound" << _path << std::endl;
    }
  }

private:
  std::string _path;
  bool _isLoaded;
  sf::SoundBuffer _buffer;
};

class SoundId
{
public:
  SoundId(SoundDefinition &soundDefinition)
      : _soundDefinition(soundDefinition)
  {
  }

  ~SoundId()
  {
    stop();
  }

  void play(bool loop = false)
  {
    _soundDefinition.load();
    _sound.setBuffer(_soundDefinition._buffer);
    _sound.setLoop(false);
    _sound.play();
  }

  void setVolume(float volume)
  {
    std::cout << "setVolume(" << volume << ")" << std::endl; 
    _sound.setVolume(volume);
  }

  float getVolume() const
  {
    return _sound.getVolume();
  }

  void stop()
  {
    _sound.stop();
  }

private:
  SoundDefinition &_soundDefinition;
  sf::Sound _sound;
};

struct Verb
{
  std::string id;
  std::string image;
  std::string func;
  std::string text;
  std::string key;
};

class VerbSlot
{
public:
  void setVerb(int index, const Verb &verb) { _verbs[index] = verb; }
  const Verb &getVerb(int index) const { return _verbs[index]; }

private:
  std::array<Verb, 10> _verbs;
};

struct VerbUiColors
{
  sf::Color sentence;
  sf::Color verbNormal;
  sf::Color verbNormalTint;
  sf::Color verbHighlight;
  sf::Color verbHighlightTint;
  sf::Color dialogNormal;
  sf::Color dialogHighlight;
  sf::Color inventoryFrame;
  sf::Color inventoryBackground;
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
  void addRoom(GGRoom &room) { _rooms.push_back(std::unique_ptr<GGRoom>(&room)); }
  void addFunction(std::unique_ptr<Function> function) { _newFunctions.push_back(std::move(function)); }

  std::vector<std::unique_ptr<GGActor>> &getActors() { return _actors; }

  void loopMusic(const std::string &name);
  std::shared_ptr<SoundDefinition> defineSound(const std::string &name);
  std::shared_ptr<SoundId> playSound(SoundDefinition &soundDefinition, bool loop = false);
  void stopSound(SoundId &sound);

  void playState(GGObject &object, int index);

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

private:
  sf::IntRect getVerbRect(const std::string &name, std::string lang = "en", bool isRetro = false) const;
  sf::IntRect getGameSheetRect(const std::string &name) const;
  sf::IntRect getInventoryItemsRect(const std::string &name) const;
  void drawVerbs(sf::RenderTarget &target) const;

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
  sf::Texture _verbTexture;
  sf::Texture _gameSheetTexture;
  sf::Texture _inventoryItemsTexture;
  bool _inputActive;
  bool _showCursor;
  nlohmann::json _jsonVerb;
  nlohmann::json _jsonGameSheet;
  nlohmann::json _jsonInventoryItems;
};
} // namespace gg
