#include <fstream>
#include "engge/Engine/EngineSettings.hpp"
#include "engge/Graphics/GGFont.hpp"
#include "engge/System/Locator.hpp"
#include "../System/_Util.hpp"

namespace ng {
GGFont::~GGFont() = default;

const ngf::Texture &GGFont::getTexture(unsigned int) const { return *_texture; }

float GGFont::getKerning(unsigned int, unsigned int, unsigned int) const { return 0; }

const Glyph &GGFont::getGlyph(unsigned int codePoint) const {
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

  _texture = _resourceManager->getTexture(_path);

  for (const auto &jFrame : _json["frames"].hash_value) {
    auto sValue = jFrame.first;
    auto key = std::stoi(sValue);
    auto frame = _toRect(_json["frames"][sValue]["frame"]);
    auto spriteSourceSize = _toRect(_json["frames"][sValue]["spriteSourceSize"]);
    auto sourceSize = _toSize(_json["frames"][sValue]["sourceSize"]);
    Glyph glyph;
    glyph.advance = std::max(sourceSize.x - spriteSourceSize.getTopLeft().x - 4, 0);
    glyph.bounds = spriteSourceSize;
    glyph.textureRect = frame;
    _glyphs[key] = glyph;
  }

}
} // namespace ng
