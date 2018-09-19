#include <fstream>
#include <memory>
#include <algorithm>
#include <iostream>
#include <nlohmann/json.hpp>
#include "GGRoom.h"
#include "_GGUtil.h"
#include "Screen.h"

namespace gg
{
GGRoom::GGRoom(TextureManager &textureManager, const GGEngineSettings &settings)
    : _textureManager(textureManager),
      _settings(settings),
      _showDrawWalkboxes(false),
      _showObjects(true),
      _showLayers(true)
{
}

GGRoom::~GGRoom() = default;

void GGRoom::loadBackgrounds(nlohmann::json jWimpy, nlohmann::json json)
{
    int width = 0;
    if (jWimpy["background"].is_array())
    {
        RoomLayer layer;
        for (auto &bg : jWimpy["background"])
        {
            auto frame = json["frames"][bg.get<std::string>()]["frame"];
            auto sprite = sf::Sprite();
            sprite.move(width, 0);
            sprite.setTexture(_textureManager.get(_sheet));
            sprite.setTextureRect(_toRect(frame));
            // _backgrounds.push_back(sprite);
            width += sprite.getTextureRect().width;
            layer.getSprites().push_back(sprite);
        }
        _layers.push_back(layer);
    }
    else if (jWimpy["background"].is_string())
    {
        auto frame = json["frames"][jWimpy["background"].get<std::string>()]["frame"];
        auto sprite = sf::Sprite();
        sprite.setTexture(_textureManager.get(_sheet));
        sprite.setTextureRect(_toRect(frame));
        // _backgrounds.push_back(sprite);
        RoomLayer layer;
        layer.getSprites().push_back(sprite);
        _layers.push_back(layer);
    }
    // room width seems to be not enough :S
    if (width > _roomSize.x)
    {
        _roomSize.x = width;
    }
}

void GGRoom::loadLayers(nlohmann::json jWimpy, nlohmann::json json)
{
    if (jWimpy["layers"].is_null())
        return;

    for (auto jLayer : jWimpy["layers"])
    {
        RoomLayer layer;
        auto zsort = jLayer["zsort"].get<int>();
        layer.setZOrder(zsort);
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
                offsetX += rect.width;
                layer.getSprites().push_back(s);
            }
        }
        else
        {
            auto layerName = jLayer["name"].get<std::string>();
            // layer.getNames().push_back(layerName);

            const auto &rect = _toRect(json["frames"][layerName]["frame"]);
            sf::Sprite s;
            s.setTexture(_textureManager.get(_sheet));
            s.setTextureRect(rect);
            const auto &sourceRect = _toRect(json["frames"][layerName]["spriteSourceSize"]);
            s.setOrigin(sf::Vector2f(-sourceRect.left, -sourceRect.top));
            layer.getSprites().push_back(s);
        }
        if (jLayer["parallax"].is_string())
        {
            auto parallax = _parsePos(jLayer["parallax"].get<std::string>());
            layer.setParallax(sf::Vector2f(parallax));
        }
        else
        {
            auto parallax = jLayer["parallax"].get<float>();
            layer.setParallax(sf::Vector2f(parallax, 0));
        }
        std::cout << "Read layer zsort: " << layer.getZOrder() << std::endl;
        _layers.push_back(layer);
    }
    // sort layers
    auto cmpLayers = [](RoomLayer &a, RoomLayer &b) {
        return a.getZOrder() > b.getZOrder();
    };
    std::sort(_layers.begin(), _layers.end(), cmpLayers);
}

