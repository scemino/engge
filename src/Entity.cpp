#include <utility>
#include "Entity.h"
#include "Room.h"
#include "ScriptEngine.h"
#include "Trigger.h"

namespace ng
{
void Entity::update(const sf::Time &elapsed)
{
    for (auto &&f : _functions)
    {
        (*f)(elapsed);
    }
    _functions.erase(std::remove_if(_functions.begin(), _functions.end(),
        [](std::unique_ptr<Function>& x){return x->isElapsed();}),_functions.end());
}

void Entity::setLit(bool isLit)
{
    _isLit = isLit;
}

bool Entity::isLit() const
{
    return _isLit;
}

void Entity::setVisible(bool isVisible)
{
    _isVisible = isVisible;
}

bool Entity::isVisible() const
{
    return _isVisible;
}

void Entity::setUsePosition(const sf::Vector2f &pos)
{
    _usePos = pos;
}

void Entity::setPosition(const sf::Vector2f &pos)
{
    _transform.setPosition(pos);
}

sf::Vector2f Entity::getPosition() const
{
    return _transform.getPosition();
}

sf::Vector2f Entity::getRealPosition() const
{
    return getPosition() + _offset;
}

void Entity::setOffset(const sf::Vector2f &offset)
{
    _offset = offset;
}

sf::Vector2f Entity::getOffset() const
{
    return _offset;
}

void Entity::setRotation(float angle) { _transform.setRotation(angle); }
float Entity::getRotation() const 
{
    // SFML give rotation in degree between [0, 360]
    float angle = _transform.getRotation();
    // convert it to [-180, 180]
    if(angle > 180) angle -= 360;
    return angle;
}

void Entity::setColor(const sf::Color& color)
{
    _color = color;
}

const sf::Color& Entity::getColor() const
{
    return _color;
}

void Entity::setScale(float s)
{
    _transform.setScale(s, s);
}

float Entity::getScale() const
{
    return _transform.getScale().x;
}

sf::Transform Entity::getTransform() const
{
    auto transform = _transform;
    transform.move(_offset.x, _offset.y);
    return transform.getTransform();
}

sf::Vector2f Entity::getUsePosition() const
{
    return _usePos;
}

void Entity::setTrigger(int triggerNumber, Trigger *pTrigger)
{
    _triggers[triggerNumber] = pTrigger;
}

void Entity::trig(int triggerNumber)
{
    auto it = _triggers.find(triggerNumber);
    if (it != _triggers.end())
    {
        it->second->trig();
    }
}

void Entity::trigSound(const std::string &name)
{
}

void Entity::drawForeground(sf::RenderTarget &target, sf::RenderStates states) const
{
}

SoundTrigger *Entity::createSoundTrigger(Engine &engine, const std::vector<SoundDefinition *> &sounds)
{
    auto trigger = std::make_unique<SoundTrigger>(engine, sounds, this);
    SoundTrigger *pTrigger = trigger.get();
    _soundTriggers.push_back(std::move(trigger));
    return pTrigger;
}

void Entity::setTouchable(bool isTouchable)
{
    _isTouchable = isTouchable;
}

bool Entity::isTouchable() const
{
    if (!isVisible())
        return false;
    return _isTouchable;
}

void Entity::setRenderOffset(const sf::Vector2i &offset)
{
    _renderOffset = offset;
}

sf::Vector2i Entity::getRenderOffset() const
{
    return _renderOffset;
}

void Entity::alphaTo(float destination, sf::Time time, InterpolationMethod method)
{
    auto m = ScriptEngine::getInterpolationMethod(method);
    auto getAlpha = [](const Entity &o) { return (o.getColor().a / 255.f); };
    auto setAlpha = [](Entity &o, float a) {
        const auto c = o.getColor();
        return o.setColor(sf::Color(c.r, c.g, c.b, (sf::Uint8)(a * 255.f)));
    };
    auto getalpha = std::bind(getAlpha, std::cref(*this));
    auto setalpha = std::bind(setAlpha, std::ref(*this), std::placeholders::_1);
    auto alphaTo = std::make_unique<ChangeProperty<float>>(getalpha, setalpha, destination, time, m);
    _functions.push_back(std::move(alphaTo));
}

void Entity::offsetTo(sf::Vector2f destination, sf::Time time, InterpolationMethod method)
{
    auto m = ScriptEngine::getInterpolationMethod(method);
    auto get = std::bind(&Entity::getOffset, this);
    auto set = std::bind(&Entity::setOffset, this, std::placeholders::_1);
    auto offsetTo = std::make_unique<ChangeProperty<sf::Vector2f>>(get, set, destination, time, m);
    _functions.push_back(std::move(offsetTo));
}

void Entity::moveTo(sf::Vector2f destination, sf::Time time, InterpolationMethod method)
{
    auto m = ScriptEngine::getInterpolationMethod(method);
    auto get = std::bind(&Entity::getPosition, this);
    auto set = std::bind(&Entity::setPosition, this, std::placeholders::_1);
    auto offsetTo = std::make_unique<ChangeProperty<sf::Vector2f>>(get, set, destination, time, m, 
        method == InterpolationMethod::Looping);
    _functions.push_back(std::move(offsetTo));
}

void Entity::rotateTo(float destination, sf::Time time, InterpolationMethod method)
{
    auto m = ScriptEngine::getInterpolationMethod(method);
    auto get = std::bind(&Entity::getRotation, this);
    auto set = std::bind(&Entity::setRotation, this, std::placeholders::_1);
    auto rotateTo =
        std::make_unique<ChangeProperty<float>>(get, set, destination, time, m,
                                                method == InterpolationMethod::Looping);
    _functions.push_back(std::move(rotateTo));
}

void Entity::scaleTo(float destination, sf::Time time, InterpolationMethod method)
{
    auto m = ScriptEngine::getInterpolationMethod(method);
    auto get = std::bind(&Entity::getScale, this);
    auto set = std::bind(&Entity::setScale, this, std::placeholders::_1);
    auto scalteTo = std::make_unique<ChangeProperty<float>>(get, set, destination, time, m,
        method == InterpolationMethod::Looping);
    _functions.push_back(std::move(scalteTo));
}

} // namespace ng