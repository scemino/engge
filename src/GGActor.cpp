#include "GGActor.h"

namespace gg
{
GGActor::GGActor(TextureManager &textureManager)
    : _settings(textureManager.getSettings()), _costume(_settings)
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

void GGActor::setCostume(const std::string &name)
{
    std::string path(_settings.getGamePath());
    path.append(name).append(".json");
    _costume.loadCostume(path);
}

void GGActor::draw(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const
{
    sf::RenderStates states;
    states.transform = _transform;
    states.transform.translate(-cameraPos);
    _costume.draw(window, states);
}

void GGActor::update(const sf::Time &time)
{
    _costume.update(time);
}
} // namespace gg
