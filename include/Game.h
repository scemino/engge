#pragma once
#include "SFML/Graphics.hpp"
#include "SFML/Audio.hpp"
#include "Object.h"
#include "OptionsDialog.h"
#include "Function.h"
#include "NonCopyable.h"
#include "Engine.h"

namespace ng
{
class InputEventHandler
{
public:
  virtual void run(sf::Event event) {}
  virtual ~InputEventHandler() = default;
};

class Game : public NonCopyable
{
public:
  Game();
  ~Game();

  void run();
  void setEngine(Engine* pEngine);

  sf::RenderWindow &getWindow() { return _window; }
  std::vector<std::unique_ptr<InputEventHandler>> &getInputEventHandlers() { return _inputEventHandlers; }

private:
  void processEvents();
  void update(const sf::Time &time);
  void render();

  Engine *_pEngine{};
  sf::RenderWindow _window;
  std::vector<std::unique_ptr<InputEventHandler>> _inputEventHandlers;
  OptionsDialog _optionsDialog;
};
} // namespace ng
