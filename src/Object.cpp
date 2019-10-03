#include <sstream>
#include "Animation.h"
#include "Function.h"
#include "Object.h"
#include "ScriptEngine.h"
#include "Trigger.h"

namespace ng
{
struct Object::Impl
{
    std::vector<std::unique_ptr<Animation>> _anims;
    std::optional<Animation> _pAnim{std::nullopt};
    std::wstring _name, _id;
    int _zorder{0};
    UseDirection _direction{UseDirection::Front};
    bool _prop{false};
    bool _spot{false};
    bool _trigger{false};
    sf::Vector2f _usePos;
    sf::Vector2f _defaultPosition;
    sf::IntRect _hotspot;
    bool _isTouchable{true};
    Room *_pRoom{nullptr};
    int _state{0};
    std::vector<std::shared_ptr<Trigger>> _triggers;
    HSQOBJECT _pTable{};
    bool _hotspotVisible{false};
    bool _triggerEnabled{true};
    Object *pParentObject{nullptr};
    int dependentState{0};
    std::string _icon;
    Actor* _owner{nullptr};
};

Object::Object()
    : pImpl(std::make_unique<Impl>())
{
}

Object::~Object() = default;

void Object::setZOrder(int zorder)
{
    pImpl->_zorder = zorder;
}

int Object::getZOrder() const
{
    return pImpl->_zorder;
}

void Object::setProp(bool prop)
{
    pImpl->_prop = prop;
}

void Object::setSpot(bool spot)
{
    pImpl->_spot = spot;
}

void Object::setTrigger(bool trigger)
{
    pImpl->_trigger = trigger;
}

void Object::setUseDirection(UseDirection direction) { pImpl->_direction = direction; }
UseDirection Object::getUseDirection() const { return pImpl->_direction; }

void Object::setHotspot(const sf::IntRect &hotspot) { pImpl->_hotspot = hotspot; }
const sf::IntRect &Object::getHotspot() const { return pImpl->_hotspot; }

void Object::setId(const std::wstring &id) { pImpl->_id = id; }
const std::wstring &Object::getId() const { return pImpl->_id; }

void Object::setIcon(const std::string &icon) { pImpl->_icon = icon; }
std::string Object::getIcon() const { return pImpl->_icon; }

void Object::setOwner(Actor* pActor) { pImpl->_owner = pActor; }
Actor* Object::getOwner() const { return pImpl->_owner; }

HSQOBJECT &Object::getTable() { return pImpl->_pTable; }
HSQOBJECT &Object::getTable() const { return pImpl->_pTable; }
bool Object::isInventoryObject() const { return getOwner() != nullptr; }

std::vector<std::unique_ptr<Animation>> &Object::getAnims() { return pImpl->_anims; }

Room *Object::getRoom() { return pImpl->_pRoom; }
const Room *Object::getRoom() const { return pImpl->_pRoom; }
void Object::setRoom(Room *pRoom) { pImpl->_pRoom = pRoom; }

void Object::addTrigger(const std::shared_ptr<Trigger> &trigger) { pImpl->_triggers.push_back(trigger); }
void Object::removeTrigger()
{
    for (auto &trigger : pImpl->_triggers)
    {
        trigger->disable();
    }
}

Trigger *Object::getTrigger() { return !pImpl->_triggers.empty() ? pImpl->_triggers[0].get() : nullptr; }
void Object::enableTrigger(bool enabled) { pImpl->_triggerEnabled = enabled; }

bool Object::isTouchable() const
{
    if (!isVisible())
        return false;
    if (pImpl->_trigger)
        return false;
    if (pImpl->_spot)
        return false;
    if (pImpl->_prop)
        return false;
    return pImpl->_isTouchable;
}

sf::IntRect Object::getRealHotspot() const
{
    auto rect = getHotspot();
    auto transform = getTransform();
    return (sf::IntRect)transform.transformRect((sf::FloatRect)rect);
}

bool Object::isVisible() const
{
    if(pImpl->_state == ObjectStateConstants::GONE) return false;
    return Entity::isVisible();
}

void Object::setStateAnimIndex(int animIndex)
{
    std::ostringstream s;
    s << "state" << animIndex;
    pImpl->_state = animIndex;
    if (animIndex == ObjectStateConstants::GONE)
    {
        setVisible(false);
    }
    else if (animIndex == ObjectStateConstants::HERE)
    {
        setVisible(true);
    }
    setAnimation(s.str());
}

void Object::playAnim(const std::string &anim, bool loop)
{
    setAnimation(anim);
    pImpl->_pAnim->play(loop);
}

void Object::playAnim(int animIndex, bool loop)
{
    setStateAnimIndex(animIndex);
    pImpl->_pAnim->play(loop);
}

int Object::getState()
{
    return pImpl->_state;
}

void Object::setAnimation(const std::string &name)
{
    auto it = std::find_if(pImpl->_anims.begin(), pImpl->_anims.end(), [name](std::unique_ptr<Animation> &animation) { return animation->getName() == name; });
    if (it == pImpl->_anims.end())
    {
        pImpl->_pAnim = std::nullopt;
        return;
    }

    auto &anim = *(it->get());
    pImpl->_pAnim = anim;
    pImpl->_pAnim->setObject(this);
}

std::optional<Animation> &Object::getAnimation()
{
    return pImpl->_pAnim;
}

void Object::update(const sf::Time &elapsed)
{
    Entity::update(elapsed);
    if (pImpl->pParentObject)
    {
        setVisible(pImpl->pParentObject->getState() == pImpl->dependentState);
    }
    if (pImpl->_pAnim)
    {
        pImpl->_pAnim->update(elapsed);
    }
    if (pImpl->_triggerEnabled)
    {
        for (auto &trigger : pImpl->_triggers)
        {
            trigger->trig();
        }
    }
}

void Object::showHotspot(bool show) { pImpl->_hotspotVisible = show; }

bool Object::isHotspotVisible() const { return pImpl->_hotspotVisible; }

void Object::drawHotspot(sf::RenderTarget &target, sf::RenderStates states) const
{
    if (!pImpl->_hotspotVisible)
        return;

    auto rect = getHotspot();

    sf::RectangleShape s(sf::Vector2f(rect.width, rect.height));
    s.setPosition(rect.left, rect.top);
    s.setOutlineThickness(1);
    s.setOutlineColor(sf::Color::Red);
    s.setFillColor(sf::Color::Transparent);
    target.draw(s, states);

    sf::RectangleShape vl(sf::Vector2f(1, 5));
    vl.setPosition(pImpl->_usePos.x, -pImpl->_usePos.y - 2);
    vl.setFillColor(sf::Color::Red);
    target.draw(vl, states);

    sf::RectangleShape hl(sf::Vector2f(5, 1));
    hl.setPosition(pImpl->_usePos.x - 2, -pImpl->_usePos.y);
    hl.setFillColor(sf::Color::Red);
    target.draw(hl, states);
}

void Object::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    auto transform = getTransform();
    if (!isVisible())
        return;
    states.transform *= transform;

    if (pImpl->_pAnim)
    {
        pImpl->_pAnim->getSprite().setColor(getColor());
        target.draw(*pImpl->_pAnim, states);
    }

    drawHotspot(target, states);
}

void Object::dependentOn(Object *parentObject, int state)
{
    pImpl->dependentState = state;
    pImpl->pParentObject = parentObject;
}

void Object::setFps(int fps)
{
    if (pImpl->_pAnim.has_value())
    {
        pImpl->_pAnim->setFps(fps);
    }
}

bool Object::isTrigger() const
{
    return pImpl->_trigger;
}

std::wostream &operator<<(std::wostream &os, const Object &obj)
{
    return os << obj.getName() << L" (" << obj.getRealPosition().x << L"," << obj.getRealPosition().y << L":" << obj.getZOrder() << L")";
}

} // namespace ng
