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
      _scale(1.f)
{
}

GGObject::~GGObject()
{
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
    auto& sprite = _pAnim->getSprite();
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

void GGObject::draw(sf::RenderWindow &window) const
{
    if (!_isVisible)
        return;
    if (_pAnim)
    {
        _pAnim->draw(window);
    }
}

} // namespace gg
