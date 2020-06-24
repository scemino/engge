#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "Font/FntFont.hpp"

namespace ng {
// This code has been copied from SFML Text and adapted to us FntFont
class Text : public sf::Drawable, public sf::Transformable {
public:
  Text();
  Text(const sf::String &string, const FntFont &font, unsigned int characterSize = 30);

  void setString(const sf::String &string);
  void setFont(const Font &font);
  void setFillColor(const sf::Color &color);
  void setMaxWidth(float maxWidth);

  const sf::String &getString() const;
  const Font *getFont() const;
  const sf::Color &getFillColor() const;
  sf::FloatRect getLocalBounds() const;
  sf::FloatRect getGlobalBounds() const;

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

  void ensureGeometryUpdate() const;

  sf::String m_string;              ///< String to display
  const Font *m_font;                ///< GGFont used to display the string
  unsigned int m_characterSize;       ///< Base size of characters, in pixels
  mutable sf::Color m_fillColor;           ///< Text fill color
  sf::Color m_outlineColor;        ///< Text outline color
  mutable sf::VertexArray m_vertices;            ///< Vertex array containing the fill geometry
  mutable sf::FloatRect m_bounds;              ///< Bounding rectangle of the text (in local coordinates)
  mutable bool m_geometryNeedUpdate;  ///< Does the geometry need to be recomputed?
  mutable const sf::Texture *m_fontTextureId;       ///< The font texture id
  float m_maxWidth{0};
};

} // namespace sf
