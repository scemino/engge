#pragma once
#include "engge/Parsers/JsonTokenReader.hpp"
#include "engge/Graphics/ResourceManager.hpp"
#include <ngf/Graphics/Font.h>
#include <memory>

namespace ng {
class GGFont : public ngf::Font {
public:
  ~GGFont() override;
  void setTextureManager(ResourceManager *textureManager);

  void load(const std::string &path);

  [[nodiscard]] const ngf::Texture *getTexture(unsigned int) override;
  [[nodiscard]] const ngf::Glyph &getGlyph(unsigned int codePoint,
                                           unsigned int characterSize,
                                           float outlineThickness) override;
  [[nodiscard]] float getKerning(unsigned int first,
                                 unsigned int second, unsigned int characterSize) override { return 0.f; }
  float getLineSpacing(unsigned int characterSize) override { return 16.f; }

private:
  std::map<unsigned int, ngf::Glyph> _glyphs;
  ResourceManager *_resourceManager{nullptr};
  std::string _path;
  std::string _jsonFilename;
  ng::GGPackValue _json;
  std::shared_ptr<ngf::Texture> _texture;
};

} // namespace ng