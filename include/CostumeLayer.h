#pragma once
#include <sstream>
#include <optional>
#include "SFML/Graphics.hpp"

namespace ng
{
class Actor;

class CostumeLayer : public sf::Drawable
{
public:
  CostumeLayer();
  ~CostumeLayer();

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
  void reset() { _index=0; }
  void setVisible(bool isVisible) { _isVisible = isVisible; }
  int getVisible() const { return _isVisible; }
  std::vector<std::optional<int>> &getTriggers() { return _triggers; }
  std::vector<std::optional<std::string>> &getSoundTriggers() { return _soundTriggers; }
  void setActor(Actor *pActor) { _pActor = pActor; }
  void setLoop(bool loop) { _loop = loop; }
  void setTexture(const sf::Texture *pTexture) { _pTexture = pTexture; }
  void setLeftDirection(bool leftDirection) { _leftDirection = leftDirection; }
  bool contains(const sf::Vector2f& pos) const;

  bool update(const sf::Time &elapsed);

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
  void updateTrigger();
  void updateSoundTrigger();

private:
  std::string _name;
  std::vector<sf::IntRect> _frames;
  std::vector<sf::IntRect> _sourceFrames;
  std::vector<sf::Vector2i> _sizes;
  std::vector<sf::Vector2i> _offsets;
  std::vector<std::optional<int>> _triggers;
  std::vector<std::optional<std::string>> _soundTriggers;
  const sf::Texture *_pTexture{nullptr};
  int _fps{10};
  int _flags{0};
  sf::Time _time;
  int _index{0};
  bool _isVisible{true};
  Actor *_pActor{nullptr};
  bool _loop{false};
  bool _leftDirection{false};
};
} // namespace ng