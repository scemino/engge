#pragma once
#include "Object.hpp"

namespace ngf {
class Font;
}

namespace ng {
enum class TextAlignment : unsigned long {
  None =   0x00000000,
  Left =   0x00000001,
  Center = 0x00000002,
  Right =  0x00000004,
  Horizontal = Left | Center | Right,
  Top =    0x00000008,
  Bottom = 0x00000010,
  Vertical = Top | Bottom,
};

class TextObject final : public Object {
public:
  TextObject();
  ~TextObject() final;

  const ngf::Font *getFont() { return m_font; }
  void setFont(const ngf::Font *font) { m_font = font; }

  void setText(const std::string &text);
  std::string getText() const;

  void setAlignment(TextAlignment alignment) { m_alignment = alignment; }
  TextAlignment getAlignment() const { return m_alignment; }

  void setMaxWidth(int maxWidth) { m_maxWidth = maxWidth; }
  int getMaxWidth() const { return m_maxWidth; }

private:
  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const final;

private:
  mutable const ngf::Font *m_font{nullptr};
  std::wstring m_text;
  TextAlignment m_alignment{TextAlignment::Left};
  int m_maxWidth{0};
};
} // namespace ng
