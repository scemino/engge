#pragma once
#include <string>
#include <map>
#include <memory>
#include "ResourceManager.hpp"

namespace ng {
class SpriteSheet {
public:
  void setTextureManager(ResourceManager *pTextureManager) { _pResourceManager = pTextureManager; }
  void load(const std::string &name);
  [[nodiscard]] const ngf::Texture &getTexture() const { return *_pResourceManager->getTexture(_textureName); }
  [[nodiscard]] std::string getTextureName() const { return _textureName; }
  [[nodiscard]] bool hasRect(const std::string &name) const;
  [[nodiscard]] ngf::irect getRect(const std::string &name) const;
  [[nodiscard]] ngf::irect getSpriteSourceSize(const std::string &name) const;
  [[nodiscard]] glm::ivec2 getSourceSize(const std::string &name) const;

private:
  ResourceManager *_pResourceManager{nullptr};
  std::map<std::string, ngf::irect> _rects;
  std::map<std::string, ngf::irect> _spriteSourceSize;
  std::map<std::string, glm::ivec2> _sourceSize;
  std::string _textureName;
};
} // namespace ng
