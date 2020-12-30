#include "engge/Engine/EngineSettings.hpp"
#include "engge/Parsers/JsonTokenReader.hpp"
#include "engge/System/Locator.hpp"
#include "../System/_Util.hpp"
#include "engge/Graphics/SpriteSheet.hpp"

namespace ng {
void SpriteSheet::load(const std::string &name) {
  if (_textureName == name)
    return;

  _textureName = name;

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

ngf::irect SpriteSheet::getRect(const std::string &name) const {
  const auto it = _rects.find(name);
  return it->second;
}

ngf::irect SpriteSheet::getSpriteSourceSize(const std::string &name) const {
  const auto it = _spriteSourceSize.find(name);
  return it->second;
}

glm::ivec2 SpriteSheet::getSourceSize(const std::string &name) const {
  const auto it = _sourceSize.find(name);
  return it->second;
}

[[nodiscard]] SpriteSheetItem SpriteSheet::getItem(const std::string &name) const {
  return SpriteSheetItem{name, getRect(name), getSpriteSourceSize(name), getSourceSize(name)};
}

} // namespace ng
