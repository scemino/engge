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
    sf::Vector2f getOrigin() const {return _origin; }
    void setCallback(Callback callback);
    void call();

private:
    sf::IntRect _rect;
    sf::Vector2f _origin;
    Callback _callback{nullptr};
};

} // namespace ng
