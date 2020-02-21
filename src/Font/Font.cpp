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

NGText::NGText()
    : _alignment(NGTextAlignment::Left)
{
}

sf::FloatRect NGText::getBoundRect() const
{
    float width = 0;
    float height = 0;
    for (auto letter : _text)
    {
        auto rect = _font.getRect(letter);
        height = std::max(height, (float)rect.height);
        width += std::max(rect.width - 2, 5);
    }
    sf::FloatRect r(0, height/2.f, width, height);
    return getTransform().transformRect(r);
}

void NGText::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    states.transform *= getTransform();
    std::vector<sf::IntRect> rects;
    std::vector<sf::IntRect> sourceRects;
    int width = 0;
    for (auto letter : _text)
    {
        auto rect = _font.getRect(letter);
        sourceRects.push_back(_font.getSourceSize(letter));
        rects.push_back(rect);
        auto w = std::max(rect.width - 2, 5);
        width += w;
    }

    auto x = _alignment == NGTextAlignment::Center ? -width / 2.f : 0.f;
    for (size_t i = 0; i < rects.size(); i++)
    {
        auto rect = rects[i];
        const auto &sourceRect = sourceRects[i];
        sf::Sprite sprite;
        sprite.setTextureRect(rect);
        sprite.setTexture(_font.getTexture());
        sprite.setOrigin(-sourceRect.left, -sourceRect.top);
        sprite.setColor(_color);
        sprite.setPosition(x, 0);
        target.draw(sprite, states);
        auto w = std::max(rect.width - 2, 5);
        x += w;
    }
}

} // namespace ng
