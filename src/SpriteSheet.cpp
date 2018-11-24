#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
#include "_GGUtil.h"
#include "SpriteSheet.h"

namespace gg
{
void SpriteSheet::load(const std::string &name)
{
    _texture = _textureManager.get(name);

    _rects.clear();
    nlohmann::json json;
    std::string jsonFilename;
    jsonFilename.append(_settings.getGamePath()).append(name).append(".json");
    {
        std::ifstream i(jsonFilename);
        i >> json;
    }

    auto jFrames = json["frames"];
    for (auto it = jFrames.begin(); it != jFrames.end(); ++it)
    {
        auto n = it.key();
        auto rect = _toRect(json["frames"][n]["frame"]);
        std::cout << "frame " << n << " (" << rect.width << "," << rect.height << ")" << std::endl;
        _rects.insert(std::pair(n, rect));
    }
}

sf::IntRect SpriteSheet::getRect(const std::string &name) const
{
    const auto it = _rects.find(name);
    return it->second;
}

} // namespace gg
