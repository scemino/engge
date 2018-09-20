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

namespace gg
{
GGEngine::GGEngine(const GGEngineSettings &settings)
    : _settings(settings),
      _textureManager(settings),
      _room(_textureManager, settings),
      _fadeAlpha(0),
      _pWindow(nullptr)
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
    const auto &size = _room.getRoomSize();
    if (_cameraPos.x > size.x - 320)
        _cameraPos.x = size.x - 320;
    if (_cameraPos.y > size.y - 180)
        _cameraPos.y = size.y - 180;
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
    _room.update(elapsed);
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

SoundId *GGEngine::playSound(const std::string &name, bool loop)
{
    std::string path(_settings.getGamePath());
    path.append(name);
    auto sound = new SoundId();
    if (!sound->buffer.loadFromFile(path))
    {
        std::cerr << "Can't load the sound" << std::endl;
        return nullptr;
    }
    _sounds.push_back(std::unique_ptr<SoundId>(sound));
    sound->sound.setBuffer(sound->buffer);
    sound->sound.setLoop(loop);
    sound->sound.play();
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
    _room.draw(window, cameraPos);
    for (auto &actor : _actors)
    {
        actor->draw(window, cameraPos);
    }

    sf::RectangleShape fadeShape;
    fadeShape.setSize(sf::Vector2f(Screen::Width, Screen::Height));
    fadeShape.setFillColor(sf::Color(0, 0, 0, _fadeAlpha));
    window.draw(fadeShape);

    // std::stringstream s;
    // s << "camera: " << std::fixed << std::setprecision(0) << cameraPos.x << ", " << cameraPos.y;
    // _font.draw(s.str(), window);
}

void GGEngine::playState(GGObject &object, int index)
{
    object.setStateAnimIndex(index);
}
} // namespace gg
