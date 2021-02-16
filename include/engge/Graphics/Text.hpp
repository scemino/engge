#pragma once
#include <string>
#include <vector>
#include <ngf/Graphics/Drawable.h>
#include <engge/Graphics/Font.h>
#include <ngf/Graphics/Vertex.h>
#include <ngf/Math/Transform.h>

namespace ng {
// This code has been copied from SFML Text and adapted to use Font.

/// @brief Graphical text that can be drawn to a render target
class Text : public ngf::Drawable {
public:
  /// @brief Creates an empty text.
  Text();
  /// @brief Creates a text from a string, font and size.
  Text(std::wstring string, const ng::Font &font, unsigned int characterSize = 30);

  /// @brief Sets the text's string.
  /// \param string The text's string.
  void setString(const std::string &string);
  /// @brief Gets the text's string.
  std::string getString() const;

  /// @brief Sets the text's string.
  /// \param string The text's string.
  void setWideString(const std::wstring &string);
  /// @brief Gets the text's string.
  std::wstring getWideString() const { return m_string; }

  /// @brief Sets the text's font.
  /// \param font The text's font.
  void setFont(const ng::Font &font);
  /// @brief Gets the text's font.
  /// \return The current text's font.
  const ng::Font *getFont() const;

  /// @brief Sets the maximum width allowed to display the text.
  /// \param maxWidth The maximum width allowed to display the text.
  void setMaxWidth(float maxWidth);
  /// @brief Gets the maximum width allowed to display the text.
  /// \return The maximum width allowed to display the text.
  float getMaxWidth() const { return m_maxWidth; }

  /// @brief Sets the color of the text.
  /// \param color The color of the text.
  void setColor(const ngf::Color &color);
  /// @brief Gets the color of the text.
  /// \return The color of the text.
  ngf::Color getColor() const { return m_color; }

  /// @brief Draws the text to the target with the specified render states.
  /// \param target This is where the drawing is made (a window, a texture, etc.)
  /// \param states Render states to use to draw this text.
  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const override;

  /// @brief Gets the local bounding rectangle of the text.
  /// \return The local bounding rectangle of the text.
  ngf::frect getLocalBounds() const;

  /// @brief Gets the transformation of the text.
  /// \return The transformation of the text.
  ngf::Transform &getTransform() { return m_transform; }
  /// @brief Gets the read-only transformation of the sprite.
  /// \return The read-only transformation of the sprite.
  const ngf::Transform &getTransform() const { return m_transform; }

  /// @brief Sets the origin of the transformation of the text.
  /// \param anchor The orgin of the text.
  void setAnchor(ngf::Anchor anchor);

private:
  void ensureGeometryUpdate() const;

private:
  ngf::Transform m_transform{};
  std::wstring m_string;
  const ng::Font *m_font{nullptr};
  unsigned int m_characterSize{0};
  mutable ngf::Color m_color{ngf::Colors::White};
  mutable std::vector<ngf::Vertex> m_vertices;
  mutable ngf::frect m_bounds{};
  mutable bool m_geometryNeedUpdate{false};
  mutable std::shared_ptr<ngf::Texture> m_fontTexture{nullptr};
  float m_maxWidth{0};
};
}
