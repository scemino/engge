#include <sstream>
#include "Animation.h"
#include "Object.h"
#include "Screen.h"
#include "Trigger.h"

namespace ng
{
struct Object::Impl
{
    std::vector<std::unique_ptr<Animation>> _anims;
    std::optional<Animation> _pAnim;
    bool _isVisible;
    std::wstring _name, _id;
    int _zorder;
    UseDirection _direction;
    bool _prop;
    bool _spot;
    bool _trigger;
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

    Impl()
        : _pAnim(std::nullopt),
          _isVisible(true),
          _zorder(0),
          _direction(UseDirection::Front),
          _prop(false),
          _spot(false),
          _trigger(false),
          _color(sf::Color::White),
          _angle(0),
          _isTouchable(true),
          _pRoom(nullptr),
          _state(0),
          _hotspotVisible(false),
          _verb(1)
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

bool Object::getProp() const
{
    return pImpl->_prop;
}

void Object::setSpot(bool spot)
{
    pImpl->_spot = spot;
}

bool Object::getSpot() const
{
    return pImpl->_spot;
}

void Object::setTrigger(bool trigger)
{
    pImpl->_trigger = trigger;
}
bool Object::getTrigger() const
{
    return pImpl->_trigger;
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
const float Object::getRotation() const { return _transform.getRotation(); }

bool Object::isVisible() const { return pImpl->_isVisible; }

Room *Object::getRoom() { return pImpl->_pRoom; }
const Room *Object::getRoom() const { return pImpl->_pRoom; }
void Object::setRoom(Room *pRoom) { pImpl->_pRoom = pRoom; }

void Object::setHotspotVisible(bool isVisible) { pImpl->_hotspotVisible = isVisible; }

void Object::addTrigger(std::shared_ptr<Trigger> trigger) { pImpl->_triggers.push_back(trigger); }
void Object::removeTrigger() { pImpl->_triggers.clear(); }
Trigger* Object::getTrigger() { return pImpl->_triggers.size() > 0 ? pImpl->_triggers[0].get() : nullptr; }

bool Object::isTouchable() const
{
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

void Object::setVisible(bool isVisible)
{
    pImpl->_isVisible = isVisible;
    if (!pImpl->_isVisible)
    {
        pImpl->_isTouchable = false;
    }
}

sf::IntRect Object::getRealHotspot() const
{
    auto rect = getHotspot();
    auto pos = getPosition();
    return (sf::IntRect)_transform.getTransform().transformRect((sf::FloatRect)rect);
}

void Object::setStateAnimIndex(int animIndex)
{
    std::ostringstream s;
    s << "state" << animIndex;
    pImpl->_state = animIndex;
    if (animIndex == 4)
    {
        pImpl->_isTouchable = false;
        pImpl->_isVisible = false;
    }
    setAnimation(s.str());
}

int Object::getStateAnimIndex()
{
    if (!pImpl->_pAnim.has_value())
        return -1;
    if (pImpl->_pAnim->getName().find("state") == std::string::npos)
        return -1;
    return static_cast<int>(std::strtol(pImpl->_pAnim->getName().c_str() + 5, nullptr, 10));
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
    if (pImpl->_pAnim)
    {
        pImpl->_pAnim->update(elapsed);
    }
    for (auto &trigger : pImpl->_triggers)
    {
        trigger->trig();
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
    if (!pImpl->_isVisible)
        return;
    states.transform *= _transform.getTransform();

    if (pImpl->_pAnim)
    {
        target.draw(*pImpl->_pAnim, states);
    }
}

std::wostream &operator<<(std::wostream &os, const Object &obj)
{
    return os << obj.getName() << L" (" << obj.getPosition().x << L"," << obj.getPosition().y << L":" << obj.getZOrder() << L")";
}

} // namespace ng
