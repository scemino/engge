#include <iostream>
#include "Entities/Actor/BlinkState.hpp"
#include "Entities/Actor/Costume.hpp"
#include "Engine/EngineSettings.hpp"
#include "Parsers/JsonTokenReader.hpp"
#include "System/Locator.hpp"
#include "../../System/_Util.hpp"

namespace ng
{
Costume::Costume(TextureManager &textureManager)
    : _textureManager(textureManager),
      _blinkState(*this)
{
    resetLockFacing();
}

Costume::~Costume() = default;

void Costume::setLayerVisible(const std::string &name, bool isVisible)
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
    if(_pCurrentAnimation->getLayers().empty())
        return;
    auto it = std::find_if(_pCurrentAnimation->getLayers().begin(), _pCurrentAnimation->getLayers().end(), [name](CostumeLayer *pLayer) {
        return pLayer->getName() == name;
    });
    if (it != _pCurrentAnimation->getLayers().end())
    {
        (*it)->setVisible(isVisible);
    }
}

Facing Costume::getFacing() const
{
    if(_lockFacing)
    {
        return _facings.at(_facing);
    }
    return _facing;
}

void Costume::setFacing(Facing facing)
{
    if (_facing == facing)
        return;
    _facing = facing;
    updateAnimation();
}

void Costume::lockFacing(Facing left, Facing right, Facing front, Facing back)
{
    _facings[Facing::FACE_LEFT] = left;
    _facings[Facing::FACE_RIGHT] = right;
    _facings[Facing::FACE_FRONT] = front;
    _facings[Facing::FACE_BACK] = back;
    _lockFacing = true;
}

void Costume::resetLockFacing()
{
    _facings[Facing::FACE_LEFT] = Facing::FACE_LEFT;
    _facings[Facing::FACE_RIGHT] = Facing::FACE_RIGHT;
    _facings[Facing::FACE_FRONT] = Facing::FACE_FRONT;
    _facings[Facing::FACE_BACK] = Facing::FACE_BACK;
}

void Costume::unlockFacing()
{
    _lockFacing = false;
}

void Costume::setState(const std::string &name)
{
    _animation = name;
    updateAnimation();
}

CostumeLayer* Costume::loadLayer(const GGPackValue& jLayer) const
{
    auto layer = new CostumeLayer();
    layer->setTexture(&_costumeSheet.getTexture());
    auto fps = jLayer["fps"].isNull() ? 10 : jLayer["fps"].int_value;
    layer->setFps(fps);
    auto layerName = jLayer["name"].string_value;
    layer->setName(layerName);
    if (!jLayer["flags"].isNull())
    {
        layer->setFlags(jLayer["flags"].int_value);
    }
    for (const auto &jFrame : jLayer["frames"].array_value)
    {
        auto frameName = jFrame.string_value;
        if (frameName == "null")
        {
            layer->getFrames().emplace_back();
            layer->getSourceFrames().emplace_back();
            layer->getSizes().emplace_back();
        }
        else
        {
            layer->getFrames().push_back(_costumeSheet.getRect(frameName));
            layer->getSourceFrames().push_back(_costumeSheet.getSpriteSourceSize(frameName));
            layer->getSizes().push_back(_costumeSheet.getSourceSize(frameName));
        }
    }
    if (!jLayer["triggers"].isNull())
    {
        for (const auto &jTrigger : jLayer["triggers"].array_value)
        {
            if (!jTrigger.isNull())
            {
                auto triggerName = jTrigger.string_value;
                char *end;
                auto trigger = std::strtol(triggerName.data() + 1, &end, 10);
                if (end == triggerName.data() + 1)
                {
                    layer->getSoundTriggers().emplace_back(triggerName.data() + 1);
                    layer->getTriggers().emplace_back(std::nullopt);
                }
                else
                {
                    layer->getTriggers().emplace_back(trigger);
                    layer->getSoundTriggers().emplace_back(std::nullopt);
                }
            }
            else
            {
                layer->getSoundTriggers().emplace_back(std::nullopt);
                layer->getTriggers().emplace_back(std::nullopt);
            }
        }
    }
    for (const auto &jOffset : jLayer["offsets"].array_value)
    {
        layer->getOffsets().emplace_back((sf::Vector2i)_parsePos(jOffset.string_value));
    }
    if(!jLayer["loop"].isNull() && jLayer["loop"].int_value == 1)
    {
        layer->setLoop(true);
    }
    layer->setActor(_pActor);
    return layer;
}

