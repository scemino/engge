#include <fstream>
#include "Font.h"
#include "_Util.h"

namespace ng
{
Font::Font() = default;

Font::~Font() = default;

void Font::setSettings(EngineSettings *settings)
{
    _settings = settings;
}

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
    _settings->readEntry(_jsonFilename, buffer);
    _json = nlohmann::json::parse(buffer.data());

    _texture = _textureManager->get(_path);
}

sf::IntRect Font::getRect(char letter) const
{
    const auto &s = std::to_string((int)letter);
    return _toRect(_json["frames"][s]["frame"]);
}

sf::IntRect Font::getSize(char letter) const
{
    const auto &s = std::to_string((int)letter);
    return _toRect(_json["frames"][s]["spriteSourceSize"]);
}

NGText::NGText()
    : _alignment(NGTextAlignment::Center)
{
}

sf::FloatRect NGText::getBoundRect() const
{
    float width = 0;
    float scale = 0.2f;
    float height = 0;
    for (auto letter : _text)
    {
        auto rect = _font.getRect(letter);
        height = std::max(height, (float)rect.height * scale);
        width += std::max(rect.width * scale, 10.f * scale);
    }
    sf::FloatRect r(0, 0, width, height);
    return getTransform().transformRect(r);
}

void NGText::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    states.transform *= getTransform();
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

    auto x = _alignment == NGTextAlignment::Center ? -width / 2.f : 0.f;
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
        _sprite.setPosition(x, 0);
        target.draw(_sprite, states);
        x += std::max(rect.width * scale, 10.f * scale);
    }
}

} // namespace ng
