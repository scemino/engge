#include <fstream>
#include <memory>
#include <algorithm>
#include <iostream>
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
        RoomLayer layer;
        for (auto &bg : jWimpy["background"])
        {
            auto frame = json["frames"][bg.get<std::string>()]["frame"];
            auto sprite = sf::Sprite();
            sprite.move(xOff, 0);
            sprite.setTexture(_textureManager.get(_sheet));
            sprite.setTextureRect(_toRect(frame));
            // _backgrounds.push_back(sprite);
            xOff += sprite.getTextureRect().width;
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

    // layers
    for (auto jLayer : jWimpy["layers"])
    {
        RoomLayer layer;
        auto zsort = jLayer["zsort"].get<int>();
        layer.setZOrder(zsort);
        if (jLayer["name"].is_array())
        {
            float offsetX = 0;
            for (auto jName : jLayer["name"])
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

    auto &texture = _textureManager.get(_sheet);

    // objects
    for (auto jObject : jWimpy["objects"])
    {
        auto object = std::make_unique<GGObject>();
        // name
        auto name = jObject["name"].get<std::string>();
        object->setName(name);
        // zsort
        object->setZOrder(jObject["zsort"].get<int>());
        std::cout << "Read object " << object->getName() << ", zsort: " << object->getZOrder() << std::endl;
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
            object->setPosition(sf::Vector2f(pos.x, _roomSize.y - pos.y));
            object->setUsePosition((sf::Vector2f)usePos);
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
                    anim->getRects().push_back(_toRect(json["frames"][n]["frame"]));
                    anim->getSourceRects().push_back(_toRect(json["frames"][n]["spriteSourceSize"]));
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

GGTextObject &GGRoom::createTextObject(const std::string &name, GGFont &font)
{
    auto object = std::make_unique<GGTextObject>(font);
    auto &obj = *object.get();
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
        obj.get()->draw(window, cameraPos);
    }
}

void GGRoom::drawLayer(const RoomLayer &layer, sf::RenderWindow &window, const sf::Vector2f &cameraPos) const
{
    for (const auto &s : layer.getSprites())
    {
        auto sprite(s);
        auto parallax = layer.getParallax();
        auto pos = (160.f - cameraPos.x) * parallax.x - 160.f;
        sprite.move(pos, -cameraPos.y);
        window.draw(sprite);
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
    std::for_each(_objects.begin(), _objects.end(), [elapsed](std::unique_ptr<GGObject> &obj) { obj.get()->update(elapsed); });

    _entities.clear();
    for(auto& layer : _layers){
        _entities.push_back(&layer);
    }
    for(auto& object : _objects){
        _entities.push_back(object.get());
    }
    auto cmpEntities = [](GGEntity* a, GGEntity* b) {
        return a->getZOrder() > b->getZOrder();
    };
    std::sort(_entities.begin(), _entities.end(), cmpEntities);
}

void GGRoom::draw(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const
{
    sf::RenderStates states;
    states.transform.translate(-cameraPos);

    for(const auto& entity : _entities)
    {
        entity->draw(window, cameraPos);
    }

    // drawBackgroundLayers(window, cameraPos);
    // drawObjects(window, cameraPos);
    // drawForegroundLayers(window, cameraPos);
    // drawWalkboxes(window, states);
}
} // namespace gg
