#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include "GGCostume.h"
#include "_GGUtil.h"

namespace gg
{
GGCostume::GGCostume(TextureManager &textureManager)
    : _settings(textureManager.getSettings()),
      _textureManager(textureManager),
      _pCurrentAnimation(nullptr),
      _facing(Facing::FACE_FRONT),
      _animation("stand")
{
}

GGCostume::~GGCostume() = default;

void GGCostume::setLayerVisible(const std::string &name, bool isVisible)
{
    if (!isVisible)
    {
        _hiddenLayers.emplace(name);
        return;
    }
    _hiddenLayers.erase(name);
}

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

void GGCostume::loadCostume(const std::string &path, const std::string &sheet)
{
    _path = path;
    _sheet = sheet;
}

void GGCostume::setAnimation(const std::string &animName)
{
    nlohmann::json json;
    nlohmann::json jSheet;
    {
        std::ifstream i(_path);
        i >> json;
    }
    if (_sheet.empty())
    {
        _sheet = json["sheet"].get<std::string>();
    }

    std::string sheetPath(_settings.getGamePath());
    sheetPath.append(_sheet).append(".json");
    {
        std::ifstream i(sheetPath);
        i >> jSheet;
    }

    // load texture
    _texture = _textureManager.get(_sheet);

    // find animation matching name
    for (auto j : json["animations"])
    {
        auto name = j["name"].get<std::string>();
        if (animName != name)
            continue;

        _pCurrentAnimation = std::make_unique<GGCostumeAnimation>(name, _texture);
        for (auto jLayer : j["layers"])
        {
            auto layer = new GGLayer();
            auto fps = jLayer["fps"].is_null() ? 10 : jLayer["fps"].get<int>();
            layer->setFps(fps);
            auto layerName = jLayer["name"].get<std::string>();
            layer->setVisible(_hiddenLayers.find(layerName)==_hiddenLayers.end());
            layer->setName(layerName);
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
            _pCurrentAnimation->getLayers().push_back(layer);
        }
        std::cout << "found animation: " << name << std::endl;
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
