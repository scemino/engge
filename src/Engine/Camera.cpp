#include <algorithm>
#include "engge/Engine/Camera.hpp"
#include "engge/Engine/Engine.hpp"
#include "engge/Room/Room.hpp"

namespace ng {
struct Camera::Impl {
  Engine *_pEngine{nullptr};
  glm::vec2 _at{0, 0};
  std::optional<ngf::irect> _bounds;
  bool _isMoving{false};
  glm::vec2 _init{0, 0}, _target{0, 0};;
  ngf::TimeSpan _elapsed, _time;
  std::function<float(float)> _function = InterpolationHelper::getInterpolationMethod(InterpolationMethod::Linear);
  const Actor *pFollow{nullptr};

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

  auto roomSize = pRoom->getRoomSize();
  auto screenSize = pRoom->getScreenSize();

  if (_bounds) {
    at.x =
        std::clamp<int>(at.x, screenSize.x / 2 + _bounds->getTopLeft().x, screenSize.x / 2 + _bounds->getTopRight().x);
    at.y = std::clamp<int>(at.y,
                           screenSize.y / 2 + _bounds->getTopLeft().y,
                           screenSize.y / 2 + _bounds->getBottomLeft().y);
  }
  at.x = std::clamp<float>(at.x, screenSize.x / 2, std::max(roomSize.x - screenSize.x / 2, 0));
  at.y = std::clamp<float>(at.y, screenSize.y / 2, std::max(roomSize.y - screenSize.y / 2, 0));
}

Camera::Camera() : m_pImpl(std::make_unique<Impl>()) {}

Camera::~Camera() = default;

void Camera::setEngine(Engine *pEngine) { m_pImpl->_pEngine = pEngine; }

void Camera::at(const glm::vec2 &at) {
  m_pImpl->_at = at;
  m_pImpl->clampCamera(m_pImpl->_at);
  m_pImpl->_target = m_pImpl->_at;
  m_pImpl->_time = ngf::TimeSpan::seconds(0);
  m_pImpl->_isMoving = false;
}

ngf::frect Camera::getRect() const {
  auto pRoom = m_pImpl->_pEngine->getRoom();
  auto screenSize = pRoom->getScreenSize();
  return ngf::frect::fromCenterSize(m_pImpl->_at, screenSize);
}

glm::vec2 Camera::getAt() const { return m_pImpl->_at; }

void Camera::move(const glm::vec2 &offset) {
  m_pImpl->_at += offset;
  m_pImpl->clampCamera(m_pImpl->_at);
  m_pImpl->_target = m_pImpl->_at;
  m_pImpl->_isMoving = false;
}

void Camera::setBounds(const ngf::irect &cameraBounds) {
  m_pImpl->_bounds = cameraBounds;
  m_pImpl->clampCamera(m_pImpl->_at);
}

std::optional<ngf::irect> Camera::getBounds() const { return m_pImpl->_bounds; }

void Camera::resetBounds() { m_pImpl->_bounds = std::nullopt; }

void Camera::panTo(glm::vec2 target, ngf::TimeSpan time, InterpolationMethod interpolation) {
  if (!m_pImpl->_isMoving) {
    m_pImpl->_isMoving = true;
    m_pImpl->_init = m_pImpl->_at;
    m_pImpl->_elapsed = ngf::TimeSpan::seconds(0);
  }
  m_pImpl->_function = InterpolationHelper::getInterpolationMethod(interpolation);
  m_pImpl->_target = target;
  m_pImpl->_time = time;
}

void Camera::update(const ngf::TimeSpan &elapsed) {
  m_pImpl->_elapsed += elapsed;
  auto isMoving = m_pImpl->_elapsed < m_pImpl->_time;

  if (m_pImpl->_isMoving && !isMoving) {
    m_pImpl->_isMoving = false;
    m_pImpl->_time = ngf::TimeSpan::seconds(0);
    at(m_pImpl->_target);
  }

  if (isMoving) {
    auto t = m_pImpl->_elapsed.getTotalSeconds() / m_pImpl->_time.getTotalSeconds();
    auto d = m_pImpl->_target - m_pImpl->_init;
    auto pos = m_pImpl->_init + m_pImpl->_function(t) * d;

    m_pImpl->clampCamera(pos);
    m_pImpl->_at = pos;
    return;
  }

  const auto *pFollowActor = m_pImpl->_pEngine->getFollowActor();
  const auto *pRoom = m_pImpl->_pEngine->getRoom();
  if (pFollowActor && pFollowActor->isVisible() && pFollowActor->getRoom() == pRoom) {
    const auto screen = pRoom->getScreenSize();
    const auto pos = pFollowActor->getPosition();
    const auto margin = glm::vec2(screen.x / 4, screen.y / 4);
    const auto cameraPos = getAt();

    const auto d = pos - cameraPos;
    const auto delta = d * elapsed.getTotalSeconds();
    const auto sameActor = m_pImpl->pFollow == pFollowActor;

    float x, y;
    if (sameActor && (pos.x > (cameraPos.x + margin.x))) {
      x = pos.x - margin.x;
    } else if (sameActor && (pos.x < (cameraPos.x - margin.x))) {
      x = pos.x + margin.x;
    } else {
      x = cameraPos.x + (d.x > 0 ? std::min(delta.x, d.x) : std::max(delta.x, d.x));
    }
    if (sameActor && (pos.y > (cameraPos.y + margin.y))) {
      y = pos.y - margin.y;
    } else if (sameActor && (pos.y < (cameraPos.y - margin.y))) {
      y = pos.y + margin.y;
    } else {
      y = cameraPos.y + (d.y > 0 ? std::min(delta.y, d.y) : std::max(delta.y, d.y));
    }
    at({x, y});
    if (!sameActor && std::abs(pos.x - x) < 1 && std::abs(pos.y - y) < 1) {
      m_pImpl->pFollow = pFollowActor;
    }
  }
}

bool Camera::isMoving() const { return m_pImpl->_isMoving; }

} // namespace ng
