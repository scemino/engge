#include <ngf/IO/Json/JsonParser.h>
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

  ngf::GGPackValue json;

  std::string jsonFilename;
  jsonFilename.append(name).append(".json");
  {
    auto buffer = Locator<EngineSettings>::get().readBuffer(jsonFilename);

#if 0
    std::ofstream out;
    out.open(jsonFilename, std::ios::out);
    out.write(buffer.data(), buffer.size());
    out.close();
#endif
    json = ngf::Json::parse(buffer.data());
  }

  auto jFrames = json["frames"];
  for (auto &it : jFrames.items()) {
    auto rect = _toRect(it.value()["frame"]);
    _rects.insert(std::make_pair(it.key(), rect));
    rect = _toRect(it.value()["spriteSourceSize"]);
    _spriteSourceSize.insert(std::make_pair(it.key(), rect));
    auto size = _toSize(it.value()["sourceSize"]);
    _sourceSize.insert(std::make_pair(it.key(), size));
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
  return SpriteSheetItem{name, getRect(name), getSpriteSourceSize(name), getSourceSize(name), false};
}

} // namespace ng
