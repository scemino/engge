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
  std::string path;
  path.append(id).append(".png");
  std::vector<char> data;
  Locator<EngineSettings>::get().readEntry(path, data);

#if 0
  std::ofstream os(path, std::ios::out|std::ios::binary);
  os.write(data.data(), data.size());
  os.close();
#endif

  ngf::Image img;
  if (!img.loadFromMemory(data.data(), data.size())) {
    error("Fail to load texture {}", path);
  }

  auto texture = std::make_shared<ngf::Texture>(img);
  _textureMap.insert(std::make_pair(id, TextureResource{texture, data.size()}));
}

void ResourceManager::loadFont(const std::string &id) {
  info("Load font {}", id);
  auto font = std::make_shared<GGFont>();
  font->setTextureManager(this);
  font->load(id);
  _fontMap.insert(std::make_pair(id, font));
}

void ResourceManager::loadFntFont(const std::string &id) {
  info("Load Fnt font {}", id);
  auto font = std::make_shared<ngf::FntFont>();

  std::filesystem::path path = id;
  path = path.replace_extension(".png").u8string();

  std::vector<char> data;
  Locator<EngineSettings>::get().readEntry(path, data);
  ngf::Image img;
  img.loadFromMemory(data.data(), data.size());
  font->setTexture(img);

  Locator<EngineSettings>::get().readEntry(id, data);
  ngf::MemoryStream ms(data.data(), data.data() + data.size());
  font->load(id, ms);

  _fntFontMap.insert(std::make_pair(id, font));
}

void ResourceManager::loadSpriteSheet(const std::string &id) {
  info("Load SpriteSheet {}", id);
  auto spriteSheet = std::make_shared<SpriteSheet>();
  spriteSheet->setTextureManager(this);
  spriteSheet->load(id);
  _spriteSheetMap.insert(std::make_pair(id, spriteSheet));
}

std::shared_ptr<ngf::Texture> ResourceManager::getTexture(const std::string &id) {
  auto found = _textureMap.find(id);
  if (found == _textureMap.end()) {
    load(id);
    found = _textureMap.find(id);
  }
  return found->second._texture;
}

GGFont &ResourceManager::getFont(const std::string &id) {
  auto found = _fontMap.find(id);
  if (found == _fontMap.end()) {
    loadFont(id);
    found = _fontMap.find(id);
  }
  return *found->second;
}

ngf::FntFont &ResourceManager::getFntFont(const std::string &id) {
  auto found = _fntFontMap.find(id);
  if (found == _fntFontMap.end()) {
    loadFntFont(id);
    found = _fntFontMap.find(id);
  }
  return *found->second;
}

const SpriteSheet &ResourceManager::getSpriteSheet(const std::string &id) {
  auto found = _spriteSheetMap.find(id);
  if (found == _spriteSheetMap.end()) {
    loadSpriteSheet(id);
    found = _spriteSheetMap.find(id);
  }
  return *found->second;
}

} // namespace ng
