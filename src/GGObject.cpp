#include "GGObject.h"

namespace gg
{
GGObject::GGObject()
    : _pAnim(nullptr),
      _isVisible(true),
      _zorder(0),
      _direction(UseDirection::DIR_FRONT),
      _prop(false),
      _color(sf::Color::White),
      _scale(1.f),
      _isHotspotVisible(false)
{
}

GGObject::~GGObject()
{
}

sf::IntRect GGObject::getRealHotspot() const
{
    auto rect = getHotspot();
    auto pos = getPosition();
    return sf::IntRect(pos.x + rect.left, pos.y + rect.top, rect.width, rect.height);
}

void GGObject::setStateAnimIndex(int animIndex)
{
    std::ostringstream s;
    s << "state" << animIndex;
    setAnim(s.str());
}

void GGObject::setAnim(const std::string &name)
{
    auto it = std::find_if(_anims.begin(), _anims.end(), [name](std::unique_ptr<GGAnim> &anim) { return anim.get()->getName() == name; });
    auto &anim = *(it->get());
    _pAnim = &anim;
    auto &sprite = _pAnim->getSprite();
    sprite.setPosition(_pos);
    sprite.setColor(_color);
    sprite.setScale(_scale, _scale);
}

void GGObject::move(float x, float y)
{
    _pos += sf::Vector2f(x, y);
}

void GGObject::setPosition(float x, float y)
{
    _pos = sf::Vector2f(x, y);
}

const sf::Vector2f &GGObject::getPosition() const
{
    return _pos;
}

void GGObject::setUsePosition(float x, float y)
{
    _usePos = sf::Vector2f(x, y);
}

const sf::Vector2f &GGObject::getUsePosition() const
{
    return _usePos;
}

void GGObject::setColor(const sf::Color &color)
{
    _color = color;
}

const sf::Color &GGObject::getColor() const
{
    return _color;
}

void GGObject::setScale(float s)
{
    _scale = s;
}

void GGObject::update(const sf::Time &elapsed)
{
    if (_pAnim)
    {
        _pAnim->update(elapsed);
    }
}

void GGObject::drawHotspot(sf::RenderWindow &window) const
{
    if (!_isHotspotVisible) return;
    auto rect = getHotspot();
    auto pos = getPosition();
    sf::RectangleShape s(sf::Vector2f(rect.width, rect.height));
    s.setPosition(pos.x + rect.left, pos.y + rect.top);
    s.setOutlineThickness(1);
    s.setOutlineColor(_isHotspotVisible ? sf::Color::Red : sf::Color::Blue);
    s.setFillColor(sf::Color::Transparent);
    window.draw(s);

    sf::RectangleShape vl(sf::Vector2f(1, 5));
    vl.setPosition(pos.x + _usePos.x, pos.y - _usePos.y - 2);
    vl.setFillColor(_isHotspotVisible ? sf::Color::Red : sf::Color::Blue);
    window.draw(vl);
    
    sf::RectangleShape hl(sf::Vector2f(5, 1));
    hl.setPosition(pos.x + _usePos.x - 2, pos.y - _usePos.y);
    hl.setFillColor(_isHotspotVisible ? sf::Color::Red : sf::Color::Blue);
    window.draw(hl);
}

void GGObject::draw(sf::RenderWindow &window) const
{
    if (_isVisible && _pAnim)
    {
        _pAnim->draw(window);
    }
    drawHotspot(window);
}

} // namespace gg