void Costume::loadCostume(const std::string &path, const std::string &sheet)
{
    _path = path;
    _sheet = sheet;

    Locator<EngineSettings>::get().readEntry(_path, _hash);
    if (_sheet.empty())
    {
        _sheet = _hash["sheet"].string_value;
    }

    _costumeSheet.setTextureManager(&_textureManager);
    _costumeSheet.load(_sheet);

    // load animations
    _animations.clear();
    _hiddenLayers.clear();
    _hiddenLayers.emplace("blink");
    _hiddenLayers.emplace("eyes_left");
    _hiddenLayers.emplace("eyes_right");
    for (auto j : _hash["animations"].array_value)
    {
        auto name = j["name"].string_value;
        auto pAnimation = std::make_unique<CostumeAnimation>(name);
        if(j["layers"].isNull())
        {
            auto layer = loadLayer(j);
            pAnimation->getLayers().push_back(layer);
        } 
        else 
        {
            for (auto jLayer : j["layers"].array_value)
            {
                auto layer = loadLayer(jLayer);
                pAnimation->getLayers().push_back(layer);
            }
        }
        _animations.push_back(std::move(pAnimation));
    }

    // don't know if it's necessary, reyes has no costume in the intro
    setAnimation("stand_front");
}

bool Costume::setAnimation(const std::string &animName)
{
    if (_pCurrentAnimation && _pCurrentAnimation->getName() == animName)
        return true;

    for(auto&& pAnim : _animations)
    {
        if(pAnim->getName() == animName)
        {
            _pCurrentAnimation = pAnim.get();
            for(auto&& layer : _pCurrentAnimation->getLayers())
            {
                auto layerName = layer->getName();
                layer->setVisible(_hiddenLayers.find(layerName) == _hiddenLayers.end());
            }
            _pCurrentAnimation->play();
            return true;
        }
    }

    _pCurrentAnimation = nullptr;
    return false;
}

static bool _startsWith(const std::string &str, const std::string &prefix)
{
    return str.length() >= prefix.length() && 0 == str.compare(0, prefix.length(), prefix);
}

void Costume::updateAnimation()
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

    if (!setAnimation(_animation))
    {
        std::string name(_animation);
        name.append("_");
        switch (getFacing())
        {
        case Facing::FACE_BACK:
            name.append("back");
            break;
        case Facing::FACE_FRONT:
            name.append("front");
            break;
        case Facing::FACE_LEFT:
            name.append("right");
            break;
        case Facing::FACE_RIGHT:
            name.append("right");
            break;
        }
        setAnimation(name);
    }

    if (_pCurrentAnimation)
    {
        auto &layers = _pCurrentAnimation->getLayers();
        for (auto layer : layers)
        {
            layer->setLeftDirection(getFacing() == Facing::FACE_LEFT);
        }
    }
}

void Costume::update(const sf::Time &elapsed)
{
    if (!_pCurrentAnimation)
        return;
    _pCurrentAnimation->update(elapsed);
    _blinkState.update(elapsed);
}

void Costume::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    if (!_pCurrentAnimation)
        return;
    target.draw(*_pCurrentAnimation, states);
}

void Costume::setHeadIndex(int index)
{
    _headIndex = index;
    if(!_pCurrentAnimation) return;
    for (int i = 0; i < 6; i++)
    {
        std::ostringstream s;
        s << _headAnimName << (i + 1);
        auto layerName = s.str();
        auto it = std::find_if(_pCurrentAnimation->getLayers().begin(), _pCurrentAnimation->getLayers().end(), [layerName](CostumeLayer *pLayer) {
            return pLayer->getName() == layerName;
        });
        if (it != _pCurrentAnimation->getLayers().end())
        {
            (*it)->setVisible(_headIndex == i);
        }
    }
}

void Costume::setAnimationNames(const std::string &headAnim, const std::string &standAnim, const std::string &walkAnim, const std::string &reachAnim)
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

void Costume::setBlinkRate(double min, double max)
{
    _blinkState.setRate(min, max);
}
} // namespace ng
