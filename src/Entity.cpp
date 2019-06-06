#include <utility>

#include "Entity.h"
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

sf::Vector2f Entity::getDefaultPosition() const
{
    return getPosition();
}

sf::Vector2f Entity::getUsePosition() const
{
    return _usePos;
}

void Entity::setTrigger(int triggerNumber, Trigger* pTrigger)
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

SoundTrigger* Entity::createSoundTrigger(Engine &engine, const std::vector<SoundDefinition*> &sounds)
{
    auto trigger = std::make_unique<SoundTrigger>(engine, sounds);
    SoundTrigger* pTrigger = trigger.get();
    _soundTriggers.push_back(std::move(trigger));
    return pTrigger;
}
} // namespace ng