#include <fstream>
#include <math.h>
#include <memory>
#include <algorithm>
#include <iostream>
#include <list>
#include "squirrel.h"
#include "nlohmann/json.hpp"
#include "Animation.h"
#include "PathFinder.h"
#include "Room.h"
#include "RoomLayer.h"
#include "RoomScaling.h"
#include "Screen.h"
#include "SpriteSheet.h"
#include "TextObject.h"
#include "_NGUtil.h"

namespace ng
{
struct Room::Impl
{
    TextureManager &_textureManager;
    EngineSettings &_settings;
    std::vector<std::unique_ptr<Object>> _objects;
    std::vector<Walkbox> _walkboxes;
    std::vector<std::unique_ptr<RoomLayer>> _layers;
    std::vector<RoomScaling> _scalings;
    sf::Vector2i _roomSize;
    bool _showDrawWalkboxes{false};
    std::string _sheet;
    std::string _id;
    int _fullscreen{0};
    HSQOBJECT _table;
    std::shared_ptr<Path> _path;
    std::shared_ptr<PathFinder> _pf;
    std::vector<Walkbox> _graphWalkboxes;
    sf::Color _ambientColor{255, 255, 255, 255};
    SpriteSheet _spriteSheet;
    Room *_pRoom{nullptr};

    Impl(TextureManager &textureManager, EngineSettings &settings)
        : _textureManager(textureManager),
          _settings(settings)
    {
        _spriteSheet.setTextureManager(&textureManager);
        _spriteSheet.setSettings(&settings);
    }

    void setRoom(Room *pRoom)
    {
        _pRoom = pRoom;
    }

    void loadBackgrounds(GGPackValue &jWimpy)
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

    void loadLayers(GGPackValue &jWimpy)
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

