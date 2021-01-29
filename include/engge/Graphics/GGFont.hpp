#pragma once
#include "engge/Parsers/JsonTokenReader.hpp"
#include "engge/Graphics/ResourceManager.hpp"
#include "Font.h"
#include <memory>

namespace ng {
class GGFont : public Font {
public:
  ~GGFont() override;
  void setTextureManager(ResourceManager *textureManager);

  void load(const std::string &path);

  [[nodiscard]] const std::shared_ptr<ngf::Texture> &getTexture(unsigned int) const override;
  [[nodiscard]] const Glyph &getGlyph(unsigned int codePoint) const override;
  [[nodiscard]] float getKerning(unsigned int first, unsigned int second, unsigned int characterSize) const override;

private:
  std::map<unsigned int, Glyph> _glyphs;
  ResourceManager *_resourceManager{nullptr};
  std::string _path;
  std::string _jsonFilename;
  ng::GGPackValue _json;
  std::shared_ptr<ngf::Texture> _texture;
};

} // namespace ng