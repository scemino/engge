#pragma once
#include "SFML/Graphics.hpp"
#include "SFML/Audio.hpp"
#include "NGObject.h"
#include "Function.h"
#include "NonCopyable.h"
#include "NGEngine.h"

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
  explicit Game(NGEngine &engine);
  ~Game();
  void run();

  sf::RenderWindow &getWindow() { return _window; }
  std::vector<std::unique_ptr<InputEventHandler>> &getInputEventHandlers() { return _inputEventHandlers; }

private:
  void processEvents();
  void update(const sf::Time &time);
  void render();

  NGEngine &_engine;
  sf::RenderWindow _window;
  std::vector<std::unique_ptr<InputEventHandler>> _inputEventHandlers;
};
} // namespace ng
