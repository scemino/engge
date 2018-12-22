#pragma once
#include <sstream>
#include "SFML/Graphics.hpp"

namespace ng
{
class Layer
{
public:
  Layer();
  ~Layer();

  std::vector<sf::IntRect> &getFrames() { return _frames; }
  std::vector<sf::IntRect> &getSourceFrames() { return _sourceFrames; }
  std::vector<sf::Vector2i> &getSizes() { return _sizes; }
  std::vector<sf::Vector2i> &getOffsets() { return _offsets; }

  void setName(const std::string &name) { _name = name; }
  const std::string &getName() const { return _name; }
  int getFps() const { return _fps; }
  void setFps(int fps) { _fps = fps; }
  int getFlags() const { return _flags; }
  void setFlags(int flags) { _flags = flags; }
  int getIndex() const { return _index; }
  void setVisible(bool isVisible) { _isVisible = isVisible; }
  int getVisible() const { return _isVisible; }

  bool update(const sf::Time &elapsed);

private:
  std::string _name;
  std::vector<sf::IntRect> _frames;
  std::vector<sf::IntRect> _sourceFrames;
  std::vector<sf::Vector2i> _sizes;
  std::vector<sf::Vector2i> _offsets;
  int _fps;
  int _flags;
  sf::Time _time;
  int _index;
  bool _isVisible;
};
} // namespace ng