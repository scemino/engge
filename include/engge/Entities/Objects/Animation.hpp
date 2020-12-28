#pragma once
#include <vector>
#include <ngf/Graphics/Drawable.h>
#include <ngf/Graphics/Color.h>
#include "engge/System/NonCopyable.hpp"
#include "AnimationFrame.hpp"

namespace ng {
class Object;
class AnimationFrame;

enum class AnimState {
  Pause,
  Play
};

class Animation : public ngf::Drawable {
public:
  Animation();
  explicit Animation(std::string texture, std::string name);
  ~Animation() override;

  void setName(const std::string &name) { _name = name; }
  [[nodiscard]] const std::string &getName() const { return _name; }

  void setColor(const ngf::Color &color) { _color = color; }
  [[nodiscard]] ngf::Color getColor() const { return _color; }

  void addFrame(AnimationFrame &&frame);
  AnimationFrame &at(size_t index);

  [[nodiscard]] size_t size() const noexcept;
  [[nodiscard]] bool empty() const noexcept;
  [[nodiscard]] size_t getIndex() const { return _index; }
  void setIndex(size_t index) { _index = index; }

  void setFps(int fps) { _fps = fps; }

  void setLeftDirection(bool leftDirection) { _leftDirection = leftDirection; }

  void update(const ngf::TimeSpan &elapsed);

  void reset();
  void play(bool loop = false);
  void pause() { _state = AnimState::Pause; }
  [[nodiscard]] bool isPlaying() const { return _state == AnimState::Play; }

  [[nodiscard]] bool contains(const glm::vec2 &pos) const;

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const override;

private:
  std::string _texture;
  std::string _name;
  std::vector<AnimationFrame> _frames;
  int _fps{10};
  ngf::TimeSpan _time;
  size_t _index{0};
  AnimState _state{AnimState::Pause};
  bool _loop{false};
  ngf::Color _color{ngf::Colors::White};
  bool _leftDirection{false};
};
} // namespace ng
