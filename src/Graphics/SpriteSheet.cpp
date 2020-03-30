#include "Engine/EngineSettings.hpp"
#include "Parsers/JsonTokenReader.hpp"
#include "System/Locator.hpp"
#include "../System/_Util.hpp"
#include "Graphics/SpriteSheet.hpp"

namespace ng {
SpriteSheet::SpriteSheet() = default;

void SpriteSheet::load(const std::string &name) {
  _texture = _pTextureManager->get(name);

  _rects.clear();
  _spriteSourceSize.clear();
  _sourceSize.clear();

  ng::GGPackValue json;

  std::string jsonFilename;
  jsonFilename.append(name).append(".json");
  {
    std::vector<char> buffer;
    Locator<EngineSettings>::get().readEntry(jsonFilename, buffer);

#if 0
    std::ofstream out;
    out.open(jsonFilename, std::ios::out);
    out.write(buffer.data(), buffer.size());
    out.close();
#endif
    ng::Json::Parser::parse(buffer, json);
  }

  auto jFrames = json["frames"];
  for (auto &it : jFrames.hash_value) {
    auto &n = it.first;
    auto rect = _toRect(json["frames"][n]["frame"]);
    _rects.insert(std::make_pair(n, rect));
    rect = _toRect(json["frames"][n]["spriteSourceSize"]);
    _spriteSourceSize.insert(std::make_pair(n, rect));
    auto size = _toSize(json["frames"][n]["sourceSize"]);
    _sourceSize.insert(std::make_pair(n, size));
  }
}

bool SpriteSheet::hasRect(const std::string &name) const {
  const auto it = _rects.find(name);
  return it != _rects.end();
}

sf::IntRect SpriteSheet::getRect(const std::string &name) const {
  const auto it = _rects.find(name);
  return it->second;
}

sf::IntRect SpriteSheet::getSpriteSourceSize(const std::string &name) const {
  const auto it = _spriteSourceSize.find(name);
  return it->second;
}

sf::Vector2i SpriteSheet::getSourceSize(const std::string &name) const {
  const auto it = _sourceSize.find(name);
  return it->second;
}

} // namespace ng
