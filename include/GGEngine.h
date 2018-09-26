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
class SoundId
{
public:
  SoundId() = default;

  ~SoundId();
  sf::SoundBuffer buffer;
  sf::Sound sound;
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
  void setFadeAlpha(sf::Uint8 fade) { _fadeAlpha = fade; }
  sf::Uint8 getFadeAlpha() const { return _fadeAlpha; }

  void addActor(GGActor &actor) { _actors.push_back(std::unique_ptr<GGActor>(&actor)); }
  void addRoom(GGRoom &room) { _rooms.push_back(std::unique_ptr<GGRoom>(&room)); }
  void addFunction(std::unique_ptr<Function> function) { _newFunctions.push_back(std::move(function)); }

  void loopMusic(const std::string &name);
  SoundId *defineSound(const std::string &name);
  SoundId *playSound(const std::string &name, bool loop);
  void stopSound(SoundId &sound);

  void playState(GGObject &object, int index);

  void update(const sf::Time &elapsed);
  void draw(sf::RenderWindow &window) const;

private:
  const GGEngineSettings &_settings;
  TextureManager _textureManager;
  GGRoom *_pRoom;
  std::vector<std::unique_ptr<GGActor>> _actors;
  std::vector<std::unique_ptr<GGRoom>> _rooms;
  std::vector<std::unique_ptr<Function>> _newFunctions;
  std::vector<std::unique_ptr<Function>> _functions;
  std::vector<std::unique_ptr<SoundId>> _sounds;
  sf::Music _music;
  sf::Uint8 _fadeAlpha;
  sf::RenderWindow *_pWindow;
  sf::Vector2f _cameraPos;
  GGTextDatabase _textDb;
  GGFont _font;
};
} // namespace gg
