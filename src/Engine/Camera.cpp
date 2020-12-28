#include <algorithm>
#include "engge/Engine/Camera.hpp"
#include "engge/Engine/Engine.hpp"
#include "engge/Room/Room.hpp"

namespace ng {
struct Camera::Impl {
  Engine *_pEngine{nullptr};
  glm::vec2 _at;
  std::optional<ngf::irect> _bounds;
  bool _isMoving{false};
  glm::vec2 _init, _target;
  ngf::TimeSpan _elapsed, _time;
  std::function<float(float)> _function = InterpolationHelper::getInterpolationMethod(InterpolationMethod::Linear);

  void clampCamera(glm::vec2 &at);
};

void Camera::Impl::clampCamera(glm::vec2 &at) {
  if (at.x < 0)
    at.x = 0;
  if (at.y < 0)
    at.y = 0;

  auto pRoom = _pEngine->getRoom();
  if (!pRoom)
    return;

  if (_bounds) {
    at.x = std::clamp<int>(at.x, _bounds->getTopLeft().x, _bounds->getTopRight().x);
    at.y = std::clamp<int>(at.y, _bounds->getTopLeft().y, _bounds->getBottomLeft().y);
  }

  auto roomSize = pRoom->getRoomSize();
  auto screenSize = pRoom->getScreenSize();
  at.x = std::clamp<int>(at.x, 0, std::max(roomSize.x - screenSize.x, 0));
  at.y = std::clamp<int>(at.y, 0, std::max(roomSize.y - screenSize.y, 0));
}

Camera::Camera() : _pImpl(std::make_unique<Impl>()) {}

Camera::~Camera() = default;

void Camera::setEngine(Engine *pEngine) { _pImpl->_pEngine = pEngine; }

void Camera::at(const glm::vec2 &at) {
  _pImpl->_at = at;
  _pImpl->clampCamera(_pImpl->_at);
  _pImpl->_target = _pImpl->_at;
  _pImpl->_time = ngf::TimeSpan::seconds(0);
  _pImpl->_isMoving = false;
}

glm::vec2 Camera::getAt() const { return _pImpl->_at; }

void Camera::move(const glm::vec2 &offset) {
  _pImpl->_at += offset;
  _pImpl->clampCamera(_pImpl->_at);
  _pImpl->_target = _pImpl->_at;
  _pImpl->_isMoving = false;
}

void Camera::setBounds(const ngf::irect &cameraBounds) {
  _pImpl->_bounds = cameraBounds;
  _pImpl->clampCamera(_pImpl->_at);
}

std::optional<ngf::irect> Camera::getBounds() const { return _pImpl->_bounds; }

void Camera::resetBounds() { _pImpl->_bounds = std::nullopt; }

void Camera::panTo(glm::vec2 target, ngf::TimeSpan time, InterpolationMethod interpolation) {
  if (!_pImpl->_isMoving) {
    _pImpl->_isMoving = true;
    _pImpl->_init = _pImpl->_at;
    _pImpl->_elapsed = ngf::TimeSpan::seconds(0);
  }
  _pImpl->_function = InterpolationHelper::getInterpolationMethod(interpolation);
  _pImpl->_target = target;
  _pImpl->_time = time;
}

void Camera::update(const ngf::TimeSpan &elapsed) {
  _pImpl->_elapsed += elapsed;
  auto isMoving = _pImpl->_elapsed < _pImpl->_time;

  if (_pImpl->_isMoving && !isMoving) {
    _pImpl->_isMoving = false;
    at(_pImpl->_target);
  }
  if (!isMoving)
    return;

  auto t = _pImpl->_elapsed.getTotalSeconds() / _pImpl->_time.getTotalSeconds();
  auto d = _pImpl->_target - _pImpl->_init;
  auto pos = _pImpl->_init + _pImpl->_function(t) * d;

  _pImpl->clampCamera(pos);
  _pImpl->_at = pos;
}

bool Camera::isMoving() const { return _pImpl->_isMoving; }

} // namespace ng
