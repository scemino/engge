#include "GGObject.h"
#include "Screen.h"

namespace gg
{
GGObject::GGObject()
    : _pAnim(nullptr),
      _isVisible(true),
      _zorder(0),
      _direction(UseDirection::Front),
      _prop(false),
      _color(sf::Color::White),
      _isHotspotVisible(false),
      _angle(0),
      _isTouchable(true),
      _pOwner(nullptr)
{
}

GGObject::~GGObject() = default;

sf::IntRect GGObject::getRealHotspot() const
{
    auto rect = getHotspot();
    auto pos = getPosition();
    return sf::IntRect(static_cast<int>(pos.x + rect.left), static_cast<int>(pos.y + rect.top), rect.width, rect.height);
}

void GGObject::setStateAnimIndex(int animIndex)
{
    std::ostringstream s;
    s << "state" << animIndex;
    setAnimation(s.str());
}

int GGObject::getStateAnimIndex()
{
    if (_pAnim == nullptr)
        return -1;
    if (_pAnim->getName().find("state") == std::string::npos)
        return -1;
    return std::strtol(_pAnim->getName().c_str(), nullptr, 10);
}

void GGObject::setAnimation(const std::string &name)
{
    auto it = std::find_if(_anims.begin(), _anims.end(), [name](std::unique_ptr<GGAnimation> &animation) { return animation->getName() == name; });
    if (it == _anims.end())
    {
        _pAnim = nullptr;
        return;
    }

    auto &anim = *(it->get());
    _pAnim = &anim;
    auto &sprite = _pAnim->getSprite();
    sprite.setColor(_color);
}

void GGObject::move(const sf::Vector2f &offset)
{
    _transform.move(offset);
}

void GGObject::setPosition(const sf::Vector2f &pos)
{
    _transform.setPosition(pos);
}

sf::Vector2f GGObject::getPosition() const
{
    return _transform.getPosition();
}

void GGObject::setUsePosition(const sf::Vector2f &pos)
{
    _usePos = sf::Vector2f(pos);
}

sf::Vector2f GGObject::getUsePosition() const
{
    return _usePos;
}

void GGObject::setColor(const sf::Color &color)
{
    _color = color;
    if (_pAnim)
    {
        _pAnim->getSprite().setColor(color);
    }
}

const sf::Color &GGObject::getColor() const
{
    return _color;
}

void GGObject::setScale(float s)
{
    _transform.scale(s, s);
}

void GGObject::update(const sf::Time &elapsed)
{
    if (_pAnim)
    {
        _pAnim->update(elapsed);
    }
}

void GGObject::drawHotspot(sf::RenderWindow &window, sf::RenderStates states) const
{
    if (!_isHotspotVisible)
        return;
    auto rect = getHotspot();
    auto pos = getPosition();
    sf::RectangleShape s(sf::Vector2f(rect.width, rect.height));
    s.setPosition(pos.x + rect.left, pos.y + rect.top);
    s.setOutlineThickness(1);
    s.setOutlineColor(_isHotspotVisible ? sf::Color::Red : sf::Color::Blue);
    s.setFillColor(sf::Color::Transparent);
    window.draw(s, states);

    sf::RectangleShape vl(sf::Vector2f(1, 5));
    vl.setPosition(pos.x + _usePos.x, pos.y - _usePos.y - 2);
    vl.setFillColor(_isHotspotVisible ? sf::Color::Red : sf::Color::Blue);
    window.draw(vl, states);

    sf::RectangleShape hl(sf::Vector2f(5, 1));
    hl.setPosition(pos.x + _usePos.x - 2, pos.y - _usePos.y);
    hl.setFillColor(_isHotspotVisible ? sf::Color::Red : sf::Color::Blue);
    window.draw(hl, states);
}

void GGObject::draw(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const
{
    sf::RenderStates states;
    states.transform = _transform.getTransform();
    states.transform.translate(-cameraPos);
    if (_isVisible && _pAnim)
    {
        _pAnim->draw(window, states);
    }
    drawHotspot(window, states);
}

std::ostream &operator<<(std::ostream &os, const GGObject &obj)
{
    return os << obj.getName() << " (" << obj.getPosition().x << "," << obj.getPosition().y << ":" << obj.getZOrder() << ")";
}

} // namespace gg
