#pragma once
#include "Object.hpp"
#include <ngf/Graphics/FntFont.h>

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

class TextObject : public Object {
public:
  TextObject();
  ~TextObject() override;

  ngf::FntFont *getFont() { return _font; }
  void setFont(ngf::FntFont* font) { _font = font; }
  void setText(const std::string &text);
  void setAlignment(TextAlignment alignment) { _alignment = alignment; }
  void setMaxWidth(int maxWidth) { _maxWidth = maxWidth; }

private:
  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const override;

private:
  mutable ngf::FntFont *_font{nullptr};
  std::wstring _text;
  TextAlignment _alignment;
  int _maxWidth{0};
};
} // namespace ng
