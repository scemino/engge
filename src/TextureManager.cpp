#include <iostream>
#include "TextureManager.h"

namespace gg
{
TextureManager::TextureManager(const GGEngineSettings &settings)
    : _settings(settings)
{
}

TextureManager::~TextureManager()
{
}

void TextureManager::load(const std::string &id)
{
    std::cout << "Load texture " << id << std::endl;
    std::string path(_settings.getGamePath());
    path.append(id).append(".png");
    std::unique_ptr<sf::Texture> texture(new sf::Texture());
    texture->loadFromFile(path);

    TextureMap.insert(std::make_pair(id, std::move(texture)));
}

const sf::Texture &TextureManager::get(const std::string &id)
{
    auto found = TextureMap.find(id);
    if (found == TextureMap.end())
    {
        load(id);
        found = TextureMap.find(id);
    }
    return *found->second;
}
} // namespace gg
