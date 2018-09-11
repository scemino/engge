#include <fstream>
#include <memory>
#include <algorithm>
#include "GGRoom.h"
#include "_GGUtil.h"
#include <nlohmann/json.hpp>

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

GGRoom::~GGRoom()
{
}

void GGRoom::load(const char *name)
{
    // load wimpy file
    std::string wimpyFilename;
    std::string path(_settings.getGamePath());
    wimpyFilename.append(path).append(name).append(".wimpy");
    printf("Load room %s\n", wimpyFilename.c_str());
    nlohmann::json jWimpy;
    {
        std::ifstream i(wimpyFilename);
        i >> jWimpy;
    }

    _sheet = jWimpy["sheet"].get<std::string>();
    _roomSize = _parsePos(jWimpy["roomsize"].get<std::string>());

    // load json file
    std::string jsonFilename;
    path = _settings.getGamePath();
    jsonFilename.append(path).append(_sheet).append(".json");
    nlohmann::json json;
    {
        std::ifstream i(jsonFilename);
        i >> json;
    }

    // backgrounds
    if (jWimpy["background"].is_array())
    {
        int xOff = 0;
        for (auto &bg : jWimpy["background"])
        {
            auto frame = json["frames"][bg.get<std::string>()]["frame"];
            auto sprite = sf::Sprite();
            sprite.move(xOff, 0);
            sprite.setTexture(_textureManager.get(_sheet));
            sprite.setTextureRect(_toRect(frame));
            _backgrounds.push_back(sprite);
            xOff += sprite.getTextureRect().width;
        }
    }
    else if (jWimpy["background"].is_string())
    {
        auto frame = json["frames"][jWimpy["background"].get<std::string>()]["frame"];
        auto sprite = sf::Sprite();
        sprite.setTexture(_textureManager.get(_sheet));
        sprite.setTextureRect(_toRect(frame));
        _backgrounds.push_back(sprite);
    }

    // layers
    for (auto jLayer : jWimpy["layers"])
    {
        RoomLayer layer;
        auto zsort = jLayer["zsort"].get<int>();
        layer.setZsort(zsort);
        if (jLayer["name"].is_array())
        {
            float offsetX = 0;
            for (auto jName : jLayer["name"])
            {
                auto layerName = jName.get<std::string>();
                layer.getNames().push_back(layerName);

                const auto &rect = _toRect(json["frames"][layerName]["frame"]);
                sf::Sprite s;
                s.setTexture(_textureManager.get(_sheet));
                s.setTextureRect(rect);
                if (layer.getZsort() < 0)
                {
                    s.setPosition(sf::Vector2f(offsetX, _roomSize.y - rect.height));
                    offsetX += rect.width;
                }
                layer.getSprites().push_back(s);
            }
        }
        else
        {
            auto layerName = jLayer["name"].get<std::string>();
            layer.getNames().push_back(layerName);

            const auto &rect = _toRect(json["frames"][layerName]["frame"]);
            sf::Sprite s;
            s.setTexture(_textureManager.get(_sheet));
            s.setTextureRect(rect);
            if (layer.getZsort() < 0)
            {
                s.setPosition(sf::Vector2f(0, _roomSize.y - rect.height));
            }
            layer.getSprites().push_back(s);
        }
        if (jLayer["parallax"].is_string())
        {
            auto parallax = _parsePos(jLayer["parallax"].get<std::string>());
            layer.setParallax(static_cast<sf::Vector2f>(parallax));
        }
        else
        {
            auto parallax = jLayer["parallax"].get<float>();
            layer.setParallax(sf::Vector2f(parallax, 0));
        }
        _layers.push_back(layer);
    }
    // sort layers
    auto cmpLayers = [](RoomLayer &a, RoomLayer &b) {
        return a.getZsort() > b.getZsort();
    };
    std::sort(_layers.begin(), _layers.end(), cmpLayers);

    auto &texture = _textureManager.get(_sheet);

    // objects
    for (auto jObject : jWimpy["objects"])
    {
        auto object = std::make_unique<GGObject>();
        // name
        auto name = jObject["name"].get<std::string>();
        object->setName(name);
        printf("Read object %s\n", object->getName().c_str());
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

        // if (!jObject["prop"].empty() && jObject["prop"].get<int>() == 1)
        {
            object->setPosition(pos.x, _roomSize.y - pos.y);
            object->setUsePosition(usePos.x, usePos.y);
        }

        sf::IntRect rect;
        // animations
        if (!jObject["animations"].empty())
        {
            for (auto jAnim : jObject["animations"])
            {
                auto animName = jAnim["name"].get<std::string>();
                auto anim = std::make_unique<GGAnim>(texture, animName);
                if (!jAnim["fps"].is_null())
                {
                    anim->setFps(jAnim["fps"].get<int>());
                }
                for (auto jFrame : jAnim["frames"])
                {
                    auto n = jFrame.get<std::string>();
                    if (json["frames"][n].is_null())
                        continue;
                    auto frame = json["frames"][n]["frame"];
                    sf::IntRect r = _toRect(frame);
                    anim->getRects().push_back(r);
                    auto spriteSourceSize = json["frames"][n]["spriteSourceSize"];
                    anim->getSourceRects().push_back(_toRect(spriteSourceSize));
                }
                object->getAnims().push_back(std::move(anim));
            }

            for (auto &anim : object->getAnims())
            {
                if (anim.get()->getRects().size() > 0)
                {
                    object.get()->setAnim(anim.get()->getName());
                    break;
                }
            }
        }
        _objects.push_back(std::move(object));
    }

    // sort objects
    auto cmpObjs = [](std::unique_ptr<GGObject> &a, std::unique_ptr<GGObject> &b) {
        return a.get()->getZOrder() > b.get()->getZOrder();
    };
    std::sort(_objects.begin(), _objects.end(), cmpObjs);

    // read scalings
    if (jWimpy["scalings"].is_array() && jWimpy["scalings"].size() > 0)
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
                auto scale = atof(value.substr(0, index - 1).c_str());
                auto yPos = atof(value.substr(index + 1).c_str());
                Scaling s;
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
                    auto scale = atof(value.substr(0, index - 1).c_str());
                    auto yPos = atof(value.substr(index + 1).c_str());
                    Scaling s;
                    s.scale = scale;
                    s.yPos = yPos;
                    scaling.getScalings().push_back(s);
                }
                _scalings.push_back(scaling);
            }
        }
    }

    // read walkboxes
    for (auto jWalkbox : jWimpy["walkboxes"])
    {
        std::vector<sf::Vector2i> vertices;
        if (jWalkbox["name"].is_string())
        {
            auto walkboxName = jWalkbox["name"].get<std::string>();
        }
        auto polygon = jWalkbox["polygon"].get<std::string>();
        _parsePolygon(polygon, vertices, _roomSize.y);
        _walkboxes.push_back(Walkbox(vertices));
    }
}

