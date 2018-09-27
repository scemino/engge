#pragma once
#include "SFML/Graphics.hpp"
#include "NonCopyable.h"

namespace gg
{
enum class AnimState
{
  Pause,
  Play
};

class GGAnimation
{
public:
  explicit GGAnimation(const sf::Texture &texture, const std::string &name);
  ~GGAnimation();

  void setName(const std::string &name) { _name = name; }
  const std::string &getName() const { return _name; }

  std::vector<sf::IntRect> &getRects() { return _rects; }
  std::vector<sf::IntRect> &getSourceRects() { return _sourceRects; }

  int getFps() const { return _fps; }
  void setFps(int fps) { _fps = fps; }

  void update(const sf::Time &elapsed);
  void draw(sf::RenderWindow &window, const sf::RenderStates &states) const;

  void reset();
  void play(bool loop = false);
  void pause() { _state = AnimState::Pause; }

  sf::Sprite &getSprite() { return _sprite; }
  const sf::Sprite &getSprite() const { return _sprite; }

private:
  sf::Sprite _sprite;
  std::string _name;
  std::vector<sf::IntRect> _rects;
  std::vector<sf::IntRect> _sourceRects;
  int _fps;
  sf::Time _time;
  size_t _index;
  AnimState _state;
  bool _loop;
};
} // namespace gg
