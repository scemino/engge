#include "Room.h"
#include "Animation.h"
#include "JsonTokenReader.h"
#include "Light.h"
#include "Locator.h"
#include "Logger.h"
#include "PathFinder.h"
#include "ResourceManager.h"
#include "RoomLayer.h"
#include "RoomScaling.h"
#include "SpriteSheet.h"
#include "TextObject.h"
#include "Thread.h"
#include "_Util.h"
#include "squirrel.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <list>
#include <math.h>
#include <memory>

namespace ng
{
struct CmpLayer
{
    bool operator()(int a, int b) const { return a > b; }
};

struct Room::Impl
{
    TextureManager &_textureManager;
    EngineSettings &_settings;
    std::vector<std::unique_ptr<Object>> _objects;
    std::vector<Walkbox> _walkboxes;
    std::map<int, std::unique_ptr<RoomLayer>, CmpLayer> _layers;
    std::vector<RoomScaling> _scalings;
    RoomScaling _scaling;
    sf::Vector2i _roomSize;
    int32_t _screenHeight{0};
    bool _showDrawWalkboxes{false};
    std::string _sheet;
    std::string _name;
    int _fullscreen{0};
    HSQOBJECT _table{};
    std::shared_ptr<Path> _path;
    std::shared_ptr<PathFinder> _pf;
    std::vector<Walkbox> _graphWalkboxes;
    sf::Color _ambientColor{255, 255, 255, 255};
    SpriteSheet _spriteSheet;
    Room *_pRoom{nullptr};
    std::vector<std::unique_ptr<Light>> _lights;
    float _rotation{0};
    std::vector<std::unique_ptr<ThreadBase>> _threads;
    sf::Shader _shader{};
    int _selectedEffect{RoomEffectConstants::EFFECT_NONE};
    sf::Color _overlayColor{sf::Color::Transparent};

    Impl(TextureManager &textureManager, EngineSettings &settings)
        : _textureManager(textureManager), _settings(settings)
    {
        _spriteSheet.setTextureManager(&textureManager);
        _spriteSheet.setSettings(&settings);
        for (int i = -3; i < 6; ++i)
        {
            _layers[i] = std::make_unique<RoomLayer>();
        }

        const std::string vertexShader = "void main()"
                                         "{"
                                         "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
                                         "    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;"
                                         "    gl_FrontColor = gl_Color;"
                                         "}";

        // load vertex shader
        if (!_shader.loadFromMemory(vertexShader, sf::Shader::Type::Vertex))
        {
            std::cerr << "Error loading shaders" << std::endl;
            return;
        }
    }

    void setEffect(int effect)
    {
        if (effect == RoomEffectConstants::EFFECT_BLACKANDWHITE)
        {
            _selectedEffect = effect;
            const std::string fragmentShader = "uniform sampler2D texture;\n"
                                               "void main()\n"
                                               "{\n"
                                               "vec4 texColor = texture2D(texture, gl_TexCoord[0].xy);\n"
                                               "vec4 col = gl_Color * texColor;\n"
                                               "float gray = dot(col.xyz, vec3(0.299, 0.587, 0.114));\n"
                                               "gl_FragColor = vec4(gray, gray, gray, col.a);\n"
                                               "}";
            if (!_shader.loadFromMemory(fragmentShader, sf::Shader::Type::Fragment))
            {
                std::cerr << "Error loading shaders" << std::endl;
            }
            _shader.setUniform("texture", sf::Shader::CurrentTexture);
            return;
        }
        _selectedEffect = RoomEffectConstants::EFFECT_NONE;
    }

    void setRoom(Room *pRoom) { _pRoom = pRoom; }

