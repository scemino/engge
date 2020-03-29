#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "Font/FntFont.hpp"

namespace ng
{
// This code has been copied from SFML Text and adapted to us FntFont
class Text : public sf::Drawable, public sf::Transformable
{
public:

    enum Style
    {
        Regular       = 0,      ///< Regular characters, no style
        Bold          = 1 << 0, ///< Bold characters
        Italic        = 1 << 1, ///< Italic characters
        Underlined    = 1 << 2, ///< Underlined characters
        StrikeThrough = 1 << 3  ///< Strike through characters
    };

    Text();
    Text(const sf::String& string, const FntFont& font, unsigned int characterSize = 30);

    void setString(const sf::String& string);
    void setFont(const Font& font);
    void setCharacterSize(unsigned int size);
    void setLineSpacing(float spacingFactor);
    void setLetterSpacing(float spacingFactor);
    void setStyle(sf::Uint32 style);
    void setFillColor(const sf::Color& color);
    void setOutlineColor(const sf::Color& color);
    void setOutlineThickness(float thickness);
    void setMaxWidth(float maxWidth);

    const sf::String& getString() const;
    const Font* getFont() const;
    unsigned int getCharacterSize() const;
    float getLetterSpacing() const;
    float getLineSpacing() const;
    sf::Uint32 getStyle() const;
    const sf::Color& getFillColor() const;
    const sf::Color& getOutlineColor() const;
    float getOutlineThickness() const;
    sf::FloatRect getLocalBounds() const;
    sf::FloatRect getGlobalBounds() const;

private:

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    void ensureGeometryUpdate() const;

    sf::String                 m_string;              ///< String to display
    const Font*                m_font;                ///< GGFont used to display the string
    unsigned int               m_characterSize;       ///< Base size of characters, in pixels
    float                      m_letterSpacingFactor; ///< Spacing factor between letters
    float                      m_lineSpacingFactor;   ///< Spacing factor between lines
    sf::Uint32                 m_style;               ///< Text style (see Style enum)
    mutable sf::Color          m_fillColor;           ///< Text fill color
    sf::Color                  m_outlineColor;        ///< Text outline color
    float                      m_outlineThickness;    ///< Thickness of the text's outline
    mutable sf::VertexArray    m_vertices;            ///< Vertex array containing the fill geometry
    mutable sf::VertexArray    m_outlineVertices;     ///< Vertex array containing the outline geometry
    mutable sf::FloatRect      m_bounds;              ///< Bounding rectangle of the text (in local coordinates)
    mutable bool               m_geometryNeedUpdate;  ///< Does the geometry need to be recomputed?
    mutable const sf::Texture* m_fontTextureId;       ///< The font texture id
    float                      m_maxWidth{0};
};

} // namespace sf
