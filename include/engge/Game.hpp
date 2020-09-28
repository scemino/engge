#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include "engge/Entities/Objects/Object.hpp"
#include "engge/Engine/Function.hpp"
#include "engge/System/NonCopyable.hpp"
#include "engge/Engine/Engine.hpp"

namespace ng {
class Game : public NonCopyable {
public:
  Game();
  ~Game();

  void run();
  void setEngine(Engine *pEngine);

private:
  void processEvents();
  void update(const sf::Time &time);
  void render();
  static sf::Uint32 getStyle() ;

private:
  Engine *_pEngine{nullptr};
  sf::RenderWindow _window{};
  bool _isMousePressed{false};
  bool _isKeyPressed{false};
  sf::Vector2i _pos = sf::Mouse::getPosition();
};
} // namespace ng
