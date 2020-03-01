#pragma once
#include "SFML/Graphics.hpp"
#include "Parsers/JsonTokenReader.hpp"
#include "Graphics/TextureManager.hpp"

namespace ng
{
class Font
{
public:
  void setTextureManager(TextureManager *textureManager);

  void load(const std::string &path);

  const sf::Texture &getTexture() const { return _texture; }
  sf::IntRect getRect(uint32_t letter) const;
  sf::IntRect getSourceSize(uint32_t letter) const;

private:
  TextureManager *_textureManager{nullptr};
  std::string _path;
  std::string _jsonFilename;
  ng::GGPackValue _json;
  sf::Texture _texture;
};

} // namespace ng