    void loadBackgrounds(GGPackValue &jWimpy)
    {
        int width = 0;
        if (!jWimpy["fullscreen"].isNull())
        {
            _fullscreen = jWimpy["fullscreen"].int_value;
        }
        if (jWimpy["background"].isArray())
        {
            for (auto &bg : jWimpy["background"].array_value)
            {
                auto frame = _spriteSheet.getRect(bg.string_value);
                auto sourceRect = _spriteSheet.getSpriteSourceSize(bg.string_value);
                auto sprite = sf::Sprite();
                sprite.move(sourceRect.left + width, sourceRect.top);
                sprite.setTexture(_textureManager.get(_sheet));
                sprite.setTextureRect(frame);
                width += sprite.getTextureRect().width;
                _layers[0]->getSprites().push_back(sprite);
            }
        }
        else if (jWimpy["background"].isString())
        {
            auto frame = _spriteSheet.getRect(jWimpy["background"].string_value);
            auto sourceRect = _spriteSheet.getSpriteSourceSize(jWimpy["background"].string_value);
            auto sprite = sf::Sprite();
            sprite.move(sourceRect.left, sourceRect.top);
            sprite.setTexture(_textureManager.get(_sheet));
            sprite.setTextureRect(frame);
            _layers[0]->getSprites().push_back(sprite);
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
            auto zsort = jLayer["zsort"].int_value;
            auto &layer = _layers[zsort];
            layer->setZOrder(zsort);
            if (jLayer["name"].isArray())
            {
                float offsetX = 0;
                for (const auto &jName : jLayer["name"].array_value)
                {
                    auto layerName = jName.string_value;
                    auto rect = _spriteSheet.getRect(layerName);
                    auto sourceRect = _spriteSheet.getSpriteSourceSize(layerName);
                    sf::Sprite s;
                    s.setTexture(_textureManager.get(_sheet));
                    s.setTextureRect(rect);
                    s.setOrigin(sf::Vector2f(-sourceRect.left, -sourceRect.top));
                    s.move(sf::Vector2f(offsetX, 0));
                    offsetX += rect.width;
                    layer->getSprites().push_back(s);
                }
            }
            else
            {
                auto layerName = jLayer["name"].string_value;
                auto rect = _spriteSheet.getRect(layerName);
                auto sourceRect = _spriteSheet.getSpriteSourceSize(layerName);
                sf::Sprite s;
                s.setTexture(_textureManager.get(_sheet));
                s.setTextureRect(rect);
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
        }
    }

    void loadObjects(GGPackValue &jWimpy)
    {
        auto &texture = _textureManager.get(_sheet);

        for (auto jObject : jWimpy["objects"].array_value)
        {
            auto object = std::make_unique<Object>();
            // name
            auto objectName = jObject["name"].string_value;
            object->setName(objectName);
            // parent
            if (jObject["parent"].isString())
            {
                auto parent = jObject["parent"].string_value;
                auto it = std::find_if(_objects.begin(), _objects.end(), [&parent](const std::unique_ptr<Object> &o) {
                    return o->getName() == parent;
                });
                if (it != _objects.end())
                {
                    (*it)->addChild(object.get());
                }
            }
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

            object->setPosition(sf::Vector2f(pos.x, _roomSize.y - pos.y));
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
            _layers[0]->addEntity(*object);
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
                    auto scale = std::strtof(value.substr(0, index).c_str(), nullptr);
                    auto yPos = std::strtof(value.substr(index + 1).c_str(), nullptr);
                    Scaling s{};
                    s.scale = scale;
                    s.yPos = yPos;
                    scaling.getScalings().push_back(s);
                }
                _scalings.push_back(scaling);
            }
            else if (jWimpy["scaling"][0].isHash())
            {
                for (auto jScaling : jWimpy["scaling"].array_value)
                {
                    RoomScaling scaling;
                    if (jScaling["trigger"].isString())
                    {
                        scaling.setTrigger(jScaling["trigger"].string_value);
                    }
                    for (auto jSubScaling : jScaling["scaling"].array_value)
                    {
                        if (jSubScaling.isString())
                        {
                            auto value = jSubScaling.string_value;
                            auto index = value.find('@');
                            auto scale = std::strtof(value.substr(0, index).c_str(), nullptr);
                            auto yPos = std::strtof(value.substr(index + 1).c_str(), nullptr);
                            Scaling s{};
                            s.scale = scale;
                            s.yPos = yPos;
                            scaling.getScalings().push_back(s);
                        }
                        else if (jSubScaling.isArray())
                        {
                            for (auto jSubScalingScaling : jSubScaling.array_value)
                            {
                                auto value = jSubScalingScaling.string_value;
                                auto index = value.find('@');
                                auto scale = std::strtof(value.substr(0, index).c_str(), nullptr);
                                auto yPos = std::strtof(value.substr(index + 1).c_str(), nullptr);
                                Scaling s{};
                                s.scale = scale;
                                s.yPos = yPos;
                                scaling.getScalings().push_back(s);
                            }
                        }
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

    void drawFade(sf::RenderWindow &window) const
    {
        sf::RectangleShape fadeShape;
        auto screen = window.getView().getSize();
        fadeShape.setSize(sf::Vector2f(screen.x, screen.y));
        fadeShape.setFillColor(_overlayColor);
        window.draw(fadeShape);
    }
};

Room::Room(TextureManager &textureManager, EngineSettings &settings)
    : pImpl(std::make_unique<Impl>(textureManager, settings))
{
    _id = Locator::getResourceManager().getRoomId();
    pImpl->setRoom(this);
}

Room::~Room() = default;

void Room::setName(const std::string& name) { pImpl->_name = name; }
std::string Room::getName() const { return pImpl->_name; }

std::vector<std::unique_ptr<Object>> &Room::getObjects() { return pImpl->_objects; }

std::vector<std::unique_ptr<Light>> &Room::getLights() { return pImpl->_lights; }

const std::string &Room::getSheet() const { return pImpl->_sheet; }

void Room::showDrawWalkboxes(bool show) { pImpl->_showDrawWalkboxes = show; }

bool Room::areDrawWalkboxesVisible() const { return pImpl->_showDrawWalkboxes; }

std::vector<Walkbox> &Room::getWalkboxes() { return pImpl->_walkboxes; }

sf::Vector2i Room::getRoomSize() const { return pImpl->_roomSize; }

int32_t Room::getScreenHeight() const { return pImpl->_screenHeight; }

int32_t Room::getFullscreen() const { return pImpl->_fullscreen; }

HSQOBJECT &Room::getTable() { return pImpl->_table; }

bool Room::walkboxesVisible() const { return pImpl->_showDrawWalkboxes; }

void Room::setAmbientLight(sf::Color color) { pImpl->_ambientColor = color; }

sf::Color Room::getAmbientLight() const { return pImpl->_ambientColor; }

void Room::setAsParallaxLayer(Entity *pEntity, int layerNum)
{
    for (auto &layer : pImpl->_layers)
    {
        layer.second->removeEntity(*pEntity);
    }
    pImpl->_layers[layerNum]->addEntity(*pEntity);
}

void Room::roomLayer(int layerNum, bool enabled) { pImpl->_layers[layerNum]->setEnabled(enabled); }

void Room::removeEntity(Entity *pEntity)
{
    for (auto &layer : pImpl->_layers)
    {
        layer.second->removeEntity(*pEntity);
    }
    pImpl->_objects.erase(std::remove_if(pImpl->_objects.begin(), pImpl->_objects.end(),
                                    [pEntity](std::unique_ptr<Object> &pObj) { return pObj.get() == pEntity; }),
                     pImpl->_objects.end());
}

void Room::load(const char *name)
{
    pImpl->_name = name;

    // load wimpy file
    std::string wimpyFilename;
    wimpyFilename.append(name).append(".wimpy");
    trace("Load room {}", wimpyFilename);

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
    pImpl->_screenHeight = hash["height"].int_value;
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
    obj.setRoom(this);
    pImpl->_objects.push_back(std::move(object));
    pImpl->_layers[0]->addEntity(obj);
    return obj;
}

void Room::deleteObject(Object &object) { pImpl->_layers[0]->removeEntity(object); }

Object &Room::createObject(const std::vector<std::string> &anims) { return createObject(pImpl->_sheet, anims); }

Object &Room::createObject(const std::string &sheet, const std::vector<std::string> &anims)
{
    auto &texture = pImpl->_textureManager.get(sheet);

    // load json file
    std::string jsonFilename;
    jsonFilename.append(sheet).append(".json");
    std::vector<char> buffer;
    pImpl->_settings.readEntry(jsonFilename, buffer);
    auto json = ng::Json::Parser::parse(buffer);

    auto object = std::make_unique<Object>();
    auto animation = std::make_unique<Animation>(texture, "state0");
    for (const auto &n : anims)
    {
        if (json["frames"][n].isNull())
            continue;
        auto frame = json["frames"][n]["frame"];
        animation->getRects().push_back(_toRect(frame));
        animation->getSizes().push_back(_toSize(json["frames"][n]["sourceSize"]));
        animation->getSourceRects().push_back(_toRect(json["frames"][n]["spriteSourceSize"]));
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
    obj.setTemporary(true);
    obj.setRoom(this);
    obj.setZOrder(1);
    pImpl->_layers[0]->addEntity(obj);
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
    obj.setTemporary(true);
    obj.setZOrder(1);
    obj.setRoom(this);
    pImpl->_layers[0]->addEntity(obj);
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
    for (auto &&layer : pImpl->_layers)
    {
        layer.second->update(elapsed);
    }
}

void Room::draw(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const
{
    sf::RenderStates states;
    if (pImpl->_selectedEffect != RoomEffectConstants::EFFECT_NONE)
    {
        states.shader = &pImpl->_shader;
    }

    auto screen = window.getView().getSize();
    auto w = screen.x / 2.f;
    auto h = screen.y / 2.f;

    for (const auto &layer : pImpl->_layers)
    {
        auto parallax = layer.second->getParallax();
        auto posX = (w - cameraPos.x) * parallax.x - w;
        auto posY = (h - cameraPos.y) * parallax.y - h;

        sf::Transform t;
        t.rotate(pImpl->_rotation, w, h);
        t.translate(posX + (w - w * parallax.x), posY + (h - h * parallax.y));
        states.transform = t;
        layer.second->draw(window, states);
    }
}

void Room::drawForeground(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const
{
    sf::RenderStates states;
    if (pImpl->_selectedEffect != RoomEffectConstants::EFFECT_NONE)
    {
        states.shader = &pImpl->_shader;
    }

    auto screen = window.getView().getSize();
    auto w = screen.x / 2.f;
    auto h = screen.y / 2.f;

    sf::Transform t;
    t.rotate(pImpl->_rotation, w, h);
    t.translate(-cameraPos);
    states.transform = t;
    drawWalkboxes(window, states);
    pImpl->drawFade(window);

    for (const auto &layer : pImpl->_layers)
    {
        auto parallax = layer.second->getParallax();
        auto posX = (w - cameraPos.x) * parallax.x - w;
        auto posY = (h - cameraPos.y) * parallax.y - h;

        sf::Transform t2;
        t2.rotate(pImpl->_rotation, w, h);
        t2.translate(posX + (w - w * parallax.x), posY + (h - h * parallax.y));
        states.transform = t2;
        layer.second->drawForeground(window, states);
    }
}

const RoomScaling &Room::getRoomScaling() const { return pImpl->_scaling; }

void Room::setRoomScaling(const RoomScaling &scaling) { pImpl->_scaling = scaling; }

void Room::setWalkboxEnabled(const std::string &name, bool isEnabled)
{
    auto it = std::find_if(pImpl->_walkboxes.begin(), pImpl->_walkboxes.end(),
                           [&name](const Walkbox &walkbox) { return walkbox.getName() == name; });
    if (it == pImpl->_walkboxes.end())
    {
        error("walkbox {} has not been found", name);
        return;
    }
    it->setEnabled(isEnabled);
    pImpl->updateGraph();
}

bool Room::inWalkbox(const sf::Vector2f &pos) const
{
    auto inWalkbox = std::any_of(pImpl->_walkboxes.begin(), pImpl->_walkboxes.end(),
                                 [pos](const Walkbox &w) { return w.inside((sf::Vector2i)pos); });
    return inWalkbox;
}

std::vector<RoomScaling> &Room::getScalings() { return pImpl->_scalings; }

std::vector<sf::Vector2i> Room::calculatePath(const sf::Vector2i &start, const sf::Vector2i &end) const
{
    return pImpl->_pf->calculatePath(start, end);
}

float Room::getRotation() const { return pImpl->_rotation; }

void Room::setRotation(float angle) { pImpl->_rotation = angle; }

Light *Room::createLight(sf::Color color, sf::Vector2i pos)
{
    auto light = std::make_unique<Light>(color, pos);
    Light *pLight = light.get();
    pImpl->_lights.emplace_back(std::move(light));
    return pLight;
}

void Room::addThread(std::unique_ptr<ThreadBase> thread) { pImpl->_threads.emplace_back(std::move(thread)); }

void Room::exit()
{
    pImpl->_threads.clear();
    for (auto &obj : pImpl->_objects) {
        if(!obj->isTemporary()) continue;
        for (auto &layer : pImpl->_layers)
        {
            layer.second->removeEntity(*obj);
        }
    }
    pImpl->_objects.erase(std::remove_if(pImpl->_objects.begin(), pImpl->_objects.end(),
                                    [](auto &pObj) { return pObj->isTemporary(); }),
                     pImpl->_objects.end());
}

bool Room::isThreadAlive(HSQUIRRELVM thread) const
{
    return std::find_if(pImpl->_threads.begin(), pImpl->_threads.end(),
                        [&thread](const std::unique_ptr<ThreadBase> &t) { return t->getThread() == thread; }) !=
           pImpl->_threads.end();
}

void Room::stopThread(HSQUIRRELVM thread)
{
    auto it = std::find_if(pImpl->_threads.begin(), pImpl->_threads.end(),
                           [&thread](const std::unique_ptr<ThreadBase> &t) { return t->getThread() == thread; });
    if (it == pImpl->_threads.end())
        return;
    pImpl->_threads.erase(it);
}

void Room::setEffect(int effect) { pImpl->setEffect(effect); }

int Room::getEffect() const { return pImpl->_selectedEffect; }

void Room::setOverlayColor(sf::Color color) { pImpl->_overlayColor = color; }

sf::Color Room::getOverlayColor() const { return pImpl->_overlayColor; }

} // namespace ng
