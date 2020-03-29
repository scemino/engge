#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <list>
#include <unordered_map>
#include "Font.hpp"

namespace ng
{
// original code from:
// - http://www.angelcode.com/products/bmfont/
// - https://github.com/Zylann/zCraft/blob/master/src/engine/bmfont/Font.cpp
// - https://www.gamedev.net/forums/topic/330742-quick-tutorial-variable-width-bitmap-fonts/
// the code has been adapted to work with SFML
struct FontInfo
{
	struct Padding
	{
		int up;
		int right;
		int down;
		int left;

		Padding() : up(0), right(0), down(0), left(0) {}
	};

	struct Spacing
	{
		int horizontal;
		int vertical;

		Spacing() : horizontal(0), vertical(0) {}
	};

	std::string face;	  // This is the name of the true type font.
	unsigned int size;	 // The size of the true type font.
	bool bold;			   // The font is bold.
	bool italic;		   // The font is italic.
	std::string charset;   // The name of the OEM charset used (when not unicode).
	bool unicode;		   // Set to 1 if it is the unicode charset.
	unsigned int stretchH; // The font height stretch in percentage. 100% means no stretch.
	bool smooth;		   // Set to 1 if smoothing was turned on.
	unsigned int aa;	   // The supersampling level used. 1 means no supersampling was used.
	Padding padding;	   // The padding for each character (up, right, down, left).
	Spacing spacing;	   // The spacing for each character (horizontal, vertical).
	unsigned int outline;  // The outline thickness for the characters.

	FontInfo()
		: size(0),
		  bold(false),
		  italic(false),
		  unicode(false),
		  stretchH(0),
		  smooth(false),
		  aa(1),
		  outline(0)
	{
	}
};

struct Kerning
{
	short first;  // The first character id.
	short second; // The second character id.
	short amount;

	Kerning() : first(0), second(0), amount(0) {}
};

class CharSet
{
  public:
	void addKerning(Kerning k);
	[[nodiscard]] short getKerning(int first, int second) const;

	void addChar(int id, sf::Glyph& cd);
	[[nodiscard]] const sf::Glyph& getChar(int id) const;

	std::vector<std::string> pages; // [id] = file

	// This is the distance in pixels between each line of text.
	unsigned short lineHeight;

	// The number of pixels from the absolute top of the line to the
	// base of the characters.
	unsigned short base;

	// The size of the texture, normally used to scale the coordinates of
	// the character image.
	unsigned short scaleW;
	unsigned short scaleH;

	// Set to 1 if the monochrome characters have been packed into each of
	// the texture channels. In this case alphaChnl describes what is
	// stored in each channel.
	unsigned short packed;

	// For each color :
	// Set to 0 if the channel holds the glyph data, 1 if it holds the
	// outline, 2 if it holds the glyph and the outline, 3 if its set to
	// zero, and 4 if its set to one.
	unsigned short alphaChnl;
	unsigned short redChnl;
	unsigned short greenChnl;
	unsigned short blueChnl;

	CharSet()
		: lineHeight(0),
		  base(0),
		  scaleW(0), scaleH(0),
		  packed(false),
		  alphaChnl(0),
		  redChnl(4),
		  greenChnl(4),
		  blueChnl(4)
	{
	}

  private:
  typedef std::map<int, sf::Glyph> GlyphTable; ///< Table mapping a codepoint to its glyph
	GlyphTable m_chars;
	std::list<Kerning> m_kernings;
};

class FntFont: public Font
{
  private:
	CharSet m_chars;
	FontInfo m_info;
	std::vector<sf::Texture> m_textures;

  public:
    ~FntFont() override;
	bool loadFromFile(const std::string& path);

	[[nodiscard]] int getLineHeight() const override;
	[[nodiscard]] const sf::Glyph& getGlyph(sf::Uint32 codePoint, unsigned int characterSize, bool bold, float outlineThickness) const override;
	[[nodiscard]] float getKerning(sf::Uint32 first, sf::Uint32 second, unsigned int characterSize) const override;
	[[nodiscard]] const sf::Texture& getTexture(unsigned int characterSize) const override;

  private:
	bool parse(const std::string& path);
};
} // namespace gg
