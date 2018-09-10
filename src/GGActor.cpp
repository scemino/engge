#include "GGActor.h"

namespace gg
{
GGActor::GGActor(TextureManager &textureManager)
    : _settings(textureManager.getSettings()), _costume(_settings)
{
}

GGActor::~GGActor()
{
}

void GGActor::move(float x, float y)
{
    _transform.translate(x, y);
}

void GGActor::setPosition(float x, float y)
{
    _transform = sf::Transform::Identity;
    _transform.translate(x, y);
}

void GGActor::setCostume(const std::string &name)
{
    std::string path(_settings.getGamePath());
    path.append(name).append(".json");
    _costume.loadCostume(path);
}

void GGActor::draw(sf::RenderWindow &window) const
{
    sf::RenderStates states;
    states.transform = _transform;
    _costume.draw(window, states);
}

void GGActor::update(const sf::Time &time)
{
    _costume.update(time);
}
} // namespace gg
