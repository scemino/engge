#include <fstream>
#include <ngf/IO/Json/JsonParser.h>
#include "engge/Engine/EngineSettings.hpp"
#include "engge/Graphics/GGFont.hpp"
#include "engge/System/Locator.hpp"
#include "../Util/Util.hpp"

namespace ng {
GGFont::~GGFont() = default;

const std::shared_ptr<ngf::Texture> &GGFont::getTexture(unsigned int) const { return m_texture; }

float GGFont::getKerning(unsigned int, unsigned int, unsigned int) const { return 0; }

const ngf::Glyph &GGFont::getGlyph(unsigned int codePoint) const {
  if (m_glyphs.find(codePoint) == m_glyphs.end())
    return m_glyphs.at(0x20);
  return m_glyphs.at(codePoint);
}

void GGFont::setTextureManager(ResourceManager *textureManager) {
  m_resourceManager = textureManager;
}

void GGFont::load(const std::string &path) {
  m_path = path + ".png";
  m_jsonFilename = path + ".json";

  auto buffer = Locator<EngineSettings>::get().readBuffer(m_jsonFilename);
  m_json = ngf::Json::parse(buffer.data());

#if 0
  std::ofstream o;
  o.open(_jsonFilename);
  o.write(buffer.data(), buffer.size());
  o.close();
#endif

  m_texture = m_resourceManager->getTexture(m_path);

  for (const auto &jFrame : m_json["frames"].items()) {
    auto sValue = jFrame.key();
    auto key = std::stoi(sValue);
    auto frame = toRect(m_json["frames"][sValue]["frame"]);
    auto spriteSourceSize = toRect(m_json["frames"][sValue]["spriteSourceSize"]);
    auto sourceSize = toSize(m_json["frames"][sValue]["sourceSize"]);
    ngf::Glyph glyph;
    glyph.advance = std::max(sourceSize.x - spriteSourceSize.getTopLeft().x - 4, 0);
    glyph.bounds = spriteSourceSize;
    glyph.textureRect = frame;
    m_glyphs[key] = glyph;
  }
}
} // namespace ng
