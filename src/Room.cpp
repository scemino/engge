#include <fstream>
#include <memory>
#include <algorithm>
#include <iostream>
#include <nlohmann/json.hpp>
#include "Room.h"
#include "_NGUtil.h"
#include "Screen.h"

namespace ng
{
Room::Room(TextureManager &textureManager, const EngineSettings &settings)
    : _textureManager(textureManager),
      _settings(settings),
      _showDrawWalkboxes(false)
{
}

void Room::setAsParallaxLayer(Entity *pEntity, int layerNum)
{
    auto itEndLayers = std::end(_layers);
    auto it = std::find_if(std::begin(_layers), itEndLayers, [layerNum](const std::unique_ptr<RoomLayer> &layer) {
        return layer->getZOrder() == layerNum;
    });
    if (it == itEndLayers)
        return;
    auto itMainLayer = std::find_if(std::begin(_layers), std::end(_layers), [](const std::unique_ptr<RoomLayer> &pLayer) {
        return pLayer->getZOrder() == 0;
    });
    itMainLayer->get()->removeEntity(*pEntity);
    it->get()->addEntity(*pEntity);
}

void Room::loadBackgrounds(nlohmann::json jWimpy, nlohmann::json json)
{
    int width = 0;
    _fullscreen = jWimpy["fullscreen"].get<int>();
    if (jWimpy["background"].is_array())
    {
        auto layer = std::make_unique<RoomLayer>();
        for (auto &bg : jWimpy["background"])
        {
            auto frame = json["frames"][bg.get<std::string>()]["frame"];
            auto sprite = sf::Sprite();
            sprite.move(width, 0);
            sprite.setTexture(_textureManager.get(_sheet));
            sprite.setTextureRect(_toRect(frame));
            width += sprite.getTextureRect().width;
            layer->getSprites().push_back(sprite);
        }
        _layers.push_back(std::move(layer));
    }
    else if (jWimpy["background"].is_string())
    {
        auto frame = json["frames"][jWimpy["background"].get<std::string>()]["frame"];
        auto sprite = sf::Sprite();
        sprite.setTexture(_textureManager.get(_sheet));
        sprite.setTextureRect(_toRect(frame));
        auto layer = std::make_unique<RoomLayer>();
        layer->getSprites().push_back(sprite);
        _layers.push_back(std::move(layer));
    }
    // room width seems to be not enough :S
    if (width > _roomSize.x)
    {
        _roomSize.x = width;
    }
}

void Room::loadLayers(nlohmann::json jWimpy, nlohmann::json json)
{
    if (jWimpy["layers"].is_null())
        return;

    for (auto jLayer : jWimpy["layers"])
    {
        auto layer = std::make_unique<RoomLayer>();
        auto zsort = jLayer["zsort"].get<int>();
        layer->setZOrder(zsort);
        if (jLayer["name"].is_array())
        {
            float offsetX = 0;
            for (const auto &jName : jLayer["name"])
            {
                auto layerName = jName.get<std::string>();
                // layer.getNames().push_back(layerName);

                const auto &rect = _toRect(json["frames"][layerName]["frame"]);
                sf::Sprite s;
                s.setTexture(_textureManager.get(_sheet));
                s.setTextureRect(rect);
                const auto &sourceRect = _toRect(json["frames"][layerName]["spriteSourceSize"]);
                s.setOrigin(sf::Vector2f(-sourceRect.left, -sourceRect.top));
                s.move(sf::Vector2f(offsetX, 0));
                offsetX += rect.width;
                layer->getSprites().push_back(s);
            }
        }
        else
        {
            auto layerName = jLayer["name"].get<std::string>();

            const auto &rect = _toRect(json["frames"][layerName]["frame"]);
            sf::Sprite s;
            s.setTexture(_textureManager.get(_sheet));
            s.setTextureRect(rect);
            const auto &sourceRect = _toRect(json["frames"][layerName]["spriteSourceSize"]);
            s.setOrigin(sf::Vector2f(-sourceRect.left, -sourceRect.top));
            layer->getSprites().push_back(s);
        }
        if (jLayer["parallax"].is_string())
        {
            auto parallax = _parsePos(jLayer["parallax"].get<std::string>());
            layer->setParallax(parallax);
        }
        else
        {
            auto parallax = jLayer["parallax"].get<float>();
            layer->setParallax(sf::Vector2f(parallax, 1));
        }
        std::cout << "Read layer zsort: " << layer->getZOrder() << std::endl;
        _layers.push_back(std::move(layer));
    }

    // push default layer
    auto layer = std::make_unique<RoomLayer>();
    _layers.push_back(std::move(layer));
}

void Room::loadScalings(nlohmann::json jWimpy)
{
    if (jWimpy["scaling"].is_array())
    {
        if (jWimpy["scaling"][0].is_string())
        {
            RoomScaling scaling;
            for (auto jScaling : jWimpy["scaling"])
            {
                auto value = jScaling.get<std::string>();
                auto index = value.find('@');
                auto scale = std::strtof(value.substr(0, index - 1).c_str(), nullptr);
                auto yPos = std::strtof(value.substr(index + 1).c_str(), nullptr);
                Scaling s{};
                s.scale = scale;
                s.yPos = yPos;
                scaling.getScalings().push_back(s);
            }
            _scalings.push_back(scaling);
        }
        else if (jWimpy["scaling"][0].is_array())
        {
            for (auto jScaling : jWimpy["scaling"])
            {
                RoomScaling scaling;
                for (auto jSubScaling : jScaling["scaling"])
                {
                    if (jSubScaling["trigger"].is_string())
                    {
                        scaling.setTrigger(jSubScaling["trigger"].get<std::string>());
                    }
                    auto value = jSubScaling.get<std::string>();
                    auto index = value.find('@');
                    auto scale = std::strtof(value.substr(0, index - 1).c_str(), nullptr);
                    auto yPos = std::strtof(value.substr(index + 1).c_str(), nullptr);
                    Scaling s{};
                    s.scale = scale;
                    s.yPos = yPos;
                    scaling.getScalings().push_back(s);
                }
                _scalings.push_back(scaling);
            }
        }
    }
}

void Room::loadWalkboxes(nlohmann::json jWimpy)
{
    for (auto jWalkbox : jWimpy["walkboxes"])
    {
        std::vector<sf::Vector2i> vertices;
        auto polygon = jWalkbox["polygon"].get<std::string>();
        _parsePolygon(polygon, vertices, _roomSize.y);
        auto walkbox = std::make_unique<Walkbox>(vertices);
        if (jWalkbox["name"].is_string())
        {
            auto walkboxName = jWalkbox["name"].get<std::string>();
            walkbox->setName(walkboxName);
        }
        _walkboxes.push_back(std::move(walkbox));
    }
}

void Room::loadObjects(nlohmann::json jWimpy, nlohmann::json json)
{
    auto itLayer = std::find_if(std::begin(_layers), std::end(_layers), [](const std::unique_ptr<RoomLayer> &pLayer) {
        return pLayer->getZOrder() == 0;
    });
    auto &texture = _textureManager.get(_sheet);

    for (auto jObject : jWimpy["objects"])
    {
        auto object = std::make_unique<Object>();
        // name
        auto objectName = jObject["name"].get<std::string>();
        object->setName(objectName);
        // zsort
        object->setZOrder(jObject["zsort"].get<int>());
        // prop
        bool isProp = jObject["prop"].is_number_integer() && jObject["prop"].get<int>() == 1;
        object->setProp(isProp);
        // position
        auto pos = _parsePos(jObject["pos"].get<std::string>());
        auto usePos = _parsePos(jObject["usepos"].get<std::string>());
        auto useDir = _toDirection(jObject["usedir"].get<std::string>());
        object->setUseDirection(useDir);
        // hotspot
        auto hotspot = _parseRect(jObject["hotspot"].get<std::string>());
        object->setHotspot(hotspot);
        // spot
        bool isSpot = jObject["spot"].is_number_integer() && jObject["spot"].get<int>() == 1;
        object->setSpot(isSpot);
        // spot
        bool isTrigger = jObject["trigger"].is_number_integer() && jObject["trigger"].get<int>() == 1;
        object->setTrigger(isTrigger);

        object->setPosition(sf::Vector2f(pos.x, _roomSize.y - pos.y));
        object->setUsePosition(usePos);

        // animations
        if (!jObject["animations"].empty())
        {
            for (auto jAnimation : jObject["animations"])
            {
                auto animName = jAnimation["name"].get<std::string>();
                auto anim = std::make_unique<Animation>(texture, animName);
                if (!jAnimation["fps"].is_null())
                {
                    anim->setFps(jAnimation["fps"].get<int>());
                }
                for (const auto &jFrame : jAnimation["frames"])
                {
                    auto n = jFrame.get<std::string>();
                    if (json["frames"][n].is_null())
                        continue;
                    anim->getRects().push_back(_toRect(json["frames"][n]["frame"]));
                    anim->getSizes().push_back(_toSize(json["frames"][n]["sourceSize"]));
                    anim->getSourceRects().push_back(_toRect(json["frames"][n]["spriteSourceSize"]));
                }
                if (!jAnimation["triggers"].is_null())
                {
                    for (const auto &jtrigger : jAnimation["triggers"])
                    {
                        if (!jtrigger.is_null())
                        {
                            auto name = jtrigger.get<std::string>();
                            auto trigger = std::atoi(name.data() + 1);
                            anim->getTriggers().push_back(trigger);
                        }
                        else
                        {
                            anim->getTriggers().push_back(std::nullopt);
                        }
                    }
                }
                anim->reset();
                object->getAnims().push_back(std::move(anim));
            }

            for (auto &animation : object->getAnims())
            {
                if (!animation->getRects().empty())
                {
                    object->setAnimation(animation->getName());
                    break;
                }
            }
        }
        object->setRoom(this);
        std::cout << "Object " << *object << std::endl;
        itLayer->get()->addEntity(*object);
        _objects.push_back(std::move(object));
    }

    // sort objects
    auto cmpObjects = [](std::unique_ptr<Object> &a, std::unique_ptr<Object> &b) {
        return a->getZOrder() > b->getZOrder();
    };
    std::sort(_objects.begin(), _objects.end(), cmpObjects);
}

void Room::load(const char *name)
{
    _id = name;

    // load wimpy file
    std::string wimpyFilename;
    wimpyFilename.append(_settings.getGamePath()).append(name).append(".wimpy");
    std::cout << "Load room " << wimpyFilename << std::endl;

    nlohmann::json jWimpy;
    {
        std::ifstream i(wimpyFilename);
        i >> jWimpy;
    }

    _sheet = jWimpy["sheet"].get<std::string>();
    _roomSize = (sf::Vector2i)_parsePos(jWimpy["roomsize"].get<std::string>());

    // load json file
    std::string jsonFilename;
    jsonFilename.append(_settings.getGamePath()).append(_sheet).append(".json");
    nlohmann::json json;
    {
        std::ifstream i(jsonFilename);
        i >> json;
    }

    loadBackgrounds(jWimpy, json);
    loadLayers(jWimpy, json);
    loadObjects(jWimpy, json);
    loadScalings(jWimpy);
    loadWalkboxes(jWimpy);
}

TextObject &Room::createTextObject(const std::string &fontName)
{
    auto object = std::make_unique<TextObject>();
    std::string path;
    path.append(_settings.getGamePath()).append(fontName).append(".fnt");
    object->getFont().loadFromFile(path);
    auto &obj = *object;
    _objects.push_back(std::move(object));
    return obj;
}

void Room::deleteObject(Object &object)
{
    auto const &it = std::find_if(_objects.begin(), _objects.end(), [&](std::unique_ptr<Object> &ptr) {
        return ptr.get() == &object;
    });
    _objects.erase(it);
}

Object &Room::createObject(const std::vector<std::string> &anims)
{
    return createObject(_sheet, anims);
}

Object &Room::createObject(const std::string &sheet, const std::vector<std::string> &anims)
{
    auto &texture = _textureManager.get(sheet);

    // load json file
    std::string jsonFilename;
    jsonFilename.append(_settings.getGamePath()).append(sheet).append(".json");
    nlohmann::json json;
    {
        std::ifstream i(jsonFilename);
        i >> json;
    }

    auto object = std::make_unique<Object>();
    auto animation = std::make_unique<Animation>(texture, "state0");
    for (const auto &n : anims)
    {
        if (json["frames"][n].is_null())
            continue;
        auto frame = json["frames"][n]["frame"];
        auto r = _toRect(frame);
        animation->getRects().push_back(r);
        auto spriteSourceSize = json["frames"][n]["spriteSourceSize"];
        animation->getSizes().push_back(_toSize(json["frames"][n]["sourceSize"]));
        animation->getSourceRects().push_back(_toRect(spriteSourceSize));
    }
    animation->reset();
    object->getAnims().push_back(std::move(animation));

    for (auto &anim : object->getAnims())
    {
        if (!anim->getRects().empty())
        {
            object->setAnimation(anim->getName());
            break;
        }
    }
    auto &obj = *object;
    auto itLayer = std::find_if(std::begin(_layers), std::end(_layers), [](const std::unique_ptr<RoomLayer> &pLayer) {
        return pLayer->getZOrder() == 0;
    });
    itLayer->get()->addEntity(obj);
    _objects.push_back(std::move(object));
    return obj;
}

void Room::drawWalkboxes(sf::RenderWindow &window, sf::RenderStates states) const
{
    if (!_showDrawWalkboxes)
        return;

    for (auto &walkbox : _walkboxes)
    {
        walkbox->draw(window, states);
    }
}

void Room::update(const sf::Time &elapsed)
{
    std::for_each(std::begin(_layers), std::end(_layers),
                  [elapsed](std::unique_ptr<RoomLayer> &layer) { layer->update(elapsed); });

    std::sort(std::begin(_layers), std::end(_layers), [](std::unique_ptr<RoomLayer> &a, std::unique_ptr<RoomLayer> &b) {
        return a->getZOrder() > b->getZOrder();
    });
}

void Room::draw(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const
{
    sf::RenderStates states;
    auto ratio = ((float)Screen::Height) / _roomSize.y;
    for (const auto &layer : _layers)
    {
        auto parallax = layer->getParallax();
        auto posX = (Screen::HalfWidth - cameraPos.x) * parallax.x - Screen::HalfWidth;
        auto posY = (Screen::HalfHeight - cameraPos.y) * parallax.y - Screen::HalfHeight;

        sf::Transform t;
        t.translate(posX, posY);
        if (_fullscreen == 1)
        {
            t.scale(ratio, ratio);
        }
        states.transform = t;
        layer->draw(window, states);
    }

    sf::Transform t;
    t.translate(-cameraPos);
    states.transform = t;
    drawWalkboxes(window, states);
}

const RoomScaling &Room::getRoomScaling() const
{
    return _scalings[0];
}

} // namespace ng
