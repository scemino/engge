#include "Object.h"
#include "Animation.h"
#include "Function.h"
#include "Locator.h"
#include "ResourceManager.h"
#include "Room.h"
#include "Screen.h"
#include "ScriptEngine.h"
#include "Trigger.h"
#include <sstream>

namespace ng
{
struct Object::Impl
{
    std::vector<std::unique_ptr<Animation>> _anims;
    std::optional<Animation> _pAnim{std::nullopt};
    std::wstring _name;
    int _zorder{0};
    UseDirection _direction{UseDirection::Front};
    bool _prop{false};
    bool _spot{false};
    bool _isTrigger{false};
    sf::Vector2f _usePos;
    sf::Vector2f _defaultPosition;
    sf::IntRect _hotspot;
    Room *_pRoom{nullptr};
    int _state{0};
    std::optional<std::shared_ptr<Trigger>> _trigger;
    HSQOBJECT _pTable{};
    bool _hotspotVisible{false};
    bool _triggerEnabled{true};
    Object *pParentObject{nullptr};
    int dependentState{0};
    Actor *_owner{nullptr};
    int _fps{0};
    std::vector<std::string> _icons;
    sf::Time _elapsed;
    int _index{0};
    ScreenSpace _screenSpace{ScreenSpace::Room};
    std::vector<Object *> _children;
    bool _temporary{false};
};

Object::Object() : pImpl(std::make_unique<Impl>()) 
{
    _id = Locator::getResourceManager().getObjectId();
}

Object::~Object() = default;

void Object::setZOrder(int zorder) { pImpl->_zorder = zorder; }

int Object::getZOrder() const { return pImpl->_zorder; }

void Object::setProp(bool prop) { pImpl->_prop = prop; }

void Object::setSpot(bool spot) { pImpl->_spot = spot; }

void Object::setTrigger(bool trigger) { pImpl->_isTrigger = trigger; }

void Object::setUseDirection(UseDirection direction) { pImpl->_direction = direction; }
UseDirection Object::getUseDirection() const { return pImpl->_direction; }

void Object::setHotspot(const sf::IntRect &hotspot) { pImpl->_hotspot = hotspot; }
const sf::IntRect &Object::getHotspot() const { return pImpl->_hotspot; }

void Object::setIcon(const std::string &icon)
{
    pImpl->_icons.clear();
    pImpl->_fps = 0;
    pImpl->_index = 0;
    pImpl->_elapsed = sf::seconds(0);
    pImpl->_icons.push_back(icon);
}

std::string Object::getIcon() const { return pImpl->_icons[pImpl->_index]; }

void Object::setIcon(int fps, const std::vector<std::string> &icons)
{
    pImpl->_icons.clear();
    pImpl->_fps = fps;
    pImpl->_index = 0;
    pImpl->_elapsed = sf::seconds(0);
    std::copy(icons.begin(), icons.end(), std::back_inserter(pImpl->_icons));
}

void Object::setOwner(Actor *pActor) { pImpl->_owner = pActor; }
Actor *Object::getOwner() const { return pImpl->_owner; }

HSQOBJECT &Object::getTable() { return pImpl->_pTable; }
HSQOBJECT &Object::getTable() const { return pImpl->_pTable; }
bool Object::isInventoryObject() const { return getOwner() != nullptr; }

std::vector<std::unique_ptr<Animation>> &Object::getAnims() { return pImpl->_anims; }

Room *Object::getRoom() { return pImpl->_pRoom; }
const Room *Object::getRoom() const { return pImpl->_pRoom; }
void Object::setRoom(Room *pRoom) { pImpl->_pRoom = pRoom; }

void Object::addTrigger(const std::shared_ptr<Trigger> &trigger) { pImpl->_trigger = trigger; }

void Object::removeTrigger()
{
    if (pImpl->_trigger.has_value())
    {
        (*pImpl->_trigger)->disable();
    }
}

Trigger *Object::getTrigger() { return pImpl->_trigger.has_value() ? (*pImpl->_trigger).get() : nullptr; }
void Object::enableTrigger(bool enabled) { pImpl->_triggerEnabled = enabled; }

bool Object::isTouchable() const
{
    if (!isVisible())
        return false;
    if (pImpl->_isTrigger)
        return false;
    if (pImpl->_spot)
        return false;
    if (pImpl->_prop)
        return false;
    return Entity::isTouchable();
}

sf::IntRect Object::getRealHotspot() const
{
    auto rect = getHotspot();
    auto transform = getTransform();
    return (sf::IntRect)transform.transformRect((sf::FloatRect)rect);
}

bool Object::isVisible() const
{
    if (pImpl->_state == ObjectStateConstants::GONE)
        return false;
    return Entity::isVisible();
}

void Object::setStateAnimIndex(int animIndex)
{
    std::ostringstream s;
    s << "state" << animIndex;
    pImpl->_state = animIndex;

    setVisible(animIndex != ObjectStateConstants::GONE);
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

int Object::getState() { return pImpl->_state; }

void Object::setAnimation(const std::string &name)
{
    auto it = std::find_if(pImpl->_anims.begin(), pImpl->_anims.end(),
                           [name](std::unique_ptr<Animation> &animation) { return animation->getName() == name; });
    if (it == pImpl->_anims.end())
    {
        pImpl->_pAnim = std::nullopt;
        return;
    }

    auto &anim = *(it->get());
    pImpl->_pAnim = anim;
    pImpl->_pAnim->setObject(this);
}

std::optional<Animation> &Object::getAnimation() { return pImpl->_pAnim; }

void Object::update(const sf::Time &elapsed)
{
    if (isInventoryObject())
    {
        if (pImpl->_fps == 0)
            return;
        pImpl->_elapsed += elapsed;
        if (pImpl->_elapsed.asSeconds() > (1.f / pImpl->_fps))
        {
            pImpl->_elapsed = sf::seconds(0);
            pImpl->_index = (pImpl->_index + 1) % pImpl->_icons.size();
        }
        return;
    }

    Entity::update(elapsed);
    if (pImpl->pParentObject)
    {
        setVisible(pImpl->pParentObject->getState() == pImpl->dependentState);
    }
    if (pImpl->_pAnim)
    {
        pImpl->_pAnim->update(elapsed);
    }
    if (pImpl->_triggerEnabled && pImpl->_trigger.has_value())
    {
        (*pImpl->_trigger)->trig();
    }
}

void Object::showHotspot(bool show) { pImpl->_hotspotVisible = show; }

bool Object::isHotspotVisible() const { return pImpl->_hotspotVisible; }

void Object::setScreenSpace(ScreenSpace screenSpace) { pImpl->_screenSpace = screenSpace; }

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

void Object::drawForeground(sf::RenderTarget &target, sf::RenderStates states) const
{
    if (pImpl->_screenSpace != ScreenSpace::Object)
        return;

    const auto view = target.getView();
    target.setView(sf::View(sf::FloatRect(0, 0, Screen::Width, Screen::Height)));

    auto size = getRoom()->getRoomSize();

    sf::RenderStates s;
    auto transform = _transform;
    transform.move(getOffset());
    transform.move(0, 720 - size.y);
    s.transform = transform.getTransform();

    if (pImpl->_pAnim)
    {
        pImpl->_pAnim->getSprite().setColor(getColor());
        target.draw(*pImpl->_pAnim, s);
    }

    drawHotspot(target, s);
    target.setView(view);
}

void Object::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    if (!isVisible())
        return;

    if (pImpl->_screenSpace == ScreenSpace::Object)
        return;

    auto transform = getTransform();
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

bool Object::isTrigger() const { return pImpl->_isTrigger; }

void Object::addChild(Object *child) { pImpl->_children.push_back(child); }

void Object::stopObjectMotors()
{
    Entity::stopObjectMotors();
    for (auto &&child : pImpl->_children)
    {
        child->stopObjectMotors();
    }
}

void Object::setTemporary(bool isTemporary) { pImpl->_temporary = isTemporary; }

bool Object::isTemporary() const { return pImpl->_temporary; }

std::wostream &operator<<(std::wostream &os, const Object &obj)
{
    return os << obj.getName() << L" (" << obj.getRealPosition().x << L"," << obj.getRealPosition().y << L":"
              << obj.getZOrder() << L")";
}

} // namespace ng
