#pragma once
#include "SFML/Graphics.hpp"
#include "SFML/Audio.hpp"
#include "Entities/Objects/Object.hpp"
#include "Engine/Function.hpp"
#include "System/NonCopyable.hpp"
#include "Engine/Engine.hpp"

namespace ng
{
class InputEventHandler
{
public:
  virtual void run(sf::Event event) = 0;
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
  static const sf::Time TimePerFrame;
};
} // namespace ng
