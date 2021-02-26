#include <engge/Graphics/Text.hpp>
#include <Engine/DebugFeatures.hpp>

namespace ng {
Text::Text() {
  showTextBounds(DebugFeatures::showTextBounds);
}

Text::Text(std::wstring string, const ngf::Font &font, unsigned int characterSize)
    : ngf::Text(string, font, characterSize) {
  showTextBounds(DebugFeatures::showTextBounds);
}
}