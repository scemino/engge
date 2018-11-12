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
      _animation("stand"),
      _headAnimName("head"),
      _standAnimName("stand"),
      _walkAnimName("walk"),
      _reachAnimName("reach"),
      _headIndex(0)
{
    _hiddenLayers.emplace("blink");
    _hiddenLayers.emplace("eyes_left");
    _hiddenLayers.emplace("eyes_right");
}

GGCostume::~GGCostume() = default;

void GGCostume::setLayerVisible(const std::string &name, bool isVisible)
{
    if (!isVisible)
    {
        _hiddenLayers.emplace(name);
    }
    else
    {
        _hiddenLayers.erase(name);
    }
    if (_pCurrentAnimation == nullptr)
        return;
    auto it = std::find_if(_pCurrentAnimation->getLayers().begin(), _pCurrentAnimation->getLayers().end(), [name](GGLayer *pLayer) {
        return pLayer->getName() == name;
    });
    if (it != _pCurrentAnimation->getLayers().end())
    {
        (*it)->setVisible(isVisible);
    }
}

void GGCostume::setFacing(Facing facing)
{
    _facing = facing;
    updateAnimation();
}

void GGCostume::lockFacing(Facing facing)
{
    // TODO: lock
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
        std::cout << "Anim: " << name << std::endl;
        if (animName != name)
            continue;

        _pCurrentAnimation = std::make_unique<GGCostumeAnimation>(name, _texture);
        for (auto jLayer : j["layers"])
        {
            auto layer = new GGLayer();
            auto fps = jLayer["fps"].is_null() ? 10 : jLayer["fps"].get<int>();
            layer->setFps(fps);
            auto layerName = jLayer["name"].get<std::string>();
            layer->setVisible(_hiddenLayers.find(layerName) == _hiddenLayers.end());
            layer->setName(layerName);
            if (!jLayer["flags"].is_null())
            {
                layer->setFlags(jLayer["flags"].get<int>());
            }
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
            for (const auto &jOffset : jLayer["offsets"])
            {
                layer->getOffsets().emplace_back((sf::Vector2i)_parsePos(jOffset.get<std::string>()));
            }
            _pCurrentAnimation->getLayers().push_back(layer);
        }
        std::cout << "found animation: " << name << std::endl;
    }
}

static bool _startsWith(const std::string &str, const std::string &prefix)
{
    return str.length() >= prefix.length() && 0 == str.compare(0, prefix.length(), prefix);
}

void GGCostume::updateAnimation()
{
    // special case for eyes... bof
    if (_pCurrentAnimation && _startsWith(_animation, "eyes_"))
    {
        auto &layers = _pCurrentAnimation->getLayers();
        for (auto layer : layers)
        {
            if (!_startsWith(layer->getName(), "eyes_"))
                continue;
            setLayerVisible(layer->getName(), false);
        }
        setLayerVisible(_animation, true);
        return;
    }

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

void GGCostume::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    if (!_pCurrentAnimation)
        return;
    target.draw(*_pCurrentAnimation, states);
}

void GGCostume::setHeadIndex(int index)
{
    _headIndex = index;
    for (int i = 0; i < 6; i++)
    {
        std::ostringstream s;
        s << _headAnimName << (i + 1);
        // std::cout << "setLayerVisible(" << s.str() << "," << (_headIndex == i) << ")" << std::endl;
        auto layerName = s.str();
        auto it = std::find_if(_pCurrentAnimation->getLayers().begin(), _pCurrentAnimation->getLayers().end(), [layerName](GGLayer *pLayer) {
            return pLayer->getName() == layerName;
        });
        if (it != _pCurrentAnimation->getLayers().end())
        {
            (*it)->setVisible(_headIndex == i);
        }
    }
}

void GGCostume::setAnimationNames(const std::string &headAnim, const std::string &standAnim, const std::string &walkAnim, const std::string &reachAnim)
{
    if (!headAnim.empty())
    {
        _headAnimName = headAnim;
    }
    if (!standAnim.empty())
    {
        _standAnimName = standAnim;
    }
    if (!walkAnim.empty())
    {
        _walkAnimName = walkAnim;
    }
    if (!reachAnim.empty())
    {
        _reachAnimName = reachAnim;
    }
}
} // namespace gg