GGTextObject &GGRoom::createTextObject(const std::string &name, GGFont& font)
{
    auto object = std::make_unique<GGTextObject>(font);
    auto &obj = *object.get();
    _objects.push_back(std::move(object));
    return obj;
}

GGObject &GGRoom::createObject(const std::vector<std::string> &anims)
{
    auto &texture = _textureManager.get(_sheet);

    // load json file
    std::string jsonFilename;
    jsonFilename.append(_settings.getGamePath()).append(_sheet).append(".json");
    nlohmann::json json;
    {
        std::ifstream i(jsonFilename);
        i >> json;
    }

    auto object = std::make_unique<GGObject>();
    auto anim = std::make_unique<GGAnim>(texture, "");
    for (auto n : anims)
    {
        if (json["frames"][n].is_null())
            continue;
        auto frame = json["frames"][n]["frame"];
        auto r = _toRect(frame);
        anim->getRects().push_back(r);
        auto spriteSourceSize = json["frames"][n]["spriteSourceSize"];
        anim->getSourceRects().push_back(_toRect(spriteSourceSize));
    }
    object->getAnims().push_back(std::move(anim));

    for (auto &anim : object->getAnims())
    {
        if (anim.get()->getRects().size() > 0)
        {
            object.get()->setAnim(anim.get()->getName());
            break;
        }
    }
    auto &obj = *object.get();
    _objects.push_back(std::move(object));
    return obj;
}

void GGRoom::drawWalkboxes(sf::RenderWindow &window) const
{
    if (!_showDrawWalkboxes)
        return;

    for (auto &walkbox : _walkboxes)
    {
        walkbox.draw(window);
    }
}

void GGRoom::drawObjects(sf::RenderWindow &window) const
{
    if (!_showObjects)
        return;

    for (auto &obj : _objects)
    {
        obj.get()->draw(window);
    }
}

void GGRoom::drawBackgrounds(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const
{
    for (auto &bg : _backgrounds)
    {
        sf::Sprite b(bg);
        b.move(-cameraPos);
        window.draw(b);
    }
}

void GGRoom::drawBackgroundLayers(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const
{
    if (!_showLayers)
        return;

    for (auto &layer : _layers)
    {
        if (layer.getZsort() < 0)
            continue;
        for (auto &s : layer.getSprites())
        {
            auto sprite(s);
            auto parallax = layer.getParallax();
            auto pos = (_roomSize.x / 2.0f - cameraPos.x) * parallax.x - _roomSize.x / 2.0f;
            auto rect = sprite.getTextureRect();
            sprite.setOrigin(-rect.width / 2, 0);
            sprite.move(pos, -cameraPos.y);
            window.draw(sprite);
        }
    }
}

void GGRoom::drawForegroundLayers(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const
{
    if (!_showLayers)
        return;

    for (auto &layer : _layers)
    {
        if (layer.getZsort() >= 0)
            continue;
        for (auto &s : layer.getSprites())
        {
            auto sprite(s);
            auto parallax = layer.getParallax();
            auto pos = -((cameraPos.x - (_roomSize.x / 2.0f)) * parallax.x) - (_roomSize.x / 2.0f);
            sprite.setPosition(pos, -cameraPos.y);
            window.draw(sprite);
        }
    }
}

void GGRoom::update(const sf::Time &elapsed)
{
    std::for_each(_objects.begin(), _objects.end(), [elapsed](std::unique_ptr<GGObject> &obj) { obj.get()->update(elapsed); });
}

void GGRoom::draw(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const
{
    // drawBackgroundLayers(window, cameraPos);
    drawBackgrounds(window, cameraPos);
    drawObjects(window);
    // drawForegroundLayers(window, cameraX);
    drawWalkboxes(window);
}
} // namespace gg
