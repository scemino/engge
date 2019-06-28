#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include "Costume.h"
#include "_NGUtil.h"

namespace ng
{
BlinkState::BlinkState(Costume &costume) : _costume(costume)
{
}

void BlinkState::setRate(double min, double max)
{
    _min = min;
    _max = max;
    if (min == 0 && max == 0)
    {
        // blinking is disabled
        _state = -1;
    }
    else
    {
        _state = 0;
        _value = sf::seconds(float_rand(_min, _max));
    }
    _elapsed = sf::seconds(0);
    _costume.setLayerVisible("blink", false);
}

void BlinkState::update(sf::Time elapsed)
{
    if (_state == 0)
    {
        // wait to blink
        _elapsed += elapsed;
        if (_elapsed > _value)
        {
            _state = 1;
            _costume.setLayerVisible("blink", true);
            _elapsed = sf::seconds(0);
        }
    }
    else if (_state == 1)
    {
        // wait time the eyes are closed
        _elapsed += elapsed;
        if (_elapsed > sf::seconds(0.2))
        {
            _costume.setLayerVisible("blink", false);
            _value = sf::seconds(float_rand(_min, _max));
            _elapsed = sf::seconds(0);
            _state = 0;
        }
    }
}

Costume::Costume(TextureManager &textureManager)
    : _settings(textureManager.getSettings()),
      _textureManager(textureManager),
      _pCurrentAnimation(nullptr),
      _facing(Facing::FACE_FRONT),
      _animation("stand"),
      _headAnimName("head"),
      _standAnimName("stand"),
      _walkAnimName("walk"),
      _reachAnimName("reach"),
      _headIndex(0),
      _pActor(nullptr),
      _blinkState(*this)
{
    _hiddenLayers.emplace("blink");
    _hiddenLayers.emplace("eyes_left");
    _hiddenLayers.emplace("eyes_right");
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
    auto it = std::find_if(_pCurrentAnimation->getLayers().begin(), _pCurrentAnimation->getLayers().end(), [name](CostumeLayer *pLayer) {
        return pLayer->getName() == name;
    });
    if (it != _pCurrentAnimation->getLayers().end())
    {
        (*it)->setVisible(isVisible);
    }
}

void Costume::setFacing(Facing facing)
{
    if(_lockFacing)
    {
        facing = _facings[facing];
    }
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

void Costume::loadCostume(const std::string &path, const std::string &sheet)
{
    _path = path;
    _sheet = sheet;

    // don't know if it's necessary, reyes has no costume in the intro
    setAnimation("stand_front");
}

bool Costume::setAnimation(const std::string &animName)
{
    if (_pCurrentAnimation && _pCurrentAnimation->getName() == animName)
        return true;

    GGPackValue hash;
    _settings.readEntry(_path, hash);
    if (_sheet.empty())
    {
        _sheet = hash["sheet"].string_value;
    }

    std::string sheetPath;
    sheetPath.append(_sheet).append(".json");
    std::vector<char> buffer;
    _settings.readEntry(sheetPath, buffer);
    auto jSheet = nlohmann::json::parse(buffer.data());

    // load texture
    _texture = _textureManager.get(_sheet);

    // find animation matching name
    for (auto j : hash["animations"].array_value)
    {
        auto name = j["name"].string_value;
        // std::cout << "Anim: " << name << std::endl;
        if (animName != name)
            continue;

        _pCurrentAnimation = std::make_unique<CostumeAnimation>(name);
        for (auto jLayer : j["layers"].array_value)
        {
            auto layer = new CostumeLayer();
            layer->setTexture(&_texture);
            auto fps = jLayer["fps"].isNull() ? 10 : jLayer["fps"].int_value;
            layer->setFps(fps);
            auto layerName = jLayer["name"].string_value;
            layer->setVisible(_hiddenLayers.find(layerName) == _hiddenLayers.end());
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
                    auto jFrames = jSheet["frames"];
                    auto jf = jFrames[frameName.c_str()];
                    layer->getFrames().push_back(_toRect(jf["frame"]));
                    layer->getSourceFrames().push_back(_toRect(jf["spriteSourceSize"]));
                    layer->getSizes().push_back(_toSize(jf["sourceSize"]));
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
            layer->setActor(_pActor);
            _pCurrentAnimation->getLayers().push_back(layer);
        }
        std::cout << "found animation: " << name << std::endl;
        return true;
    }
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
        switch (_facing)
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
            layer->setLeftDirection(_facing == Facing::FACE_LEFT);
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
    for (int i = 0; i < 6; i++)
    {
        std::ostringstream s;
        s << _headAnimName << (i + 1);
        // std::cout << "setLayerVisible(" << s.str() << "," << (_headIndex == i) << ")" << std::endl;
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
