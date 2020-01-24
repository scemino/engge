#pragma once
#include <vector>
#include "SFML/Graphics.hpp"
#include "System/NonCopyable.h"

namespace ng
{
class Object;
class AnimationFrame;

enum class AnimState
{
  Pause,
  Play
};

class Animation : public sf::Drawable
{
public:
  Animation();
  explicit Animation(const sf::Texture &texture, std::string name);
  ~Animation() override;

  void setTexture(const sf::Texture* pTexture) { _pTexture = pTexture; }

  void setName(const std::string &name) { _name = name; }
  const std::string &getName() const { return _name; }

  void setColor(const sf::Color& color) { _color = color; }
  sf::Color getColor() const { return _color; }

  void addFrame(AnimationFrame&& frame);
  size_t getSize() const noexcept;
  bool empty() const noexcept;

  void setFps(int fps) { _fps = fps; }

  void update(const sf::Time &elapsed);

  void reset();
  void play(bool loop = false);
  void pause() { _state = AnimState::Pause; }
  bool isPlaying() const { return _state == AnimState::Play; }

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
  const sf::Texture* _pTexture{nullptr};
  std::string _name;
  std::vector<AnimationFrame> _frames;
  int _fps{10};
  sf::Time _time;
  size_t _index{0};
  AnimState _state{AnimState::Pause};
  bool _loop{false};
  sf::Color _color{sf::Color::White};
};
} // namespace ng
