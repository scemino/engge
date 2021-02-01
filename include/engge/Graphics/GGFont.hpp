#pragma once
#include <ngf/IO/GGPackValue.h>
#include <engge/Graphics/ResourceManager.hpp>
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
  std::map<unsigned int, Glyph> m_glyphs;
  ResourceManager *m_resourceManager{nullptr};
  std::string m_path;
  std::string m_jsonFilename;
  ngf::GGPackValue m_json;
  std::shared_ptr<ngf::Texture> m_texture;
};

} // namespace ng