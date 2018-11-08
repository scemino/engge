#include <iostream>
#include "TextureManager.h"

namespace gg
{
TextureManager::TextureManager(const GGEngineSettings &settings)
    : _settings(settings)
{
}

TextureManager::~TextureManager() = default;

void TextureManager::load(const std::string &id)
{
    std::cout << "Load texture " << id << std::endl;
    std::string path(_settings.getGamePath());
    path.append(id).append(".png");
    auto texture = std::make_shared<sf::Texture>();
    texture->loadFromFile(path);

    _textureMap.insert(std::make_pair(id, texture));
}

const sf::Texture &TextureManager::get(const std::string &id)
{
    auto found = _textureMap.find(id);
    if (found == _textureMap.end())
    {
        load(id);
        found = _textureMap.find(id);
    }
    return *found->second;
}
} // namespace gg
