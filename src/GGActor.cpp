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

void GGActor::setCostume(const std::string &name)
{
    std::string path(_settings.getGamePath());
    path.append(name).append(".json");
    _costume.loadCostume(path);
}

void GGActor::draw(sf::RenderWindow &window) const
{
    _costume.draw(window);
}

void GGActor::update(const sf::Time &time)
{
    _costume.update(time);
}
} // namespace gg
