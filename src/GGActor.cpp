#include "GGActor.h"

namespace gg
{
GGActor::GGActor(TextureManager &textureManager)
    : _settings(textureManager.getSettings()),
      _costume(textureManager),
      _color(sf::Color::White),
      _headAnimName("head"),
      _standAnimName("stand"),
      _walkAnimName("walk"),
      _reachAnimName("reach")
{
}

GGActor::~GGActor() = default;

void GGActor::move(const sf::Vector2f &offset)
{
    _transform.translate(offset);
}

void GGActor::setPosition(const sf::Vector2f &pos)
{
    _transform = sf::Transform::Identity;
    _transform.translate(pos);
}

void GGActor::setCostume(const std::string &name, const std::string& sheet)
{
    std::string path(_settings.getGamePath());
    path.append(name).append(".json");
    _costume.loadCostume(path, sheet);
}

void GGActor::draw(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const
{
    sf::RenderStates states;
    states.transform = _transform;
    states.transform.translate(-cameraPos - (sf::Vector2f)_renderOffset);
    _costume.draw(window, states);
}

void GGActor::update(const sf::Time &time)
{
    _costume.update(time);
}

void GGActor::setAnimationNames(const std::string &headAnim, const std::string &standAnim, const std::string &walkAnim, const std::string &reachAnim)
{
    if (!headAnim.empty())
    {
        _headAnimName = headAnim;
    }
    if (!standAnim.empty())
    {
        _standAnimName = standAnim;
    }
    if (!walkAnim.empty())
    {
        _walkAnimName = walkAnim;
    }
    if (!reachAnim.empty())
    {
        _reachAnimName = reachAnim;
    }
}
} // namespace gg