    void loadObjects(GGPackValue &jWimpy)
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
            object->setId(towstring(objectName));
            object->setName(towstring(objectName));
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
                                auto trigger = std::strtol(name.data() + 1, nullptr, 10);
                                anim->getTriggers().emplace_back(trigger);
                            }
                            else
                            {
                                anim->getTriggers().emplace_back(std::nullopt);
                            }
                        }
                    }
                    anim->reset();
                    object->getAnims().push_back(std::move(anim));
                }

                object->setStateAnimIndex(0);
            }
            object->setRoom(_pRoom);
            // std::cout << "Object " << *object << std::endl;
            itLayer->get()->addEntity(*object);
            _objects.push_back(std::move(object));
        }

        // sort objects
        auto cmpObjects = [](std::unique_ptr<Object> &a, std::unique_ptr<Object> &b) {
            return a->getZOrder() > b->getZOrder();
        };
        std::sort(_objects.begin(), _objects.end(), cmpObjects);
    }

    void loadScalings(GGPackValue &jWimpy)
    {
        if (jWimpy["scaling"].isArray())
        {
            if (jWimpy["scaling"][0].isString())
            {
                RoomScaling scaling;
                for (const auto jScaling : jWimpy["scaling"].array_value)
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

        if (_scalings.size() == 0)
        {
            RoomScaling scaling;
            _scalings.push_back(scaling);
        }
    }

    void loadWalkboxes(GGPackValue &jWimpy)
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

    void updateGraph()
    {
        _graphWalkboxes.clear();
        if (!_walkboxes.empty())
        {
            merge(_walkboxes, _graphWalkboxes);
        }
        _pf = std::make_shared<PathFinder>(_graphWalkboxes);
    }
};

int Room::RoomType = 1;

Room::Room(TextureManager &textureManager, EngineSettings &settings)
    : pImpl(std::make_unique<Impl>(textureManager, settings))
{
    pImpl->setRoom(this);
}

Room::~Room() = default;

void Room::setId(const std::string &id) { pImpl->_id = id; }

const std::string &Room::getId() const { return pImpl->_id; }

std::vector<std::unique_ptr<Object>> &Room::getObjects() { return pImpl->_objects; }

const std::string &Room::getSheet() const { return pImpl->_sheet; }

void Room::showDrawWalkboxes(bool show) { pImpl->_showDrawWalkboxes = show; }

bool Room::areDrawWalkboxesVisible() const { return pImpl->_showDrawWalkboxes; }

sf::Vector2i Room::getRoomSize() const { return pImpl->_roomSize; }

HSQOBJECT& Room::getTable() { return pImpl->_table; }

bool Room::walkboxesVisible() const { return pImpl->_showDrawWalkboxes; }

void Room::setAmbientLight(sf::Color color) { pImpl->_ambientColor = color; }

sf::Color Room::getAmbientLight() const { return pImpl->_ambientColor; }

void Room::setAsParallaxLayer(Entity *pEntity, int layerNum)
{
    auto itEndLayers = std::end(pImpl->_layers);
    auto it = std::find_if(std::begin(pImpl->_layers), itEndLayers, [layerNum](const std::unique_ptr<RoomLayer> &layer) {
        return layer->getZOrder() == layerNum;
    });
    if (it == itEndLayers)
        return;
    auto itMainLayer = std::find_if(std::begin(pImpl->_layers), std::end(pImpl->_layers), [](const std::unique_ptr<RoomLayer> &pLayer) {
        return pLayer->getZOrder() == 0;
    });
    itMainLayer->get()->removeEntity(*pEntity);
    it->get()->addEntity(*pEntity);
}

void Room::roomLayer(int layerNum, bool enabled)
{
    auto itEndLayers = std::end(pImpl->_layers);
    auto it = std::find_if(std::begin(pImpl->_layers), itEndLayers, [layerNum](const std::unique_ptr<RoomLayer> &layer) {
        return layer->getZOrder() == layerNum;
    });
    if (it == itEndLayers)
        return;
    it->get()->setEnabled(enabled);
}

void Room::removeEntity(Entity *pEntity)
{
    for (auto &layer : pImpl->_layers)
    {
        layer->removeEntity(*pEntity);
    }
}

void Room::load(const char *name)
{
    pImpl->_id = name;

    // load wimpy file
    std::string wimpyFilename;
    wimpyFilename.append(name).append(".wimpy");
    std::cout << "Load room " << wimpyFilename << std::endl;

    if (!pImpl->_settings.hasEntry(wimpyFilename))
        return;

    GGPackValue hash;
    pImpl->_settings.readEntry(wimpyFilename, hash);

#if 0
    std::ofstream out;
    out.open(wimpyFilename, std::ios::out);
    out << hash;
    out.close();
#endif

    pImpl->_sheet = hash["sheet"].string_value;
    pImpl->_roomSize = (sf::Vector2i)_parsePos(hash["roomsize"].string_value);

    // load json file
    pImpl->_spriteSheet.load(pImpl->_sheet);

    pImpl->loadBackgrounds(hash);
    pImpl->loadLayers(hash);
    pImpl->loadObjects(hash);
    pImpl->loadScalings(hash);
    pImpl->loadWalkboxes(hash);
}

TextObject &Room::createTextObject(const std::string &fontName)
{
    auto object = std::make_unique<TextObject>();
    std::string path;
    path.append(fontName).append(".fnt");
    object->getFont().setSettings(&pImpl->_settings);
    object->getFont().loadFromFile(path);
    auto &obj = *object;
    obj.setVisible(true);
    pImpl->_objects.push_back(std::move(object));
    auto itLayer = std::find_if(std::begin(pImpl->_layers), std::end(pImpl->_layers), [](const std::unique_ptr<RoomLayer> &pLayer) {
        return pLayer->getZOrder() == 0;
    });
    itLayer->get()->addEntity(obj);
    return obj;
}

void Room::deleteObject(Object &object)
{
    auto itLayer = std::find_if(std::begin(pImpl->_layers), std::end(pImpl->_layers), [](const std::unique_ptr<RoomLayer> &pLayer) {
        return pLayer->getZOrder() == 0;
    });
    itLayer->get()->removeEntity(object);
    //pImpl->_objects.erase(it);
}

Object &Room::createObject(const std::vector<std::string> &anims)
{
    return createObject(pImpl->_sheet, anims);
}

Object &Room::createObject(const std::string &sheet, const std::vector<std::string> &anims)
{
    auto &texture = pImpl->_textureManager.get(sheet);

    // load json file
    std::string jsonFilename;
    jsonFilename.append(sheet).append(".json");
    std::vector<char> buffer;
    pImpl->_settings.readEntry(jsonFilename, buffer);
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
    auto itLayer = std::find_if(std::begin(pImpl->_layers), std::end(pImpl->_layers), [](const std::unique_ptr<RoomLayer> &pLayer) {
        return pLayer->getZOrder() == 0;
    });
    itLayer->get()->addEntity(obj);
    pImpl->_objects.push_back(std::move(object));
    return obj;
}

Object &Room::createObject(const std::string &image)
{
    auto &texture = pImpl->_textureManager.get(image);

    auto object = std::make_unique<Object>();
    auto animation = std::make_unique<Animation>(texture, "state0");
    auto size = texture.getSize();
    sf::IntRect rect(0, 0, size.x, size.y);
    animation->getRects().push_back(rect);
    animation->getSizes().emplace_back(size);
    animation->getSourceRects().push_back(rect);
    animation->reset();
    object->getAnims().push_back(std::move(animation));

    object->setAnimation("state0");
    auto &obj = *object;
    auto itLayer = std::find_if(std::begin(pImpl->_layers), std::end(pImpl->_layers), [](const std::unique_ptr<RoomLayer> &pLayer) {
        return pLayer->getZOrder() == 0;
    });
    itLayer->get()->addEntity(obj);
    pImpl->_objects.push_back(std::move(object));
    return obj;
}

void Room::drawWalkboxes(sf::RenderWindow &window, sf::RenderStates states) const
{
    if (!pImpl->_showDrawWalkboxes)
        return;

    for (auto &walkbox : pImpl->_graphWalkboxes)
    {
        window.draw(walkbox, states);
    }

    if (pImpl->_path)
    {
        window.draw(*pImpl->_path);
    }

    if (pImpl->_pf && pImpl->_pf->getGraph())
    {
        window.draw(*pImpl->_pf->getGraph(), states);
    }
}

void Room::update(const sf::Time &elapsed)
{
    std::for_each(std::begin(pImpl->_layers), std::end(pImpl->_layers),
                  [elapsed](std::unique_ptr<RoomLayer> &layer) { layer->update(elapsed); });

    std::sort(std::begin(pImpl->_layers), std::end(pImpl->_layers), [](std::unique_ptr<RoomLayer> &a, std::unique_ptr<RoomLayer> &b) {
        return a->getZOrder() > b->getZOrder();
    });
}

void Room::draw(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const
{
    sf::RenderStates states;
    auto ratio = ((float)Screen::Height) / pImpl->_roomSize.y;
    for (const auto &layer : pImpl->_layers)
    {
        auto parallax = layer->getParallax();
        auto posX = (Screen::HalfWidth - cameraPos.x) * parallax.x - Screen::HalfWidth;
        auto posY = (Screen::HalfHeight - cameraPos.y) * parallax.y - Screen::HalfHeight;

        sf::Transform t;
        if (pImpl->_fullscreen == 1)
        {
            t.scale(ratio, ratio);
        }
        t.translate(posX, posY);
        states.transform = t;
        layer->draw(window, states);
    }

    sf::Transform t;
    t.translate(-cameraPos);
    states.transform = t;
    drawWalkboxes(window, states);

    for (const auto &layer : pImpl->_layers)
    {
        auto parallax = layer->getParallax();
        auto posX = (Screen::HalfWidth - cameraPos.x) * parallax.x - Screen::HalfWidth;
        auto posY = (Screen::HalfHeight - cameraPos.y) * parallax.y - Screen::HalfHeight;

        sf::Transform t2;
        t2.translate(posX, posY);
        if (pImpl->_fullscreen == 1)
        {
            t2.scale(ratio, ratio);
        }
        states.transform = t2;
        layer->drawForeground(window, states);
    }
}

const RoomScaling &Room::getRoomScaling() const
{
    return pImpl->_scalings[0];
}

void Room::setWalkboxEnabled(const std::string &name, bool isEnabled)
{
    auto it = std::find_if(pImpl->_walkboxes.begin(), pImpl->_walkboxes.end(), [&name](const Walkbox &walkbox) {
        return walkbox.getName() == name;
    });
    if (it == pImpl->_walkboxes.end())
    {
        std::cerr << "walkbox " << name << " has not been found" << std::endl;
        return;
    }
    it->setEnabled(isEnabled);
    pImpl->updateGraph();
}

bool Room::inWalkbox(const sf::Vector2f &pos) const
{
    auto inWalkbox = std::any_of(pImpl->_walkboxes.begin(), pImpl->_walkboxes.end(), [pos](const Walkbox &w) {
        return w.inside((sf::Vector2i)pos);
    });
    return inWalkbox;
}

std::vector<sf::Vector2i> Room::calculatePath(const sf::Vector2i &start, const sf::Vector2i &end) const
{
    return pImpl->_pf->calculatePath(start, end);
}

} // namespace ng
