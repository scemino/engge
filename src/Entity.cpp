#include <utility>

#include "Entity.h"
#include "Room.h"
#include "Trigger.h"

namespace ng
{
void Entity::update(const sf::Time &elapsed)
{
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

void Entity::setOffset(const sf::Vector2f &offset)
{
    _offset = offset;
}

sf::Vector2f Entity::getOffset() const
{
    return _offset;
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
    auto trigger = std::make_unique<SoundTrigger>(engine, sounds);
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

} // namespace ng