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

glm::vec2 AnimationFrame::getOrigin() const {
  return glm::vec2(static_cast<float>(_size.x / 2.f), static_cast<float>(_size.y / 2.f));
}

glm::vec2 AnimationFrame::getPosition(bool leftDirection) const {
  auto y = static_cast<int>(_sourceRect.getTopLeft().y);
  auto x = static_cast<int>(leftDirection ? _size.x - _sourceRect.getTopLeft().x : _sourceRect.getTopLeft().x);
  return glm::vec2(static_cast<float>(x), static_cast<float>(y));
}

void AnimationFrame::setName(const std::string &name) { _name = name; }

const std::string &AnimationFrame::getName() const { return _name; }

}
