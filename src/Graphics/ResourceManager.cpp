#include "Engine/EngineSettings.hpp"
#include "Font/FntFont.hpp"
#include "Font/GGFont.hpp"
#include "System/Locator.hpp"
#include "System/Logger.hpp"
#include "Graphics/ResourceManager.hpp"
#include "Graphics/SpriteSheet.hpp"

namespace ng {
ResourceManager::ResourceManager() = default;
ResourceManager::~ResourceManager() = default;

void ResourceManager::load(const std::string &id) {
  info("Load texture {}", id);
  std::string path;
  path.append(id).append(".png");
  auto texture = std::make_shared<sf::Texture>();
  std::vector<char> data;
  Locator<EngineSettings>::get().readEntry(path, data);

#if 0
  std::ofstream os(path, std::ios::out|std::ios::binary);
  os.write(data.data(), data.size());
  os.close();
#endif

  if (!texture->loadFromMemory(data.data(), data.size())) {
    error("Fail to load texture {}", path);
  }

  _textureMap.insert(std::make_pair(id, texture));
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
  auto font = std::make_shared<FntFont>();
  font->loadFromFile(id);
  _fntFontMap.insert(std::make_pair(id, font));
}

void ResourceManager::loadSpriteSheet(const std::string &id) {
  info("Load SpriteSheet {}", id);
  auto spriteSheet = std::make_shared<SpriteSheet>();
  spriteSheet->setTextureManager(this);
  spriteSheet->load(id);
  _spriteSheetMap.insert(std::make_pair(id, spriteSheet));
}

const sf::Texture &ResourceManager::get(const std::string &id) {
  auto found = _textureMap.find(id);
  if (found == _textureMap.end()) {
    load(id);
    found = _textureMap.find(id);
  }
  return *found->second;
}

const GGFont &ResourceManager::getFont(const std::string &id) {
  auto found = _fontMap.find(id);
  if (found == _fontMap.end()) {
    loadFont(id);
    found = _fontMap.find(id);
  }
  return *found->second;
}

const FntFont &ResourceManager::getFntFont(const std::string &id) {
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
