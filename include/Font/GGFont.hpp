#pragma once
#include "SFML/Graphics.hpp"
#include "Parsers/JsonTokenReader.hpp"
#include "Graphics/ResourceManager.hpp"
#include "Font.hpp"
#include <memory>

namespace ng {
class GGFont : public Font {
public:
  ~GGFont() override;
  void setTextureManager(ResourceManager *textureManager);

  void load(const std::string &path);

  const sf::Texture &getTexture(unsigned int) const override;
  int getLineHeight() const override;
  const sf::Glyph &getGlyph(sf::Uint32 codePoint,
                            unsigned int characterSize,
                            bool bold,
                            float outlineThickness) const override;
  float getKerning(sf::Uint32 first, sf::Uint32 second, unsigned int characterSize) const override;

private:
  std::map<sf::Uint32, sf::Glyph> _glyphs;
  ResourceManager *_resourceManager{nullptr};
  std::string _path;
  std::string _jsonFilename;
  ng::GGPackValue _json;
  std::shared_ptr<sf::Texture> _texture;
};

} // namespace ng