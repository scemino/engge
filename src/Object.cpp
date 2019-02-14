#include "Object.h"
#include "Screen.h"

namespace ng
{
Object::Object()
    : _pAnim(std::nullopt),
      _isVisible(true),
      _zorder(0),
      _direction(UseDirection::Front),
      _prop(false),
      _color(sf::Color::White),
      _angle(0),
      _isTouchable(true),
      _pRoom(nullptr),
      _state(0),
      _hotspotVisible(false)
{
}

Object::~Object() = default;

void Object::setVisible(bool isVisible)
{
    _isVisible = isVisible;
    if (!_isVisible)
    {
        _isTouchable = false;
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
    _state = animIndex;
    if (animIndex == 4)
    {
        _isTouchable = false;
        _isVisible = false;
    }
    setAnimation(s.str());
}

int Object::getStateAnimIndex()
{
    if (!_pAnim.has_value())
        return -1;
    if (_pAnim->getName().find("state") == std::string::npos)
        return -1;
    return static_cast<int>(std::strtol(_pAnim->getName().c_str() + 5, nullptr, 10));
}

void Object::setAnimation(const std::string &name)
{
    auto it = std::find_if(_anims.begin(), _anims.end(), [name](std::unique_ptr<Animation> &animation) { return animation->getName() == name; });
    if (it == _anims.end())
    {
        _pAnim = std::nullopt;
        return;
    }

    auto &anim = *(it->get());
    _pAnim = anim;
    _pAnim->setObject(this);
    auto &sprite = _pAnim->getSprite();
    sprite.setColor(_color);
}

void Object::move(const sf::Vector2f &offset)
{
    _transform.move(offset);
}

void Object::setColor(const sf::Color &color)
{
    _color = color;
    if (_pAnim)
    {
        _pAnim->getSprite().setColor(color);
    }
}

const sf::Color &Object::getColor() const
{
    return _color;
}

void Object::setScale(float s)
{
    _transform.setScale(s, s);
}

void Object::update(const sf::Time &elapsed)
{
    if (_pAnim)
    {
        _pAnim->update(elapsed);
    }
    for (auto &trigger : _triggers)
    {
        trigger->trig();
    }
}

void Object::drawHotspot(sf::RenderTarget &target, sf::RenderStates states) const
{
    if(!_hotspotVisible) return;
    
    states.transform *= _transform.getTransform();
    auto rect = getHotspot();

    sf::RectangleShape s(sf::Vector2f(rect.width, rect.height));
    s.setPosition(rect.left, rect.top);
    s.setOutlineThickness(1);
    s.setOutlineColor(sf::Color::Red);
    s.setFillColor(sf::Color::Transparent);
    target.draw(s, states);

    sf::RectangleShape vl(sf::Vector2f(1, 5));
    vl.setPosition(_usePos.x, -_usePos.y - 2);
    vl.setFillColor(sf::Color::Red);
    target.draw(vl, states);

    sf::RectangleShape hl(sf::Vector2f(5, 1));
    hl.setPosition(_usePos.x - 2, -_usePos.y);
    hl.setFillColor(sf::Color::Red);
    target.draw(hl, states);
}

void Object::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    if (!_isVisible)
        return;
    states.transform *= _transform.getTransform();

    if (_pAnim)
    {
        target.draw(*_pAnim, states);
    }
}

std::ostream &operator<<(std::ostream &os, const Object &obj)
{
    return os << obj.getName() << " (" << obj.getPosition().x << "," << obj.getPosition().y << ":" << obj.getZOrder() << ")";
}

} // namespace ng
