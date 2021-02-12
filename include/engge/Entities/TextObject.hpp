#pragma once
#include "Object.hpp"

namespace ng {
enum class TextAlignment : unsigned long {
  None = 0x00000000,
  Left = 0x10000000,
  Center = 0x20000000,
  Right = 0x40000000,
  Horizontal = Left | Center | Right,
  Top = 0x80000000,
  Bottom = 0x01000000,
  Vertical = Top | Bottom,
  All = Horizontal | Vertical
};

class Font;

class TextObject final : public Object {
public:
  TextObject();
  ~TextObject() final;

  const Font *getFont() { return m_font; }
  void setFont(const Font *font) { m_font = font; }
  void setText(const std::string &text);
  void setAlignment(TextAlignment alignment) { m_alignment = alignment; }
  void setMaxWidth(int maxWidth) { m_maxWidth = maxWidth; }

private:
  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const final;

private:
  mutable const Font *m_font{nullptr};
  std::wstring m_text;
  TextAlignment m_alignment{TextAlignment::Left};
  int m_maxWidth{0};
};
} // namespace ng
