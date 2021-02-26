#pragma once
#include <ngf/Graphics/Text.h>

namespace ng {
class Text : public ngf::Text {
public:
  /// @brief Creates an empty text.
  Text();
  /// @brief Creates a text from a string, font and size.
  Text(std::wstring string, const ngf::Font &font, unsigned int characterSize);
};
}