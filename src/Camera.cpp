#include "Camera.h"
#include "Engine.h"
#include "Room.h"

namespace ng
{
struct Camera::Impl
{
    Engine *_pEngine{nullptr};
    sf::Vector2f _at;
    std::optional<sf::IntRect> _bounds;

    void clampCamera();
};

void Camera::Impl::clampCamera()
{
    if (_at.x < 0)
        _at.x = 0;
    if (_at.y < 0)
        _at.y = 0;

    if (_bounds)
    {
        if (_at.x < _bounds->left)
            _at.x = _bounds->left;
        if (_at.x > _bounds->left + _bounds->width)
            _at.x = _bounds->left + _bounds->width;
        if (_at.y < _bounds->top)
            _at.y = _bounds->top;
        if (_at.y > _bounds->top + _bounds->height)
            _at.y = _bounds->top + _bounds->height;
    }

    auto pRoom = _pEngine->getRoom();
    const auto &window = _pEngine->getWindow();
    if (!pRoom)
        return;
    auto screen = window.getView().getSize();
    const auto &size = pRoom->getRoomSize();
    if (_at.x > size.x - screen.x)
        _at.x = size.x - screen.x;
    if (_at.y > size.y - screen.y)
        _at.y = size.y - screen.y;
}

Camera::Camera() : _pImpl(std::make_unique<Impl>()) {}

Camera::~Camera() = default;

void Camera::setEngine(Engine *pEngine) { _pImpl->_pEngine = pEngine; }

void Camera::at(const sf::Vector2f &at)
{
    _pImpl->_at = at;
    _pImpl->clampCamera();
}

sf::Vector2f Camera::getAt() const { return _pImpl->_at; }

void Camera::move(const sf::Vector2f &offset)
{
    _pImpl->_at += offset;
    _pImpl->clampCamera();
}

void Camera::setBounds(const sf::IntRect &cameraBounds)
{
    _pImpl->_bounds = cameraBounds;
    _pImpl->clampCamera();
}

void Camera::resetBounds() { _pImpl->_bounds = std::nullopt; }

} // namespace ng
