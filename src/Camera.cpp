#include "Camera.h"
#include "Engine.h"
#include "Room.h"
#include "ScriptEngine.h"

namespace ng
{
struct Camera::Impl
{
    Engine *_pEngine{nullptr};
    sf::Vector2f _at;
    std::optional<sf::IntRect> _bounds;
    bool _isMoving{false};
    sf::Vector2f _init, _target;
    sf::Time _elapsed, _time;
    std::function<float(float)> _function = ScriptEngine::getInterpolationMethod(InterpolationMethod::Linear);

    void clampCamera(sf::Vector2f &at);
};

void Camera::Impl::clampCamera(sf::Vector2f &at)
{
    if (at.x < 0)
        at.x = 0;
    if (at.y < 0)
        at.y = 0;

    if (_bounds)
    {
        if (at.x < _bounds->left)
            at.x = _bounds->left;
        if (at.x > _bounds->left + _bounds->width)
            at.x = _bounds->left + _bounds->width;
        if (at.y < _bounds->top)
            at.y = _bounds->top;
        if (at.y > _bounds->top + _bounds->height)
            at.y = _bounds->top + _bounds->height;
    }

    auto pRoom = _pEngine->getRoom();
    const auto &window = _pEngine->getWindow();
    if (!pRoom)
        return;
    auto screen = window.getView().getSize();
    const auto &size = pRoom->getRoomSize();
    if (at.x > size.x - screen.x)
        at.x = size.x - screen.x;
    if (at.y > size.y - screen.y)
        at.y = size.y - screen.y;
}

Camera::Camera() : _pImpl(std::make_unique<Impl>()) {}

Camera::~Camera() = default;

void Camera::setEngine(Engine *pEngine) { _pImpl->_pEngine = pEngine; }

void Camera::at(const sf::Vector2f &at)
{
    _pImpl->_at = at;
    _pImpl->clampCamera(_pImpl->_at);
    _pImpl->_target = _pImpl->_at;
}

sf::Vector2f Camera::getAt() const { return _pImpl->_at; }

void Camera::move(const sf::Vector2f &offset)
{
    _pImpl->_at += offset;
    _pImpl->clampCamera(_pImpl->_at);
    _pImpl->_target = _pImpl->_at;
}

void Camera::setBounds(const sf::IntRect &cameraBounds)
{
    _pImpl->_bounds = cameraBounds;
    _pImpl->clampCamera(_pImpl->_at);
}

void Camera::resetBounds() { _pImpl->_bounds = std::nullopt; }

void Camera::panTo(sf::Vector2f target, sf::Time time, InterpolationMethod interpolation)
{
    if (!_pImpl->_isMoving)
    {
        _pImpl->_isMoving = true;
        _pImpl->_init = _pImpl->_at;
        _pImpl->_elapsed = sf::seconds(0);
    }
    _pImpl->_function = ScriptEngine::getInterpolationMethod((InterpolationMethod)interpolation);
    _pImpl->_target = target;
    _pImpl->_time = time;
}

void Camera::update(const sf::Time &elapsed)
{
    _pImpl->_elapsed += elapsed;
    auto isMoving = _pImpl->_elapsed < _pImpl->_time;

    if (_pImpl->_isMoving && !isMoving)
    {
        _pImpl->_isMoving = false;
        at(_pImpl->_target);
    }
    if (!isMoving)
        return;

    auto t = _pImpl->_elapsed.asSeconds() / _pImpl->_time.asSeconds();
    auto d = _pImpl->_target - _pImpl->_init;
    auto pos = _pImpl->_init + _pImpl->_function(t) * d;

    _pImpl->clampCamera(pos);
    _pImpl->_at = pos;
}

bool Camera::isMoving() const { return _pImpl->_isMoving; }

} // namespace ng
