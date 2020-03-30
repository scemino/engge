#pragma once
#include <vector>
#include "SFML/Graphics.hpp"
#include "System/NonCopyable.hpp"
#include "Entities/Objects/AnimationFrame.hpp"

namespace ng {
class Object;
class AnimationFrame;

enum class AnimState {
  Pause,
  Play
};

class Animation : public sf::Drawable {
public:
  Animation();
  explicit Animation(const sf::Texture &texture, std::string name);
  ~Animation() override;

  void setName(const std::string &name) { _name = name; }
  [[nodiscard]] const std::string &getName() const { return _name; }

  void setColor(const sf::Color &color) { _color = color; }
  [[nodiscard]] sf::Color getColor() const { return _color; }

  void addFrame(AnimationFrame &&frame);
  AnimationFrame &at(size_t index);

  [[nodiscard]] size_t size() const noexcept;
  [[nodiscard]] bool empty() const noexcept;
  [[nodiscard]] size_t getIndex() const { return _index; }
  void setIndex(size_t index) { _index = index; }

  void setFps(int fps) { _fps = fps; }

  void setLeftDirection(bool leftDirection) { _leftDirection = leftDirection; }

  void update(const sf::Time &elapsed);

  void reset();
  void play(bool loop = false);
  void pause() { _state = AnimState::Pause; }
  [[nodiscard]] bool isPlaying() const { return _state == AnimState::Play; }

  [[nodiscard]] bool contains(const sf::Vector2f &pos) const;

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
  const sf::Texture *_pTexture{nullptr};
  std::string _name;
  std::vector<AnimationFrame> _frames;
  int _fps{10};
  sf::Time _time;
  size_t _index{0};
  AnimState _state{AnimState::Pause};
  bool _loop{false};
  sf::Color _color{sf::Color::White};
  bool _leftDirection{false};
};
} // namespace ng
