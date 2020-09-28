#include <fstream>
#include "engge/Engine/EngineSettings.hpp"
#include "engge/Graphics/GGFont.hpp"
#include "engge/System/Locator.hpp"
#include "../System/_Util.hpp"

namespace ng {
GGFont::~GGFont() = default;

const sf::Texture &GGFont::getTexture(unsigned int) const { return *_texture; }

float GGFont::getKerning(sf::Uint32, sf::Uint32, unsigned int) const { return 0; }

int GGFont::getLineHeight() const {
  return 26;
}

const sf::Glyph &GGFont::getGlyph(sf::Uint32 codePoint, unsigned int, bool, float) const {
  if (_glyphs.find(codePoint) == _glyphs.end())
    return _glyphs.at(0x20);
  return _glyphs.at(codePoint);
}

void GGFont::setTextureManager(ResourceManager *textureManager) {
  _resourceManager = textureManager;
}

void GGFont::load(const std::string &path) {
  _path = path;
  _jsonFilename = path;
  _jsonFilename.append(".json");

  std::vector<char> buffer;
  Locator<EngineSettings>::get().readEntry(_jsonFilename, buffer);
  _json = ng::Json::Parser::parse(buffer);

#if 0
  std::ofstream o;
  o.open(_jsonFilename);
  o.write(buffer.data(), buffer.size());
  o.close();
#endif

  for (const auto &jFrame : _json["frames"].hash_value) {
    auto sValue = jFrame.first;
    auto key = std::stoi(sValue);
    auto frame = _toRect(_json["frames"][sValue]["frame"]);
    auto spriteSourceSize = _toRect(_json["frames"][sValue]["spriteSourceSize"]);
    auto sourceSize = _toSize(_json["frames"][sValue]["sourceSize"]);
    sf::Glyph glyph;
    glyph.advance = std::max(sourceSize.x - spriteSourceSize.left - 4, 0);
    glyph.bounds = (sf::FloatRect) spriteSourceSize;
    glyph.textureRect = frame;
    _glyphs[key] = glyph;
  }

  _texture = _resourceManager->getTexture(_path);
}
} // namespace ng
