#pragma once
#include <sstream>
#include "SFML/Graphics.hpp"

namespace gg
{
class GGLayer
{
public:
  GGLayer();
  ~GGLayer();

  std::vector<sf::IntRect> &getFrames() { return _frames; }

  std::vector<sf::IntRect> &getSourceFrames() { return _sourceFrames; }

  int getFps() const { return _fps; }
  void setFps(int fps) { _fps = fps; }
  int getFlags() const { return _flags; }
  void setFlags(int flags) { _flags = flags; }
  int getIndex() const { return _index; }
  void update(const sf::Time &elapsed);

private:
  std::vector<sf::IntRect> _frames;
  std::vector<sf::IntRect> _sourceFrames;
  int _fps;
  int _flags;
  sf::Time _time;
  int _index;
};
} // namespace gg