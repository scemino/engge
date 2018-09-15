#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include "GGCostume.h"
#include "_GGUtil.h"

namespace gg
{
CostumeAnim::CostumeAnim(const std::string &name)
    : _name(name)
{
}

CostumeAnim::~CostumeAnim()
{
}

GGCostume::GGCostume(const GGEngineSettings &settings)
    : _settings(settings),
      _pCurrentAnim(nullptr),
      _facing(Facing::FACE_FRONT),
      _anim("stand")
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
        auto name = j["name"].get<std::string>();
        auto anim = new CostumeAnim(name);
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
            anim->getLayers().push_back(layer);
        }
        std::cout << "found anim: " << name << std::endl;

        _animations.push_back(std::unique_ptr<CostumeAnim>(anim));
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
    for (auto &anim : _animations)
    {
        if (anim.get()->getName() == name)
        {
            _pCurrentAnim = anim.get();
            return;
        }
    }
}

void GGCostume::update(const sf::Time &elapsed)
{
    if (!_pCurrentAnim)
        return;
    for (int i = 0; i < _pCurrentAnim->getLayers().size(); i++)
    {
        _pCurrentAnim->getLayers()[i]->update(elapsed);
    }

    _sprites.clear();
    for (int i = 0; i < _pCurrentAnim->getLayers().size(); i++)
    {
        auto frame = _pCurrentAnim->getLayers()[i]->getIndex();
        auto &rect = _pCurrentAnim->getLayers()[i]->getFrames()[frame];
        auto &sourceRect = _pCurrentAnim->getLayers()[i]->getSourceFrames()[frame];
        sf::Sprite sprite(_texture, rect);
        sprite.setOrigin(-sourceRect.left, -sourceRect.top);
        _sprites.push_back(sprite);
    }
}

void GGCostume::draw(sf::RenderWindow &window, const sf::RenderStates states) const
{
    for (auto sprite : _sprites)
    {
        window.draw(sprite, states);
    }
}
} // namespace gg