void GGRoom::loadScalings(nlohmann::json jWimpy, nlohmann::json json)
{
    if (jWimpy["scalings"].is_array() && !jWimpy["scalings"].empty())
    {
        if (jWimpy["scalings"][0].is_string())
        {
            RoomScaling scaling;
            for (auto jScaling : jWimpy["scalings"])
            {
                if (!jScaling["trigger"].is_string())
                {
                    scaling.setTrigger(jScaling["trigger"].get<std::string>());
                }
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
        else if (jWimpy["scalings"][0].is_array())
        {
            for (auto jScaling : jWimpy["scalings"])
            {
                RoomScaling scaling;
                for (auto jSubScaling : jScaling["scalings"])
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

void GGRoom::loadWalkboxes(nlohmann::json jWimpy, nlohmann::json json)
{
    for (auto jWalkbox : jWimpy["walkboxes"])
    {
        std::vector<sf::Vector2i> vertices;
        if (jWalkbox["name"].is_string())
        {
            auto walkboxName = jWalkbox["name"].get<std::string>();
        }
        auto polygon = jWalkbox["polygon"].get<std::string>();
        _parsePolygon(polygon, vertices, _roomSize.y);
        _walkboxes.emplace_back(vertices);
    }
}

void GGRoom::loadObjects(nlohmann::json jWimpy, nlohmann::json json)
{
    auto &texture = _textureManager.get(_sheet);

    for (auto jObject : jWimpy["objects"])
    {
        auto object = std::make_unique<GGObject>();
        // name
        auto objectName = jObject["name"].get<std::string>();
        object->setName(objectName);
        // zsort
        object->setZOrder(jObject["zsort"].get<int>());
        // prop
        object->setProp(jObject["prop"].is_number_integer() && jObject["prop"].get<int>() == 1);
        // position
        auto pos = _parsePos(jObject["pos"].get<std::string>());
        auto usePos = _parsePos(jObject["usepos"].get<std::string>());
        auto useDir = _toDirection(jObject["usedir"].get<std::string>());
        object->setUseDirection(useDir);
        // hotspot
        auto hotspot = _parseRect(jObject["hotspot"].get<std::string>());
        object->setHotspot(hotspot);

        // object->setVisible(jObject["prop"].empty() || jObject["prop"].get<int>() == 0);
        // if (!jObject["prop"].empty() && jObject["prop"].get<int>() == 1)
        {
            object->setPosition(sf::Vector2f(pos.x, _roomSize.y - pos.y));
            object->setUsePosition((sf::Vector2f)usePos);
        }

        // animations
        if (!jObject["animations"].empty())
        {
            for (auto jAnimation : jObject["animations"])
            {
                auto animName = jAnimation["name"].get<std::string>();
                auto anim = std::make_unique<GGAnimation>(texture, animName);
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
                    anim->getSourceRects().push_back(_toRect(json["frames"][n]["spriteSourceSize"]));
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
        std::cout << "Object " << *object << std::endl;
        _objects.push_back(std::move(object));
    }

    // sort objects
    auto cmpObjects = [](std::unique_ptr<GGObject> &a, std::unique_ptr<GGObject> &b) {
        return a->getZOrder() > b->getZOrder();
    };
    std::sort(_objects.begin(), _objects.end(), cmpObjects);
}

void GGRoom::load(const char *name)
{
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
    _roomSize = _parsePos(jWimpy["roomsize"].get<std::string>());

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
    loadScalings(jWimpy, json);
    loadWalkboxes(jWimpy, json);
}

GGTextObject &GGRoom::createTextObject(const std::string &name, GGFont &font)
{
    auto object = std::make_unique<GGTextObject>(font);
    auto &obj = *object;
    _objects.push_back(std::move(object));
    return obj;
}

void GGRoom::deleteObject(GGObject &object)
{
    auto const &it = std::find_if(_objects.begin(), _objects.end(), [&](std::unique_ptr<GGObject> &ptr) {
        return ptr.get() == &object;
    });
    _objects.erase(it);
}

GGObject &GGRoom::createObject(const std::vector<std::string> &anims)
{
    return createObject(_sheet, anims);
}

GGObject &GGRoom::createObject(const std::string &sheet, const std::vector<std::string> &anims)
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

    auto object = std::make_unique<GGObject>();
    auto animation = std::make_unique<GGAnimation>(texture, "state0");
    for (const auto &n : anims)
    {
        if (json["frames"][n].is_null())
            continue;
        auto frame = json["frames"][n]["frame"];
        auto r = _toRect(frame);
        animation->getRects().push_back(r);
        auto spriteSourceSize = json["frames"][n]["spriteSourceSize"];
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
    _objects.push_back(std::move(object));
    return obj;
}

void GGRoom::drawWalkboxes(sf::RenderWindow &window, sf::RenderStates states) const
{
    if (!_showDrawWalkboxes)
        return;

    for (auto &walkbox : _walkboxes)
    {
        walkbox.draw(window, states);
    }
}

void GGRoom::drawObjects(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const
{
    if (!_showObjects)
        return;

    for (auto &obj : _objects)
    {
        obj->draw(window, cameraPos);
    }
}

void GGRoom::drawBackgroundLayers(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const
{
    if (!_showLayers)
        return;

    for (const auto &layer : _layers)
    {
        if (layer.getZOrder() < 0)
            continue;
        layer.draw(window, cameraPos);
    }
}

void GGRoom::drawForegroundLayers(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const
{
    if (!_showLayers)
        return;

    for (const auto &layer : _layers)
    {
        if (layer.getZOrder() >= 0)
            continue;
        layer.draw(window, cameraPos);
    }
}

void GGRoom::update(const sf::Time &elapsed)
{
    std::for_each(_objects.begin(), _objects.end(),
                  [elapsed](std::unique_ptr<GGObject> &obj) { obj->update(elapsed); });

    std::sort(_objects.begin(), _objects.end(), [](std::unique_ptr<GGObject> &a, std::unique_ptr<GGObject> &b) {
        return a->getZOrder() > b->getZOrder();
    });
}

void GGRoom::draw(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const
{
    sf::RenderStates states;
    states.transform.translate(-cameraPos);

    drawBackgroundLayers(window, cameraPos);
    drawObjects(window, cameraPos);
    drawForegroundLayers(window, cameraPos);

    drawWalkboxes(window, states);
}
} // namespace gg
