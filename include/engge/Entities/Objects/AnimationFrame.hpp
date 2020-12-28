#pragma once
#include <functional>
#include <string>
#include <glm/vec2.hpp>
#include <ngf/Graphics/Rect.h>

namespace ng {

class AnimationFrame {
public:
  using Callback = std::function<void()>;

public:
  explicit AnimationFrame(ngf::irect rect, Callback callback = nullptr);

  void setName(const std::string &name);
  [[nodiscard]] const std::string &getName() const;

  void setRect(ngf::irect rect);
  [[nodiscard]] ngf::irect getRect() const;
  [[nodiscard]] glm::vec2 getOrigin(bool leftDirection) const;

  void setSourceRect(ngf::irect rect) { _sourceRect = rect; }
  [[nodiscard]] ngf::irect getSourceRect() const { return _sourceRect; }

  void setOffset(glm::vec2 offset) { _offset = offset; }
  [[nodiscard]] glm::vec2 getOffset(bool leftDirection) const;

  void setSize(glm::ivec2 size) { _size = size; }
  [[nodiscard]] glm::ivec2 getSize() const { return _size; }

  void setCallback(Callback callback);
  void call();

private:
  std::string _name;
  ngf::irect _rect;
  ngf::irect _sourceRect;
  glm::ivec2 _size{0, 0};
  glm::vec2 _offset{0, 0};
  Callback _callback{nullptr};
};

} // namespace ng
