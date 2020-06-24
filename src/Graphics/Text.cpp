#include <cmath>
#include "Graphics/Text.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "../System/_Util.hpp"

namespace {
// Add a glyph quad to the vertex array
void addGlyphQuad(sf::VertexArray &vertices,
                  sf::Vector2f position,
                  const sf::Color &color,
                  const sf::Glyph &glyph) {
  float padding = 1.0;

  float left = glyph.bounds.left - padding;
  float top = glyph.bounds.top - padding;
  float right = glyph.bounds.left + glyph.bounds.width + padding;
  float bottom = glyph.bounds.top + glyph.bounds.height + padding;

  float u1 = static_cast<float>(glyph.textureRect.left) - padding;
  float v1 = static_cast<float>(glyph.textureRect.top) - padding;
  float u2 = static_cast<float>(glyph.textureRect.left + glyph.textureRect.width) + padding - 1;
  float v2 = static_cast<float>(glyph.textureRect.top + glyph.textureRect.height) + padding - 1;

  vertices.append(sf::Vertex(sf::Vector2f(position.x + left,
                                          position.y + top), color, sf::Vector2f(u1, v1)));
  vertices.append(sf::Vertex(sf::Vector2f(position.x + right,
                                          position.y + top), color, sf::Vector2f(u2, v1)));
  vertices.append(sf::Vertex(sf::Vector2f(position.x + left,
                                          position.y + bottom), color, sf::Vector2f(u1, v2)));
  vertices.append(sf::Vertex(sf::Vector2f(position.x + left,
                                          position.y + bottom), color, sf::Vector2f(u1, v2)));
  vertices.append(sf::Vertex(sf::Vector2f(position.x + right,
                                          position.y + top), color, sf::Vector2f(u2, v1)));
  vertices.append(sf::Vertex(sf::Vector2f(position.x + right,
                                          position.y + bottom), color, sf::Vector2f(u2, v2)));
}
} // namespace

