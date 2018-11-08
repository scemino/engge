#include <memory>

#include <iomanip>
#include <sstream>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include "GGEngine.h"
#include "Screen.h"
#include "GGFont.h"
#include "_GGUtil.h"

namespace gg
{
GGEngine::GGEngine(const GGEngineSettings &settings)
    : _settings(settings),
      _textureManager(settings),
      _fadeAlpha(255),
      _pWindow(nullptr),
      _pRoom(nullptr),
      _pCurrentActor(nullptr),
      _verbTexture(_textureManager.get("VerbSheet")),
      _inputActive(false),
      _showCursor(false)
{
    time_t t;
    auto seed = (unsigned)time(&t);
    std::cout << "seed: " << seed << std::endl;
    srand(seed);

    _font.setTextureManager(&_textureManager);
    _font.setSettings(&_settings);
    _font.load("FontModernSheet");

    std::string path(settings.getGamePath());
    path.append("ThimbleweedText_en.tsv");
    _textDb.load(path);
}

GGEngine::~GGEngine() = default;

sf::IntRect GGEngine::getVerbRect(const std::string &name, std::string lang, bool isRetro) const
{
    // load json file
    std::string jsonFilename;
    jsonFilename.append(_settings.getGamePath()).append("VerbSheet.json");
    nlohmann::json json;
    {
        std::ifstream i(jsonFilename);
        i >> json;
    }

    std::ostringstream s;
    s << name << (isRetro ? "_retro" : "") << "_" << lang;
    auto jVerb = json["frames"][s.str().c_str()];
    if (jVerb.is_null())
        return sf::IntRect();
    return _toRect(jVerb["frame"]);
}

void GGEngine::setCameraAt(const sf::Vector2f &at)
{
    _cameraPos = at;
}

void GGEngine::moveCamera(const sf::Vector2f &offset)
{
    _cameraPos += offset;
    if (_cameraPos.x < 0)
        _cameraPos.x = 0;
    if (_cameraPos.y < 0)
        _cameraPos.y = 0;
    if (!_pRoom)
        return;
    const auto &size = _pRoom->getRoomSize();
    if (_cameraPos.x > size.x - Screen::Width)
        _cameraPos.x = size.x - Screen::Width;
    if (_cameraPos.y > size.y - Screen::Height)
        _cameraPos.y = size.y - Screen::Height;
}

void GGEngine::update(const sf::Time &elapsed)
{
    for (auto &function : _newFunctions)
    {
        _functions.push_back(std::move(function));
    }
    _newFunctions.clear();
    for (auto &function : _functions)
    {
        (*function)();
    }
    _functions.erase(std::remove_if(_functions.begin(), _functions.end(),
                                    [](std::unique_ptr<Function> &f) { return f->isElapsed(); }),
                     _functions.end());
    for (auto &actor : _actors)
    {
        actor->update(elapsed);
    }
    if (!_pRoom)
        return;
    _pRoom->update(elapsed);
}

void GGEngine::loopMusic(const std::string &name)
{
    std::string path(_settings.getGamePath());
    path.append(name).append(".ogg");
    if (!_music.openFromFile(path))
    {
        std::cerr << "Can't load the music" << std::endl;
        return;
    }
    _music.setLoop(true);
    _music.play();
}

std::shared_ptr<SoundId> GGEngine::playSound(const std::string &name, bool loop)
{
    std::string path(_settings.getGamePath());
    path.append(name);
    auto sound = std::make_shared<SoundId>();
    if (!sound->buffer.loadFromFile(path))
    {
        std::cerr << "Can't load the sound" << std::endl;
        return nullptr;
    }
    _sounds.push_back(sound);
    sound->sound.setBuffer(sound->buffer);
    sound->sound.setLoop(loop);
    sound->sound.play();
    return sound;
}

std::shared_ptr<SoundId> GGEngine::defineSound(const std::string &name)
{
    std::string path(_settings.getGamePath());
    path.append(name);
    auto sound = std::make_shared<SoundId>();
    if (!sound->buffer.loadFromFile(path))
    {
        std::cerr << "Can't load the sound" << std::endl;
        return nullptr;
    }
    _sounds.push_back(sound);
    sound->sound.setBuffer(sound->buffer);
    return sound;
}

void GGEngine::stopSound(SoundId &sound)
{
    std::cout << "stopSound" << std::endl;
    sound.sound.stop();
}

void GGEngine::draw(sf::RenderWindow &window) const
{
    auto cameraPos = _cameraPos;
    if (!_pRoom)
        return;
    _pRoom->draw(window, cameraPos);
    for (auto &actor : _actors)
    {
        actor->draw(window, cameraPos);
    }

    sf::RectangleShape fadeShape;
    fadeShape.setSize(sf::Vector2f(Screen::Width, Screen::Height));
    fadeShape.setFillColor(sf::Color(0, 0, 0, _fadeAlpha));
    window.draw(fadeShape);

    // draw verbs
    if (_inputActive && !_verbSlots[0].getVerb(0).id.empty())
    {
        sf::Vector2f size(Screen::Width / 6.f, Screen::Height / 3.f / 3.f);
        for (int i = 0; i < 9; i++)
        {
            auto verb = _verbSlots[0].getVerb(i + 1);
            auto rect = getVerbRect(verb.id);
            sf::VertexArray triangle(sf::Quads, 4);

            auto x = (i / 3) * size.x;
            auto y = Screen::Height - size.y * 3 + (i % 3) * size.y;
            triangle[0].position = sf::Vector2f(x, y);
            triangle[1].position = sf::Vector2f(x + size.x, y);
            triangle[2].position = sf::Vector2f(x + size.x, y + size.y);
            triangle[3].position = sf::Vector2f(x, y + size.y);
            triangle[0].texCoords = sf::Vector2f(rect.left, rect.top);
            triangle[1].texCoords = sf::Vector2f(rect.left + rect.width, rect.top);
            triangle[2].texCoords = sf::Vector2f(rect.left + rect.width, rect.top + rect.height);
            triangle[3].texCoords = sf::Vector2f(rect.left, rect.top + rect.height);

            sf::RenderStates states;
            states.texture = &_verbTexture;

            window.draw(triangle, states);
        }
    }

    // std::stringstream s;
    // s << "camera: " << std::fixed << std::setprecision(0) << cameraPos.x << ", " << cameraPos.y;
    // _font.draw(s.str(), window);
}

void GGEngine::playState(GGObject &object, int index)
{
    object.setStateAnimIndex(index);
}

SoundId::~SoundId()
{
    sound.stop();
}
} // namespace gg
