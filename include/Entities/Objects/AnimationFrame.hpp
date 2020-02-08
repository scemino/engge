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
    AnimationFrame(sf::IntRect rect, Callback callback = nullptr);

    sf::IntRect getRect(bool leftDirection) const;
    sf::Vector2f getOrigin(bool leftDirection) const;
    
    void setSourceRect(sf::IntRect rect) {_sourceRect=rect; }
    sf::IntRect getSourceRect() const {return _sourceRect; }
    
    void setOffset(sf::Vector2f offset) { _offset = offset; }
    sf::Vector2f getOffset(bool leftDirection) const;

    void setSize(sf::Vector2i size) { _size = size; }
    sf::Vector2i getSize() const {return _size; }

    void setCallback(Callback callback);
    void call();

private:
    sf::IntRect _rect;
    sf::IntRect _sourceRect;
    sf::Vector2i _size;
    sf::Vector2f _offset;
    Callback _callback{nullptr};
};

} // namespace ng