namespace ng {
////////////////////////////////////////////////////////////
Text::Text() : m_string(),
               m_font(nullptr),
               m_characterSize(0),
               m_fillColor(255, 255, 255),
               m_outlineColor(0, 0, 0),
               m_vertices(sf::Triangles),
               m_bounds(),
               m_geometryNeedUpdate(false),
               m_fontTextureId(0) {
}

////////////////////////////////////////////////////////////
Text::Text(const sf::String &string, const FntFont &font, unsigned int characterSize)
    : m_string(string),
      m_font(&font),
      m_characterSize(characterSize),
      m_fillColor(255, 255, 255),
      m_outlineColor(0, 0, 0),
      m_vertices(sf::Triangles),
      m_bounds(),
      m_geometryNeedUpdate(true),
      m_fontTextureId(0) {
}

////////////////////////////////////////////////////////////
void Text::setString(const sf::String &string) {
  if (m_string != string) {
    m_string = string;
    m_geometryNeedUpdate = true;
  }
}

////////////////////////////////////////////////////////////
void Text::setFont(const Font &font) {
  if (m_font != &font) {
    m_font = &font;
    m_geometryNeedUpdate = true;
  }
}

////////////////////////////////////////////////////////////
void Text::setMaxWidth(float maxWidth) {
  if (m_maxWidth != maxWidth) {
    m_maxWidth = maxWidth;
    m_geometryNeedUpdate = true;
  }
}

void Text::setFillColor(const sf::Color &color) {
  if (color != m_fillColor) {
    m_fillColor = color;

    // Change vertex colors directly, no need to update whole geometry
    // (if geometry is updated anyway, we can skip this step)
    if (!m_geometryNeedUpdate) {
      for (std::size_t i = 0; i < m_vertices.getVertexCount(); ++i)
        m_vertices[i].color = m_fillColor;
    }
  }
}

////////////////////////////////////////////////////////////
const sf::String &Text::getString() const {
  return m_string;
}

////////////////////////////////////////////////////////////
const Font *Text::getFont() const {
  return m_font;
}

////////////////////////////////////////////////////////////
const sf::Color &Text::getFillColor() const {
  return m_fillColor;
}

sf::FloatRect Text::getLocalBounds() const {
  ensureGeometryUpdate();

  return m_bounds;
}

sf::FloatRect Text::getGlobalBounds() const {
  return getTransform().transformRect(getLocalBounds());
}

////////////////////////////////////////////////////////////
void Text::draw(sf::RenderTarget &target, sf::RenderStates states) const {
  if (m_font) {
    ensureGeometryUpdate();

    states.transform *= getTransform();
    states.texture = &m_font->getTexture(m_characterSize);

    target.draw(m_vertices, states);
  }
}

////////////////////////////////////////////////////////////
void Text::ensureGeometryUpdate() const {
  if (!m_font)
    return;

  // Do nothing, if geometry has not changed and the font texture has not changed
  if (!m_geometryNeedUpdate && &m_font->getTexture(m_characterSize) == m_fontTextureId)
    return;

  // Save the current fonts texture id
  m_fontTextureId = &m_font->getTexture(m_characterSize);

  // Mark geometry as updated
  m_geometryNeedUpdate = false;

  // Clear the previous geometry
  m_vertices.clear();
  m_bounds = sf::FloatRect();

  // No text: nothing to draw
  if (m_string.isEmpty())
    return;

  // Precompute the variables needed by the algorithm
  float whitespaceWidth = m_font->getGlyph(L' ', m_characterSize, false).advance;
  float x = 0.f;
  float y = 0.f;

  // Create one quad for each character
  float maxX = 0.f;
  float maxY = 0.f;
  float maxLineY = 0.f;
  sf::Uint32 prevChar = 0;

  struct CharInfo {
    sf::Uint32 chr;
    sf::Vector2f pos;
    sf::Color color;
    const sf::Glyph* pGlyph{nullptr};
  };

  int lastWordIndex = 0;
  std::vector<CharInfo> charInfos(m_string.getSize());
  for (auto i = 0; i < static_cast<int>(m_string.getSize()); ++i) {
    sf::Uint32 curChar = m_string[i];
    charInfos[i].chr = curChar;
    charInfos[i].pos = sf::Vector2f(x, y);
    charInfos[i].color = m_fillColor;

    // Skip the \r char to avoid weird graphical issues
    if (curChar == '\r')
      continue;

    // Apply the kerning offset
    x += m_font->getKerning(prevChar, curChar, m_characterSize);

    prevChar = curChar;

    // Handle special characters
    if ((curChar == L' ') || (curChar == L'\n') || (curChar == L'\t') || (curChar == L'#')) {
      if (m_maxWidth && x >= m_maxWidth) {
        y += maxLineY;
        //maxLineY = 0;
        x = 0;
        i = lastWordIndex;
        continue;
      }

      switch (curChar) {
      case L' ':
        x += whitespaceWidth;
        lastWordIndex = i;
        break;
      case L'\t':
        x += whitespaceWidth * 2;
        lastWordIndex = i;
        break;
      case L'\n':
        y += maxLineY;
        lastWordIndex = i;
        x = 0;
        //maxLineY = 0;
        break;
      case L'#':
        auto strColor = m_string.substring(i + 1, 6);
        auto color = _toColor(strColor.toAnsiString());
        if (color != m_fillColor) {
          m_fillColor = color;
        }
        i += 7;
        break;
      }

      // Next glyph, no need to create a quad for whitespace
      continue;
    }

    // Extract the current glyph's description
    const auto &glyph = m_font->getGlyph(curChar, m_characterSize, false);
    charInfos[i].pGlyph = &glyph;
    maxLineY = std::max(maxLineY, glyph.bounds.height);

    // Advance to the next character
    x += glyph.advance;
  }

  for (auto i = 0; i < static_cast<int>(m_string.getSize()); ++i) {
    auto info = charInfos[i];

    // Skip the \r char to avoid weird graphical issues
    if (info.chr == '\r')
      continue;

    // Handle special characters
    if ((info.chr == L' ') || (info.chr == L'\n') || (info.chr == L'\t') || (info.chr == L'#')) {
      switch (info.chr) {
      case L' ':
        break;
      case L'\t':
        break;
      case L'\n':
        break;
      case L'#':
        i += 7;
        break;
      }

      // Next glyph, no need to create a quad for whitespace
      continue;
    }

    // Add the glyph to the vertices
    addGlyphQuad(m_vertices, info.pos, info.color, *charInfos[i].pGlyph);

    // Update the current bounds with the non outlined glyph bounds
    maxX = std::max(maxX, info.pos.x + charInfos[i].pGlyph->bounds.width);
    maxY = std::max(maxY, charInfos[i].pGlyph->bounds.height);
  }

  // Update the bounding rectangle
  m_bounds.left = 0;
  m_bounds.top = maxY / 2.f;
  m_bounds.width = maxX;
  m_bounds.height = maxY;
}

} // namespace ng
