#include "engge/Engine/EngineSettings.hpp"
#include "engge/Graphics/GGFont.hpp"
#include "engge/System/Locator.hpp"
#include "engge/System/Logger.hpp"
#include "engge/Graphics/ResourceManager.hpp"
#include "engge/Graphics/SpriteSheet.hpp"
#include <ngf/Graphics/FntFont.h>
#include <ngf/IO/MemoryStream.h>

namespace ng {
ResourceManager::ResourceManager() = default;
ResourceManager::~ResourceManager() = default;

void ResourceManager::load(const std::string &id) {
  info("Load texture {}", id);
  auto data = Locator<EngineSettings>::get().readBuffer(id);

#if 0
  std::ofstream os(path, std::ios::out|std::ios::binary);
  os.write(data.data(), data.size());
  os.close();
#endif

  ngf::Image img;
  if (!img.loadFromMemory(data.data(), data.size())) {
    error("Fail to load texture {}", id);
  }

  auto texture = std::make_shared<ngf::Texture>(img);
  m_textureMap.insert(std::make_pair(id, TextureResource{texture, data.size()}));
}

void ResourceManager::loadFont(const std::string &id) {
  info("Load font {}", id);
  auto font = std::make_shared<GGFont>();
  font->setTextureManager(this);
  font->load(id);
  m_fontMap.insert(std::make_pair(id, font));
}

void ResourceManager::loadFntFont(const std::string &id) {
  info("Load Fnt font {}", id);
  auto font = std::make_shared<ngf::FntFont>();

  auto data = Locator<EngineSettings>::get().readBuffer(id);
  ngf::MemoryStream ms(data.data(), data.data() + data.size());
  font->load(id, ms, [](auto name) {
    return Locator<ResourceManager>::get().getTexture(name.string());
  });

  m_fntFontMap.insert(std::make_pair(id, font));
}

void ResourceManager::loadSpriteSheet(const std::string &id) {
  info("Load SpriteSheet {}", id);
  auto spriteSheet = std::make_shared<SpriteSheet>();
  spriteSheet->setTextureManager(this);
  spriteSheet->load(id);
  m_spriteSheetMap.insert(std::make_pair(id, spriteSheet));
}

std::shared_ptr<ngf::Texture> ResourceManager::getTexture(const std::string &id) {
  auto found = m_textureMap.find(id);
  if (found == m_textureMap.end()) {
    load(id);
    found = m_textureMap.find(id);
  }
  return found->second.texture;
}

GGFont &ResourceManager::getFont(const std::string &id) {
  auto found = m_fontMap.find(id);
  if (found == m_fontMap.end()) {
    loadFont(id);
    found = m_fontMap.find(id);
  }
  return *found->second;
}

ngf::FntFont &ResourceManager::getFntFont(const std::string &id) {
  auto found = m_fntFontMap.find(id);
  if (found == m_fntFontMap.end()) {
    loadFntFont(id);
    found = m_fntFontMap.find(id);
  }
  return *found->second;
}

const SpriteSheet &ResourceManager::getSpriteSheet(const std::string &id) {
  auto found = m_spriteSheetMap.find(id);
  if (found == m_spriteSheetMap.end()) {
    loadSpriteSheet(id);
    found = m_spriteSheetMap.find(id);
  }
  return *found->second;
}

} // namespace ng
