#include "Engine/EngineSettings.hpp"
#include "Font/FntFont.hpp"
#include "Font/GGFont.hpp"
#include "System/Locator.hpp"
#include "System/Logger.hpp"
#include "Graphics/TextureManager.hpp"

namespace ng {
TextureManager::TextureManager() = default;
TextureManager::~TextureManager() = default;

void TextureManager::load(const std::string &id) {
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

void TextureManager::loadFont(const std::string &id) {
  info("Load font {}", id);
  auto font = std::make_shared<GGFont>();
  font->setTextureManager(this);
  font->load(id);
  _fontMap.insert(std::make_pair(id, font));
}

void TextureManager::loadFntFont(const std::string &id) {
  info("Load Fnt font {}", id);
  auto font = std::make_shared<FntFont>();
  font->loadFromFile(id);
  _fntFontMap.insert(std::make_pair(id, font));
}

const sf::Texture &TextureManager::get(const std::string &id) {
  auto found = _textureMap.find(id);
  if (found == _textureMap.end()) {
    load(id);
    found = _textureMap.find(id);
  }
  return *found->second;
}

const GGFont &TextureManager::getFont(const std::string &id) {
  auto found = _fontMap.find(id);
  if (found == _fontMap.end()) {
    loadFont(id);
    found = _fontMap.find(id);
  }
  return *found->second;
}

const FntFont &TextureManager::getFntFont(const std::string &id) {
  auto found = _fntFontMap.find(id);
  if (found == _fntFontMap.end()) {
    loadFntFont(id);
    found = _fntFontMap.find(id);
  }
  return *found->second;
}
} // namespace ng
