#include <fstream>
#include "Engine/EngineSettings.hpp"
#include "Font/Font.hpp"
#include "System/Locator.hpp"
#include "../System/_Util.hpp"

namespace ng
{
void Font::setTextureManager(TextureManager *textureManager)
{
    _textureManager = textureManager;
}

void Font::load(const std::string &path)
{
    _path = path;
    _jsonFilename = path;
    _jsonFilename.append(".json");

    std::vector<char> buffer;
    Locator<EngineSettings>::get().readEntry(_jsonFilename, buffer);
    _json = ng::Json::Parser::parse(buffer);

    _texture = _textureManager->get(_path);
}

sf::IntRect Font::getRect(uint32_t letter) const
{
    auto s = std::to_string(letter);
    return _toRect(_json["frames"][s]["frame"]);
}

sf::IntRect Font::getSourceSize(uint32_t letter) const
{
    auto s = std::to_string(letter);
    return _toRect(_json["frames"][s]["spriteSourceSize"]);
}

} // namespace ng
