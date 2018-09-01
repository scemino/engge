#pragma once
#include "SFML/Graphics.hpp"
#include "NonCopyable.h"

namespace gg
{
class GGAnim
{
public:
  explicit GGAnim(const sf::Texture &texture, const std::string &name);
  ~GGAnim();

  const std::string &getName() const { return _name; }
  std::vector<sf::IntRect> &getRects() { return _rects; }
  const std::vector<sf::IntRect> &getRects() const { return _rects; }
  std::vector<sf::IntRect> &getSourceRects() { return _sourceRects; }
  const std::vector<sf::IntRect> &getSourceRects() const { return _sourceRects; }
  int getFps() const { return _fps; }
  void setFps(int fps) { _fps = fps; }
  size_t getIndex() const { return _index; }

  void update(const sf::Time &elapsed);
  void draw(sf::RenderWindow &window) const;

  sf::Sprite &getSprite() { return _sprite; }

private:
  sf::Sprite _sprite;
  std::string _name;
  std::vector<sf::IntRect> _rects;
  std::vector<sf::IntRect> _sourceRects;
  int _fps;
  sf::Time _time;
  size_t _index;
};
} // namespace gg
