#pragma once
#include "SFML/Graphics.hpp"
#include "SFML/Audio.hpp"
#include "GGObject.h"
#include "Function.h"
#include "NonCopyable.h"
#include "GGEngine.h"

namespace gg
{
class InputEventHandler
{
public:
  virtual void run(sf::Event event) {}
};

class Game : public NonCopyable
{
public:
  Game(GGEngine &engine);
  ~Game();
  void run();

  sf::RenderWindow &getWindow() { return _window; }
  std::vector<std::unique_ptr<InputEventHandler>> &getInputEventHandlers() { return _inputEventHandlers; }

private:
  void processEvents();
  void update(const sf::Time &time);
  void render();

  GGEngine &_engine;
  sf::RenderWindow _window;
  std::vector<std::unique_ptr<InputEventHandler>> _inputEventHandlers;
};
} // namespace gg
