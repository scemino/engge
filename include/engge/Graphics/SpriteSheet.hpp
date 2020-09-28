#pragma once
#include <string>
#include <map>
#include <memory>
#include "ResourceManager.hpp"

namespace ng {
class SpriteSheet {
public:
  void setTextureManager(ResourceManager *pTextureManager) { _pTextureManager = pTextureManager; }
  void load(const std::string &name);
  [[nodiscard]] const sf::Texture &getTexture() const { return *_texture; }
  [[nodiscard]] bool hasRect(const std::string &name) const;
  [[nodiscard]] sf::IntRect getRect(const std::string &name) const;
  [[nodiscard]] sf::IntRect getSpriteSourceSize(const std::string &name) const;
  [[nodiscard]] sf::Vector2i getSourceSize(const std::string &name) const;

private:
  ResourceManager *_pTextureManager{nullptr};
  std::shared_ptr<sf::Texture> _texture;
  std::map<std::string, sf::IntRect> _rects;
  std::map<std::string, sf::IntRect> _spriteSourceSize;
  std::map<std::string, sf::Vector2i> _sourceSize;
};
} // namespace ng
