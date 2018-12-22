#pragma once
#include "SFML/Graphics.hpp"
#include "SFML/Audio.hpp"
#include "Object.h"
#include "Function.h"
#include "NonCopyable.h"
#include "Engine.h"

namespace ng
{
class InputEventHandler
{
public:
  virtual void run(sf::Event event) {}
};

class Game : public NonCopyable
{
public:
  explicit Game(Engine &engine);
  ~Game();
  void run();

  sf::RenderWindow &getWindow() { return _window; }
  std::vector<std::unique_ptr<InputEventHandler>> &getInputEventHandlers() { return _inputEventHandlers; }

private:
  void processEvents();
  void update(const sf::Time &time);
  void render();

  Engine &_engine;
  sf::RenderWindow _window;
  std::vector<std::unique_ptr<InputEventHandler>> _inputEventHandlers;
};
} // namespace ng
