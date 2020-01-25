#include "Entities/Objects/AnimationFrame.hpp"

namespace ng
{
AnimationFrame::AnimationFrame(sf::IntRect rect, sf::Vector2f origin, std::function<void()> callback)
: _rect(rect), _origin(origin), _callback(std::move(callback))
{
}

void AnimationFrame::setCallback(Callback callback)
{
    _callback = std::move(callback);
}

void AnimationFrame::call()
{ 
    if(_callback) {
        _callback();
    } 
}

}
