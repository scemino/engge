#pragma once
#include <functional>
#include "SFML/Graphics.hpp"

namespace ng
{

class AnimationFrame
{
public:
    using Callback = std::function<void()>;

public:
    AnimationFrame(sf::IntRect rect, sf::Vector2f origin, Callback callback = nullptr);

    sf::IntRect getRect() const {return _rect; }
    void setOrigin(sf::Vector2f origin) { _origin = origin; }
    sf::Vector2f getOrigin() const {return _origin; }
    void setOffset(sf::Vector2f offset) { _offset = offset; }
    sf::Vector2f getOffset() const {return _offset; }

    void setCallback(Callback callback);
    void call();

private:
    sf::IntRect _rect;
    sf::Vector2f _origin;
    sf::Vector2f _offset;
    Callback _callback{nullptr};
};

} // namespace ng
