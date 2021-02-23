#pragma once
#include <map>
#include <memory>
#include <engge/System/NonCopyable.hpp>
#include <ngf/Graphics/Texture.h>

namespace ngf {
class FntFont;
}

namespace ng {
class GGFont;
class SpriteSheet;

struct TextureResource {
  std::shared_ptr<ngf::Texture> texture;
  size_t size;
};

class ResourceManager : public NonCopyable {
public:
  ResourceManager();
  ~ResourceManager();

  std::shared_ptr<ngf::Texture> getTexture(const std::string &id);
  GGFont &getFont(const std::string &id);
  ngf::FntFont &getFntFont(const std::string &id);
  const SpriteSheet &getSpriteSheet(const std::string &id);

  [[nodiscard]] const std::map<std::string, TextureResource> &getTextureMap() const { return m_textureMap; }

private:
  void load(const std::string &id);
  void loadFont(const std::string &id);
  void loadFntFont(const std::string &id);
  void loadSpriteSheet(const std::string &id);

private:
  std::map<std::string, TextureResource> m_textureMap;
  std::map<std::string, std::shared_ptr<GGFont>> m_fontMap;
  std::map<std::string, std::shared_ptr<ngf::FntFont>> m_fntFontMap;
  std::map<std::string, std::shared_ptr<SpriteSheet>> m_spriteSheetMap;
};
} // namespace ng