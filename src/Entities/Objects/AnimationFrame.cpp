#include "engge/Entities/Objects/AnimationFrame.hpp"

namespace ng {
AnimationFrame::AnimationFrame(ngf::irect rect, std::function<void()> callback)
    : _rect(rect), _callback(std::move(callback)) {
}

void AnimationFrame::setCallback(Callback callback) {
  _callback = std::move(callback);
}

void AnimationFrame::call() {
  if (_callback) {
    _callback();
  }
}

void AnimationFrame::setRect(ngf::irect rect) {
  _rect = rect;
}

ngf::irect AnimationFrame::getRect() const {
  return _rect;
}

glm::vec2 AnimationFrame::getOffset(bool leftDirection) const {
  auto offset = -_offset;
  if (!leftDirection) {
    offset.x = -offset.x;
  }
  return offset;
}

glm::vec2 AnimationFrame::getOrigin(bool leftDirection) const {
  auto y = static_cast<int>((_size.y + 1) / 2 - _sourceRect.getTopLeft().y);
  auto x =
      static_cast<int>(leftDirection ? _sourceRect.getTopLeft().x + _size.x / 2.f : _size.x / 2.f
          - _sourceRect.getTopLeft().x);
  return glm::vec2(static_cast<float>(x), static_cast<float>(y));
}

void AnimationFrame::setName(const std::string &name) { _name = name; }

const std::string &AnimationFrame::getName() const { return _name; }

}
