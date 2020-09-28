#include "engge/Entities/Objects/AnimationFrame.hpp"

namespace ng {
AnimationFrame::AnimationFrame(sf::IntRect rect, std::function<void()> callback)
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

void AnimationFrame::setRect(sf::IntRect rect) {
  _rect = rect;
}

sf::IntRect AnimationFrame::getRect(bool leftDirection) const {
  sf::IntRect rect = _rect;
  if (leftDirection) {
    rect.left += rect.width;
    rect.width = -rect.width;
  }
  return rect;
}

sf::Vector2f AnimationFrame::getOffset(bool leftDirection) const {
  auto offset = -_offset;
  if (!leftDirection) {
    offset.x = -offset.x;
  }
  return offset;
}

sf::Vector2f AnimationFrame::getOrigin(bool leftDirection) const {
  auto y = static_cast<int>((_size.y + 1) / 2 - _sourceRect.top);
  auto x =
      static_cast<int>(leftDirection ? _sourceRect.left + _size.x / 2.f + _sourceRect.width - _size.x : _size.x / 2.f
          - _sourceRect.left);
  return sf::Vector2f(static_cast<float>(x), static_cast<float>(y));
}

void AnimationFrame::setName(const std::string &name) { _name = name; }

const std::string &AnimationFrame::getName() const { return _name; }

}
