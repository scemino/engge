#include <fstream>
#include <nlohmann/json.hpp>
#include "GGCostume.h"

namespace gg
{
// TODO: share
static sf::IntRect _toRect(const nlohmann::json &json)
{
    sf::IntRect rect;
    rect.left = json["x"].get<int>();
    rect.top = json["y"].get<int>();
    rect.width = json["w"].get<int>();
    rect.height = json["h"].get<int>();
    return rect;
}

GGCostume::GGCostume(const GGEngineSettings &settings)
    : _settings(settings), _facing(Facing::FACE_FRONT), _anim("stand")
{
}

GGCostume::~GGCostume()
{
}

void GGCostume::lockFacing(Facing facing)
{
    _facing = facing;
    updateAnim();
}

void GGCostume::setState(const std::string &name)
{
    _anim = name;
    updateAnim();
}

void GGCostume::loadCostume(const std::string &path)
{
    nlohmann::json json;
    nlohmann::json jSheet;
    {
        std::ifstream i(path);
        i >> json;
    }
    _sheet = json["sheet"].get<std::string>();

    std::string sheetPath(_settings.getGamePath());
    sheetPath.append(_sheet).append(".json");
    {
        std::ifstream i(sheetPath);
        i >> jSheet;
    }

    // load tetxure
    std::string sheetPng(_settings.getGamePath());
    sheetPng.append(_sheet).append(".png");
    _texture.loadFromFile(sheetPng);

    // create animations
    _animations.clear();
    for (auto j : json["animations"])
    {
        auto layers = new std::vector<GGLayer>();
        auto name = j["name"].get<std::string>();
        for (auto jLayer : j["layers"])
        {
            auto layer = new GGLayer();
            auto fps = jLayer["fps"].is_null() ? 10 : jLayer["fps"].get<int>();
            layer->setFps(fps);
            auto layerName = jLayer["name"].get<std::string>();
            for (auto jFrame : jLayer["frames"])
            {
                auto frameName = jFrame.get<std::string>();
                if (frameName == "null")
                {
                    layer->getFrames().push_back(sf::IntRect());
                    layer->getSourceFrames().push_back(sf::IntRect());
                }
                else
                {
                    auto jFrames = jSheet["frames"];
                    auto jf = jFrames[frameName.c_str()];
                    layer->getFrames().push_back(_toRect(jf["frame"]));
                    layer->getSourceFrames().push_back(_toRect(jf["spriteSourceSize"]));
                }
            }
            layers->push_back(*layer);
        }
        printf("foudn anim: %s\n", name.c_str());
        _animations.insert(std::make_pair(name, std::unique_ptr<std::vector<GGLayer>>(layers)));
    }
}

void GGCostume::updateAnim()
{
    std::string name(_anim);
    name.append("_");
    switch (_facing)
    {
    case Facing::FACE_BACK:
        name.append("back");
        break;
    case Facing::FACE_FRONT:
        name.append("front");
        break;
    case Facing::FACE_LEFT:
        name.append("left");
        break;
    case Facing::FACE_RIGHT:
        name.append("right");
        break;
    }
    setAnim(name);
}

void GGCostume::setAnim(const std::string &name)
{
    auto iter = _animations.find(name);
    if (iter == _animations.end())
        return;
    _pCurrentAnim = iter->second.get();
}

void GGCostume::update(const sf::Time &elapsed)
{
    for (int i = 0; i < _pCurrentAnim->size(); i++)
    {
        (*_pCurrentAnim)[i].update(elapsed);
    }

    _sprites.clear();
    for (int i = 0; i < _pCurrentAnim->size(); i++)
    {
        auto frame = (*_pCurrentAnim)[i].getIndex();
        auto &rect = (*_pCurrentAnim)[i].getFrames()[frame];
        auto &sourceRect = (*_pCurrentAnim)[i].getSourceFrames()[frame];
        sf::Sprite sprite(_texture, rect);
        sprite.setOrigin(-sourceRect.left, -sourceRect.top);
        _sprites.push_back(sprite);
    }
}

void GGCostume::draw(sf::RenderWindow &window) const
{
    for (auto sprite : _sprites)
    {
        window.draw(sprite);
    }
}
} // namespace gg
