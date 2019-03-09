#include <fstream>
#include <math.h>
#include <memory>
#include <algorithm>
#include <iostream>
#include <list>
#include <nlohmann/json.hpp>
#include "Animation.h"
#include "Room.h"
#include "Screen.h"
#include "_NGUtil.h"

namespace ng
{
int Room::RoomType = 1;

Room::Room(TextureManager &textureManager, EngineSettings &settings)
    : _textureManager(textureManager),
      _ambientColor(255, 255, 255, 255),
      _settings(settings),
      _showDrawWalkboxes(false),
      _spriteSheet(textureManager, settings)
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

void Room::removeEntity(Entity *pEntity)
{
    for (auto &layer : _layers)
    {
        layer->removeEntity(*pEntity);
    }
}

void Room::loadBackgrounds(GGPackValue &jWimpy)
{
    int width = 0;
    if (!jWimpy["fullscreen"].isNull())
    {
        _fullscreen = jWimpy["fullscreen"].int_value;
    }
    if (jWimpy["background"].isArray())
    {
        auto layer = std::make_unique<RoomLayer>();
        for (auto &bg : jWimpy["background"].array_value)
        {
            auto frame = _spriteSheet.getRect(bg.string_value);
            auto sprite = sf::Sprite();
            sprite.move(width, 0);
            sprite.setTexture(_textureManager.get(_sheet));
            sprite.setTextureRect(frame);
            width += sprite.getTextureRect().width;
            layer->getSprites().push_back(sprite);
        }
        _layers.push_back(std::move(layer));
    }
    else if (jWimpy["background"].isString())
    {
        auto frame = _spriteSheet.getRect(jWimpy["background"].string_value);
        auto sprite = sf::Sprite();
        sprite.setTexture(_textureManager.get(_sheet));
        sprite.setTextureRect(frame);
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

void Room::loadLayers(GGPackValue &jWimpy)
{
    if (jWimpy["layers"].isNull())
        return;

    for (auto jLayer : jWimpy["layers"].array_value)
    {
        auto layer = std::make_unique<RoomLayer>();
        auto zsort = jLayer["zsort"].int_value;
        layer->setZOrder(zsort);
        if (jLayer["name"].isArray())
        {
            float offsetX = 0;
            for (const auto &jName : jLayer["name"].array_value)
            {
                auto layerName = jName.string_value;
                // layer.getNames().push_back(layerName);

                const auto &rect = _spriteSheet.getRect(layerName);
                sf::Sprite s;
                s.setTexture(_textureManager.get(_sheet));
                s.setTextureRect(rect);
                const auto &sourceRect = _spriteSheet.getSpriteSourceSize(layerName);
                s.setOrigin(sf::Vector2f(-sourceRect.left, -sourceRect.top));
                s.move(sf::Vector2f(offsetX, 0));
                offsetX += rect.width;
                layer->getSprites().push_back(s);
            }
        }
        else
        {
            auto layerName = jLayer["name"].string_value;

            const auto &rect = _spriteSheet.getRect(layerName);
            sf::Sprite s;
            s.setTexture(_textureManager.get(_sheet));
            s.setTextureRect(rect);
            const auto &sourceRect = _spriteSheet.getSpriteSourceSize(layerName);
            s.setOrigin(sf::Vector2f(-sourceRect.left, -sourceRect.top));
            layer->getSprites().push_back(s);
        }
        if (jLayer["parallax"].isString())
        {
            auto parallax = _parsePos(jLayer["parallax"].string_value);
            layer->setParallax(parallax);
        }
        else
        {
            auto parallax = jLayer["parallax"].double_value;
            layer->setParallax(sf::Vector2f(parallax, 1));
        }
        std::cout << "Read layer zsort: " << layer->getZOrder() << std::endl;
        _layers.push_back(std::move(layer));
    }

    // push default layer
    auto layer = std::make_unique<RoomLayer>();
    _layers.push_back(std::move(layer));
}

void Room::loadScalings(GGPackValue &jWimpy)
{
    if (jWimpy["scaling"].isArray())
    {
        if (jWimpy["scaling"][0].isString())
        {
            RoomScaling scaling;
            for (auto jScaling : jWimpy["scaling"].array_value)
            {
                auto value = jScaling.string_value;
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
        else if (jWimpy["scaling"][0].isArray())
        {
            for (auto jScaling : jWimpy["scaling"].array_value)
            {
                RoomScaling scaling;
                for (auto jSubScaling : jScaling["scaling"].array_value)
                {
                    if (jSubScaling["trigger"].isString())
                    {
                        scaling.setTrigger(jSubScaling["trigger"].string_value);
                    }
                    auto value = jSubScaling.string_value;
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

void Room::loadWalkboxes(GGPackValue &jWimpy)
{
    for (auto jWalkbox : jWimpy["walkboxes"].array_value)
    {
        std::vector<sf::Vector2i> vertices;
        auto polygon = jWalkbox["polygon"].string_value;
        _parsePolygon(polygon, vertices, _roomSize.y);
        Walkbox walkbox(vertices);
        if (jWalkbox["name"].isString())
        {
            auto walkboxName = jWalkbox["name"].string_value;
            walkbox.setName(walkboxName);
        }
        _walkboxes.push_back(walkbox);
    }
    updateGraph();
}

void Room::updateGraph()
{
    _graphWalkboxes.clear();
    if (!_walkboxes.empty())
    {
        merge(_walkboxes, _graphWalkboxes);
    }
    _pf = std::make_shared<PathFinder>(_graphWalkboxes);
}

void Room::loadObjects(GGPackValue &jWimpy)
{
    auto itLayer = std::find_if(std::begin(_layers), std::end(_layers), [](const std::unique_ptr<RoomLayer> &pLayer) {
        return pLayer->getZOrder() == 0;
    });
    auto &texture = _textureManager.get(_sheet);

    for (auto jObject : jWimpy["objects"].array_value)
    {
        auto object = std::make_unique<Object>();
        // name
        auto objectName = jObject["name"].string_value;
        object->setName(objectName);
        // zsort
        object->setZOrder(jObject["zsort"].int_value);
        // prop
        bool isProp = jObject["prop"].isInteger() && jObject["prop"].int_value == 1;
        object->setProp(isProp);
        // position
        auto pos = _parsePos(jObject["pos"].string_value);
        auto usePos = _parsePos(jObject["usepos"].string_value);
        auto useDir = _toDirection(jObject["usedir"].string_value);
        object->setUseDirection(useDir);
        // hotspot
        auto hotspot = _parseRect(jObject["hotspot"].string_value);
        object->setHotspot(hotspot);
        // spot
        bool isSpot = jObject["spot"].isInteger() && jObject["spot"].int_value == 1;
        object->setSpot(isSpot);
        // spot
        bool isTrigger = jObject["trigger"].isInteger() && jObject["trigger"].int_value == 1;
        object->setTrigger(isTrigger);

        object->setDefaultPosition(sf::Vector2f(pos.x, _roomSize.y - pos.y));
        object->setUsePosition(usePos);

        // animations
        if (jObject["animations"].isArray())
        {
            for (auto jAnimation : jObject["animations"].array_value)
            {
                auto animName = jAnimation["name"].string_value;
                auto anim = std::make_unique<Animation>(texture, animName);
                if (!jAnimation["fps"].isNull())
                {
                    anim->setFps(jAnimation["fps"].int_value);
                }
                for (const auto &jFrame : jAnimation["frames"].array_value)
                {
                    auto n = jFrame.string_value;
                    if (!_spriteSheet.hasRect(n))
                        continue;
                    anim->getRects().push_back(_spriteSheet.getRect(n));
                    anim->getSizes().push_back(_spriteSheet.getSourceSize(n));
                    anim->getSourceRects().push_back(_spriteSheet.getSpriteSourceSize(n));
                }
                if (!jAnimation["triggers"].isNull())
                {
                    for (const auto &jtrigger : jAnimation["triggers"].array_value)
                    {
                        if (!jtrigger.isNull())
                        {
                            auto name = jtrigger.string_value;
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

            object->setAnimation("state0");
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
    wimpyFilename.append(name).append(".wimpy");
    std::cout << "Load room " << wimpyFilename << std::endl;

    if (!_settings.hasEntry(wimpyFilename))
        return;

    GGPackValue hash;
    _settings.readEntry(wimpyFilename, hash);

    _sheet = hash["sheet"].string_value;
    _roomSize = (sf::Vector2i)_parsePos(hash["roomsize"].string_value);

    // load json file
    std::string jsonFilename;
    _spriteSheet.load(_sheet);

    loadBackgrounds(hash);
    loadLayers(hash);
    loadObjects(hash);
    loadScalings(hash);
    loadWalkboxes(hash);
}

TextObject &Room::createTextObject(const std::string &fontName)
{
    auto object = std::make_unique<TextObject>();
    std::string path;
    path.append(fontName).append("Font.fnt");
    object->getFont().setSettings(&_settings);
    object->getFont().loadFromFile(path);
    auto &obj = *object;
    obj.setVisible(true);
    _objects.push_back(std::move(object));
    auto itLayer = std::find_if(std::begin(_layers), std::end(_layers), [](const std::unique_ptr<RoomLayer> &pLayer) {
        return pLayer->getZOrder() == 0;
    });
    itLayer->get()->addEntity(obj);
    return obj;
}

void Room::deleteObject(Object &object)
{
    auto const &it = std::find_if(_objects.begin(), _objects.end(), [&](std::unique_ptr<Object> &ptr) {
        return ptr.get() == &object;
    });
    auto itLayer = std::find_if(std::begin(_layers), std::end(_layers), [](const std::unique_ptr<RoomLayer> &pLayer) {
        return pLayer->getZOrder() == 0;
    });
    itLayer->get()->removeEntity(object);
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
    jsonFilename.append(sheet).append(".json");
    std::vector<char> buffer;
    _settings.readEntry(jsonFilename, buffer);
    auto json = nlohmann::json::parse(buffer.data());

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

Object &Room::createObject(const std::string &image)
{
    auto &texture = _textureManager.get(image);

    auto object = std::make_unique<Object>();
    auto animation = std::make_unique<Animation>(texture, "state0");
    auto size = texture.getSize();
    sf::IntRect rect(0, 0, size.x, size.y);
    animation->getRects().push_back(rect);
    animation->getSizes().push_back(sf::Vector2i(size));
    animation->getSourceRects().push_back(rect);
    animation->reset();
    object->getAnims().push_back(std::move(animation));

    object->setAnimation("state0");
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

    for (auto &walkbox : _graphWalkboxes)
    {
        window.draw(walkbox, states);
    }

    if (_path)
    {
        window.draw(*_path);
    }

    if (_pf && _pf->getGraph())
    {
        window.draw(*_pf->getGraph(), states);
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
        layer->drawForeground(window, states);
    }
}

const RoomScaling &Room::getRoomScaling() const
{
    return _scalings[0];
}

void Room::setWalkboxEnabled(const std::string &name, bool isEnabled)
{
    auto it = std::find_if(_walkboxes.begin(), _walkboxes.end(), [&name](const Walkbox &walkbox) {
        return walkbox.getName() == name;
    });
    if (it == _walkboxes.end())
    {
        std::cerr << "walkbox " << name << " has not been found" << std::endl;
        return;
    }
    it->setEnabled(isEnabled);
    updateGraph();
}

bool Room::inWalkbox(const sf::Vector2f &pos) const
{
    auto inWalkbox = std::any_of(_walkboxes.begin(), _walkboxes.end(), [pos](const Walkbox &w) {
        return w.inside((sf::Vector2i)pos);
    });
    return inWalkbox;
}

std::vector<sf::Vector2i> Room::calculatePath(const sf::Vector2i &start, const sf::Vector2i &end) const
{
    return _pf->calculatePath(start, end);
}

} // namespace ng
