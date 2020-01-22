#pragma once
#include <optional>
#include "SFML/Graphics.hpp"
#include "System/NonCopyable.h"

namespace ng
{
class Object;

enum class AnimState
{
  Pause,
  Play
};

class Animation : public sf::Drawable
{
public:
  explicit Animation(const sf::Texture &texture, std::string name);
  ~Animation() override;

  void setName(const std::string &name) { _name = name; }
  const std::string &getName() const { return _name; }

  std::vector<sf::IntRect> &getRects() { return _rects; }
  std::vector<sf::Vector2i> &getSizes() { return _sizes; }
  std::vector<sf::IntRect> &getSourceRects() { return _sourceRects; }
  std::vector<std::optional<int>> &getTriggers() { return _triggers; }

  void setFps(int fps) { _fps = fps; }

  void update(const sf::Time &elapsed);

  void reset();
  void play(bool loop = false);
  void pause() { _state = AnimState::Pause; }
  bool isPlaying() const { return _state == AnimState::Play; }

  sf::Sprite &getSprite() { return _sprite; }
  const sf::Sprite &getSprite() const { return _sprite; }

  void setObject(Object *pObject) { _pObject = pObject; }

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
  void updateTrigger();

private:
  sf::Sprite _sprite;
  std::string _name;
  std::vector<sf::IntRect> _rects;
  std::vector<sf::Vector2i> _sizes;
  std::vector<sf::IntRect> _sourceRects;
  std::vector<std::optional<int>> _triggers;
  int _fps;
  sf::Time _time;
  size_t _index;
  AnimState _state;
  bool _loop{false};
  Object *_pObject{nullptr};
};
} // namespace ng
