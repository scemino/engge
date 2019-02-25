#include <iostream>
#include "TextureManager.h"

namespace ng
{
TextureManager::TextureManager(EngineSettings &settings)
    : _settings(settings)
{
}

TextureManager::~TextureManager() = default;

void TextureManager::load(const std::string &id)
{
    std::cout << "Load texture " << id << std::endl;
    std::string path;
    path.append(id).append(".png");
    auto texture = std::make_shared<sf::Texture>();
    std::vector<char> data;
    _settings.readEntry(path, data);
    if (!texture->loadFromMemory(data.data(), data.size()))
    {
        std::cerr << "Fail to load texture " << path << std::endl;
    }

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
} // namespace ng
