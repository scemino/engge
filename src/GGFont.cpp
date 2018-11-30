#include <fstream>
#include "GGFont.h"
#include "_GGUtil.h"

namespace gg
{
GGFont::GGFont() = default;

GGFont::~GGFont() = default;

void GGFont::setSettings(const GGEngineSettings *settings)
{
    _settings = settings;
}

void GGFont::setTextureManager(TextureManager *textureManager)
{
    _textureManager = textureManager;
}

void GGFont::load(const std::string &path)
{
    _path = path;
    _jsonFilename = _settings->getGamePath();
    _jsonFilename.append(path);
    _jsonFilename.append(".json");

    std::ifstream input(_jsonFilename);
    input >> _json;

    _texture = _textureManager->get(_path);
}

sf::IntRect GGFont::getRect(char letter) const
{
    const auto &s = std::to_string((int)letter);
    return _toRect(_json["frames"][s]["frame"]);
}

sf::IntRect GGFont::getSize(char letter) const
{
    const auto &s = std::to_string((int)letter);
    return _toRect(_json["frames"][s]["spriteSourceSize"]);
}

void GGText::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    float scale = 0.2f;
    std::vector<sf::IntRect> rects;
    std::vector<sf::IntRect> sourceRects;
    int width = 0;
    for (auto letter : _text)
    {
        auto rect = _font.getRect(letter);
        sourceRects.push_back(_font.getSize(letter));
        rects.push_back(rect);
        width += std::max(rect.width * scale, 10.f * scale);
    }

    auto x = 0;
    for (auto i = 0; i < rects.size(); i++)
    {
        auto rect = rects[i];
        const auto &sourceRect = sourceRects[i];
        sf::Sprite _sprite;
        _sprite.setScale(scale, scale);
        _sprite.setTextureRect(rect);
        _sprite.setTexture(_font.getTexture());
        _sprite.setOrigin(-sourceRect.left, -sourceRect.top);
        _sprite.setColor(_color);
        _sprite.setPosition(x - width / 2, 0);
        target.draw(_sprite, states);
        x += std::max(rect.width * scale, 10.f * scale);
    }
}

} // namespace gg
