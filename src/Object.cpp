#include <sstream>
#include "Animation.h"
#include "Object.h"
#include "Trigger.h"

namespace ng
{
struct Object::Impl
{
    std::vector<std::unique_ptr<Animation>> _anims;
    std::optional<Animation> _pAnim;
    std::wstring _name, _id;
    int _zorder;
    UseDirection _direction;
    bool _prop;
    bool _spot;
    bool _trigger{false};
    sf::Vector2f _usePos;
    sf::Vector2f _defaultPosition;
    sf::Color _color;
    sf::IntRect _hotspot;
    float _angle;
    bool _isTouchable;
    Room *_pRoom;
    int _state;
    int _verb;
    std::vector<std::shared_ptr<Trigger>> _triggers;
    HSQOBJECT _pTable;
    bool _hotspotVisible;
    bool _triggerEnabled{true};
    Object *pParentObject{nullptr};
    int dependentState{0};

    Impl()
        : _pAnim(std::nullopt),
          _zorder(0),
          _direction(UseDirection::Front),
          _prop(false),
          _spot(false),
          _color(sf::Color::White),
          _angle(0),
          _isTouchable(true),
          _pRoom(nullptr),
          _state(0),
          _verb(1),
          _hotspotVisible(false)
    {
    }
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

void Object::setTouchable(bool isTouchable) { pImpl->_isTouchable = isTouchable; }

void Object::setUseDirection(UseDirection direction) { pImpl->_direction = direction; }
UseDirection Object::getUseDirection() const { return pImpl->_direction; }

void Object::setHotspot(const sf::IntRect &hotspot) { pImpl->_hotspot = hotspot; }
const sf::IntRect &Object::getHotspot() const { return pImpl->_hotspot; }

void Object::setName(const std::wstring &name) { pImpl->_name = name; }
const std::wstring &Object::getName() const { return pImpl->_name; }

void Object::setId(const std::wstring &id) { pImpl->_id = id; }
const std::wstring &Object::getId() const { return pImpl->_id; }

void Object::setDefaultVerb(int verb) { pImpl->_verb = verb; }
int Object::getDefaultVerb() const { return pImpl->_verb; }

HSQOBJECT &Object::getTable() { return pImpl->_pTable; }

std::vector<std::unique_ptr<Animation>> &Object::getAnims() { return pImpl->_anims; }

void Object::setRotation(float angle) { _transform.setRotation(angle); }
float Object::getRotation() const { return _transform.getRotation(); }

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

void Object::setDefaultPosition(const sf::Vector2f &pos)
{
    pImpl->_defaultPosition = pos;
    setPosition(pos);
}

sf::Vector2f Object::getDefaultPosition() const
{
    return pImpl->_defaultPosition;
}

sf::IntRect Object::getRealHotspot() const
{
    auto rect = getHotspot();
    return (sf::IntRect)_transform.getTransform().transformRect((sf::FloatRect)rect);
}

void Object::setStateAnimIndex(int animIndex)
{
    std::ostringstream s;
    s << "state" << animIndex;
    pImpl->_state = animIndex;
    if (animIndex == 4)
    {
        setVisible(false);
    }
    else if (animIndex == 0)
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
    auto &sprite = pImpl->_pAnim->getSprite();
    sprite.setColor(pImpl->_color);
}

std::optional<Animation> &Object::getAnimation()
{
    return pImpl->_pAnim;
}

void Object::move(const sf::Vector2f &offset)
{
    _transform.move(offset);
}

void Object::setColor(const sf::Color &color)
{
    pImpl->_color = color;
    if (pImpl->_pAnim)
    {
        pImpl->_pAnim->getSprite().setColor(color);
    }
}

const sf::Color &Object::getColor() const
{
    return pImpl->_color;
}

void Object::setScale(float s)
{
    _transform.setScale(s, s);
}

void Object::update(const sf::Time &elapsed)
{
    if (pImpl->pParentObject)
    {
        if (pImpl->pParentObject->getState() == pImpl->dependentState)
        {
            if (pImpl->_state == 4)
            {
                setStateAnimIndex(0);
                setTouchable(true);
            }
        }
        else if (pImpl->_state != 4)
        {
            setStateAnimIndex(4);
            setTouchable(false);
        }
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

void Object::drawHotspot(sf::RenderTarget &target, sf::RenderStates states) const
{
    if (!pImpl->_hotspotVisible)
        return;

    states.transform *= _transform.getTransform();
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
    if (!isVisible())
        return;
    states.transform *= _transform.getTransform();

    if (pImpl->_pAnim)
    {
        target.draw(*pImpl->_pAnim, states);
    }
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

std::wostream &operator<<(std::wostream &os, const Object &obj)
{
    return os << obj.getName() << L" (" << obj.getPosition().x << L"," << obj.getPosition().y << L":" << obj.getZOrder() << L")";
}

} // namespace ng
