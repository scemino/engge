#include "GGObject.h"

namespace gg
{
GGAnim::GGAnim(const sf::Texture& texture, const std::string &name)
    : _sprite(texture), _name(name), _fps(10), _index(0)
{
}

GGAnim::~GGAnim()
{
}

void GGAnim::update(const sf::Time &elapsed)
{
    _time += elapsed;
    if (_time.asSeconds() > (1.f / _fps))
    {
        _time = sf::seconds(0);
        _index = (_index + 1) % _rects.size();

        auto &sourceRect = _sourceRects[_index];
        _sprite.setTextureRect(_rects[_index]);
        _sprite.setOrigin(sourceRect.width / 2, sourceRect.height / 2);
    }
}

void GGAnim::draw(sf::RenderWindow &window) const
{
    window.draw(_sprite);
}
} // namespace gg
