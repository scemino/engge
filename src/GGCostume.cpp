#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include "GGCostume.h"
#include "_GGUtil.h"

namespace gg
{
GGCostume::GGCostume(const GGEngineSettings &settings)
    : _settings(settings),
      _pCurrentAnimation(nullptr),
      _facing(Facing::FACE_FRONT),
      _animation("stand")
{
}

GGCostume::~GGCostume() = default;

void GGCostume::lockFacing(Facing facing)
{
    _facing = facing;
    updateAnimation();
}

void GGCostume::setState(const std::string &name)
{
    _animation = name;
    updateAnimation();
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

    // load texture
    std::string sheetPng(_settings.getGamePath());
    sheetPng.append(_sheet).append(".png");
    _texture.loadFromFile(sheetPng);

    // create animations
    _animations.clear();
    for (auto j : json["animations"])
    {
        auto name = j["name"].get<std::string>();
        auto anim = new GGCostumeAnimation(name, _texture);
        for (auto jLayer : j["layers"])
        {
            auto layer = new GGLayer();
            auto fps = jLayer["fps"].is_null() ? 10 : jLayer["fps"].get<int>();
            layer->setFps(fps);
            auto layerName = jLayer["name"].get<std::string>();
            for (const auto &jFrame : jLayer["frames"])
            {
                auto frameName = jFrame.get<std::string>();
                if (frameName == "null")
                {
                    layer->getFrames().emplace_back();
                    layer->getSourceFrames().emplace_back();
                }
                else
                {
                    auto jFrames = jSheet["frames"];
                    auto jf = jFrames[frameName.c_str()];
                    layer->getFrames().push_back(_toRect(jf["frame"]));
                    layer->getSourceFrames().push_back(_toRect(jf["spriteSourceSize"]));
                }
            }
            anim->getLayers().push_back(layer);
        }
        std::cout << "found animation: " << name << std::endl;

        _animations.push_back(std::unique_ptr<GGCostumeAnimation>(anim));
    }
}

void GGCostume::updateAnimation()
{
    std::string name(_animation);
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
    setAnimation(name);
}

void GGCostume::setAnimation(const std::string &name)
{
    for (auto &animation : _animations)
    {
        if (animation->getName() == name)
        {
            _pCurrentAnimation = animation.get();
            return;
        }
    }
}

void GGCostume::update(const sf::Time &elapsed)
{
    if (!_pCurrentAnimation)
        return;
    _pCurrentAnimation->update(elapsed);
}



void GGCostume::draw(sf::RenderWindow &window, const sf::RenderStates &states) const
{
    if (!_pCurrentAnimation)
        return;
    _pCurrentAnimation->draw(window, states);
}
} // namespace gg
