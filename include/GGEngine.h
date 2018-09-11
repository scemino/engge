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
  SoundId() {}
  ~SoundId()
  {
    sound.stop();
  }
  sf::SoundBuffer buffer;
  sf::Sound sound;
};

class GGEngine : public NonCopyable
{
public:
  GGEngine(const GGEngineSettings &settings);
  ~GGEngine();

  void setCameraAt(float x, float y);
  void moveCamera(float x, float y);
  sf::Vector2f getCameraAt() const { return _cameraPos; }
  void cameraPanTo(float x, float y, float timeInSec);
  void setWindow(sf::RenderWindow &window) { _pWindow = &window; }
  const GGEngineSettings &getSettings() const { return _settings; }
  TextureManager &getTextureManager() { return _textureManager; }
  GGRoom &getRoom() { return _room; }
  GGFont &getFont() { return _font; }

  void addActor(GGActor &actor) { _actors.push_back(std::unique_ptr<GGActor>(&actor)); }
  void addFunction(std::unique_ptr<Function> function) { _newFunctions.push_back(std::move(function)); }

  void loopMusic(const std::string &name);
  SoundId *playSound(const std::string &name, bool loop);
  void stopSound(SoundId &sound);
  void fadeOutSound(SoundId &id, float time);

  void fadeTo(float a, float time);
  void offsetTo(GGObject &object, float x, float y, float time);
  void alphaTo(GGObject &object, float x, float time);
  void playState(GGObject &object, int index);
  void update(const sf::Time &elapsed);
  void draw(sf::RenderWindow &window) const;
  std::string getText(int id) { return _textDb.getText(id); }

  void setFadeAlpha(sf::Uint8 fade) { _fadeAlpha = fade; }
  sf::Uint8 getFadeAlpha() const { return _fadeAlpha; }

private:
  const GGEngineSettings &_settings;
  TextureManager _textureManager;
  GGRoom _room;
  std::vector<std::unique_ptr<GGActor>> _actors;
  std::vector<std::unique_ptr<Function>> _newFunctions;
  std::vector<std::unique_ptr<Function>> _functions;
  std::vector<std::unique_ptr<SoundId>> _sounds;
  sf::Music _music;
  sf::SoundBuffer _buffer;
  sf::Sound _sound;
  sf::Uint8 _fadeAlpha;
  sf::RenderWindow *_pWindow;
  sf::Vector2f _cameraPos;
  GGTextDatabase _textDb;
  GGFont _font;
};
} // namespace gg
