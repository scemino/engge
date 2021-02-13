#pragma once
#include <string>
#include <map>
#include <memory>
#include "ResourceManager.hpp"
#include "SpriteSheetItem.h"

namespace ng {
class SpriteSheet {
public:
  void setTextureManager(ResourceManager *pTextureManager) { m_pResourceManager = pTextureManager; }
  void load(const std::string &name);
  [[nodiscard]] std::shared_ptr<ngf::Texture> getTexture() const { return m_pResourceManager->getTexture(m_textureName); }
  [[nodiscard]] std::string getTextureName() const { return m_textureName; }
  [[nodiscard]] bool hasRect(const std::string &name) const;
  [[nodiscard]] ngf::irect getRect(const std::string &name) const;
  [[nodiscard]] ngf::irect getSpriteSourceSize(const std::string &name) const;
  [[nodiscard]] glm::ivec2 getSourceSize(const std::string &name) const;
  [[nodiscard]] SpriteSheetItem getItem(const std::string &name) const;

private:
  ResourceManager *m_pResourceManager{nullptr};
  std::map<std::string, ngf::irect> m_rects;
  std::map<std::string, ngf::irect> m_spriteSourceSize;
  std::map<std::string, glm::ivec2> m_sourceSize;
  std::string m_textureName;
};
} // namespace ng
