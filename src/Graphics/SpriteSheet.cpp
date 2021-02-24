#include <ngf/IO/Json/JsonParser.h>
#include "engge/Engine/EngineSettings.hpp"
#include "engge/System/Locator.hpp"
#include "../Util/Util.hpp"
#include "engge/Graphics/SpriteSheet.hpp"

namespace ng {
void SpriteSheet::load(const std::string &name) {
  if (m_textureName == name)
    return;

  m_textureName = name + ".png";

  m_rects.clear();
  m_spriteSourceSize.clear();
  m_sourceSize.clear();

  ngf::GGPackValue json;

  {
    auto jsonFilename = name + ".json";
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
    auto rect = toRect(it.value()["frame"]);
    m_rects.insert(std::make_pair(it.key(), rect));
    rect = toRect(it.value()["spriteSourceSize"]);
    m_spriteSourceSize.insert(std::make_pair(it.key(), rect));
    auto size = toSize(it.value()["sourceSize"]);
    m_sourceSize.insert(std::make_pair(it.key(), size));
  }
}

bool SpriteSheet::hasRect(const std::string &name) const {
  const auto it = m_rects.find(name);
  return it != m_rects.end();
}

ngf::irect SpriteSheet::getRect(const std::string &name) const {
  const auto it = m_rects.find(name);
  return it->second;
}

ngf::irect SpriteSheet::getSpriteSourceSize(const std::string &name) const {
  const auto it = m_spriteSourceSize.find(name);
  return it->second;
}

glm::ivec2 SpriteSheet::getSourceSize(const std::string &name) const {
  const auto it = m_sourceSize.find(name);
  return it->second;
}

[[nodiscard]] SpriteSheetItem SpriteSheet::getItem(const std::string &name) const {
  return SpriteSheetItem{name, getRect(name), getSpriteSourceSize(name), getSourceSize(name), false};
}

} // namespace ng
