#pragma once
#include <ngf/Graphics/Rect.h>
#include <ngf/Graphics/Texture.h>

namespace ng {
/// @brief A glyph is the visual representation of a character.
/// Glyphs are loaded from fonts and put in a texture. In order to draw a
/// glyph, you need to known the bounding rectangle of the glyph and the
/// texture coordinates where the glyph is. Then, you can compute the
/// position of the next glyph with the `advance` field.
///
/// Generally, you do not have to manipulate glyphs directly. ngf::Text
/// can display text and make all the necessary computations for you.
/// @sa ngf::Font, ngf::Text
struct Glyph {
  float advance{0};  ///< Offset to move horizontally to the next character.
  ngf::irect bounds;      ///< Bounding rectangle of the glyph, in coordinates relative to the baseline.
  ngf::irect textureRect; ///< Texture coordinates of the glyph inside the font's texture.
};

/// @brief Defines a particular format for text.
class Font {
public:
  virtual ~Font() = default;
  /// @brief Gets the glyph for a specified codepoint.
  /// \param codepoint Codepoint for which we want to get the glyph.
  /// \return The glyph for the codepoint.
  [[nodiscard]] virtual const Glyph &getGlyph(unsigned int codepoint) const = 0;
  /// @brief Get the kerning offset of two glyphs.
  ///
  /// The kerning is an extra offset (negative) to apply between two glyphs when rendering them, to make the pair look more "natural".
  /// For example, the pair "AV" have a special kerning to make them closer than other characters.
  /// Most of the glyphs pairs have a kerning offset of zero, though.
  /// \param first Unicode code point of the first character.
  /// \param second Unicode code point of the second character.
  /// \param characterSize Reference character size.
  /// \return The kerning value for first and second, in pixels.
  [[nodiscard]] virtual float getKerning(unsigned int first,
                                         unsigned int second, unsigned int characterSize) const = 0;

  /// @brief Gets the texture containing the loaded glyphs of a certain size.
  /// \param characterSize Reference character size.
  /// \return Texture containing the glyphs of the requested size.
  [[nodiscard]] virtual const ngf::Texture &getTexture(unsigned int characterSize) const = 0;
};
}
