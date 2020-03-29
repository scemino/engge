#pragma once
#include <SFML/Graphics.hpp>

namespace ng
{

class Font
{
public:
    virtual ~Font() = default;
	[[nodiscard]] virtual int getLineHeight() const = 0;
	[[nodiscard]] virtual const sf::Glyph& getGlyph(sf::Uint32 codePoint, unsigned int characterSize, bool bold, float outlineThickness = 0) const = 0;
	[[nodiscard]] virtual float getKerning(sf::Uint32 first, sf::Uint32 second, unsigned int characterSize) const = 0;
	[[nodiscard]] virtual const sf::Texture& getTexture(unsigned int characterSize) const = 0;
};

} // namespace gg
