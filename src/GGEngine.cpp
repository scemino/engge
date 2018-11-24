#include <memory>

#include <iomanip>
#include <sstream>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <math.h>
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
      _gameSheetTexture(_textureManager.get("GameSheet")),
      _inventoryItemsTexture(_textureManager.get("InventoryItems")),
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

    // load all messages
    std::string path(settings.getGamePath());
    path.append("ThimbleweedText_en.tsv");
    _textDb.load(path);

    // load json verb file
    std::string jsonFilename;
    jsonFilename.append(_settings.getGamePath()).append("VerbSheet.json");
    {
        std::ifstream i(jsonFilename);
        i >> _jsonVerb;
    }

    jsonFilename.clear();
    jsonFilename.append(_settings.getGamePath()).append("GameSheet.json");
    {
        std::ifstream i(jsonFilename);
        i >> _jsonGameSheet;
    }

    jsonFilename.clear();
    jsonFilename.append(_settings.getGamePath()).append("InventoryItems.json");
    {
        std::ifstream i(jsonFilename);
        i >> _jsonInventoryItems;
    }
}

GGEngine::~GGEngine() = default;

sf::IntRect GGEngine::getVerbRect(const std::string &name, std::string lang, bool isRetro) const
{
    std::ostringstream s;
    s << name << (isRetro ? "_retro" : "") << "_" << lang;
    auto jVerb = _jsonVerb["frames"][s.str().c_str()];
    if (jVerb.is_null())
        return sf::IntRect();
    return _toRect(jVerb["frame"]);
}

sf::IntRect GGEngine::getGameSheetRect(const std::string &name) const
{
    auto jFrames = _jsonGameSheet["frames"][name.c_str()];
    if (jFrames.is_null())
        return sf::IntRect();
    return _toRect(jFrames["frame"]);
}

sf::IntRect GGEngine::getInventoryItemsRect(const std::string &name) const
{
    auto jFrames = _jsonInventoryItems["frames"][name.c_str()];
    if (jFrames.is_null())
        return sf::IntRect();
    return _toRect(jFrames["frame"]);
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

std::shared_ptr<SoundDefinition> GGEngine::defineSound(const std::string &name)
{
    std::string path(_settings.getGamePath());
    path.append(name);
    auto sound = std::make_shared<SoundDefinition>(path);
    _sounds.push_back(sound);
    return sound;
}

std::shared_ptr<SoundId> GGEngine::playSound(SoundDefinition &soundDefinition, bool loop)
{
    auto soundId = std::make_shared<SoundId>(soundDefinition);
    _soundIds.push_back(soundId);
    soundId->play(loop);
    return soundId;
}

void GGEngine::stopSound(SoundId &sound)
{
    std::cout << "stopSound" << std::endl;
    sound.stop();
    auto it = std::find_if(_soundIds.begin(), _soundIds.end(), [&sound](const std::shared_ptr<SoundId> &id) {
        return id.get() == &sound;
    });
    if (it == _soundIds.end())
        return;
    _soundIds.erase(it);
}

void GGEngine::draw(sf::RenderWindow &window) const
{
    auto cameraPos = _cameraPos;
    if (!_pRoom)
        return;
    _pRoom->draw(window, cameraPos);
    for (auto &actor : _actors)
    {
        window.draw(*actor);
    }

    // draw fade
    sf::RectangleShape fadeShape;
    fadeShape.setSize(sf::Vector2f(Screen::Width, Screen::Height));
    fadeShape.setFillColor(sf::Color(0, 0, 0, _fadeAlpha));
    window.draw(fadeShape);

    // draw verbs
    if (_inputActive && !_verbSlots[0].getVerb(0).id.empty())
    {
        drawVerbs(window);
    }

    // std::stringstream s;
    // s << "camera: " << std::fixed << std::setprecision(0) << cameraPos.x << ", " << cameraPos.y;
    // _font.draw(s.str(), window);
}

void GGEngine::drawVerbs(sf::RenderTarget &target) const
{
    sf::RenderStates states;
    states.texture = &_verbTexture;

    sf::Vector2f size(Screen::Width / 6.f, Screen::Height / 14.f);
    auto ratio = sf::Vector2f(Screen::Width / 1280.f, Screen::Height / 720.f);
    for (int x = 0; x < 3; x++)
    {
        auto maxW = 0;
        for (int y = 0; y < 3; y++)
        {
            auto verb = _verbSlots[0].getVerb(x * 3 + y + 1);
            auto rect = getVerbRect(verb.id);
            maxW = fmax(maxW, rect.width * ratio.x);
        }
        auto padding = (size.x - maxW) / 2.f;
        auto left = padding + x * size.x;

        for (int y = 0; y < 3; y++)
        {
            auto top = Screen::Height - size.y * 3 + y * size.y;
            auto verb = _verbSlots[0].getVerb(x * 3 + y + 1);
            auto rect = getVerbRect(verb.id);
            auto verbSize = sf::Vector2f(rect.width * ratio.x, rect.height * ratio.y);

            sf::RectangleShape verbShape;
            verbShape.setPosition(left, top);
            verbShape.setSize(verbSize);
            verbShape.setTexture(&_verbTexture);
            verbShape.setTextureRect(rect);
            target.draw(verbShape);
        }
    }

    // inventory arrows
    auto scrollUpFrameRect = getGameSheetRect("scroll_up");
    sf::RectangleShape scrollUpShape;
    sf::Vector2f scrollUpPosition(Screen::Width / 2.f, Screen::Height - 3 * Screen::Height / 14.f);
    sf::Vector2f scrollUpSize(scrollUpFrameRect.width * ratio.x, scrollUpFrameRect.height * ratio.y);
    scrollUpShape.setPosition(scrollUpPosition);
    scrollUpShape.setSize(scrollUpSize);
    scrollUpShape.setTexture(&_gameSheetTexture);
    scrollUpShape.setTextureRect(scrollUpFrameRect);
    target.draw(scrollUpShape);

    auto scrollDownFrameRect = getGameSheetRect("scroll_down");
    sf::RectangleShape scrollDownShape;
    scrollDownShape.setPosition(scrollUpPosition.x, scrollUpPosition.y + scrollUpFrameRect.height * ratio.y);
    scrollDownShape.setSize(scrollUpSize);
    scrollDownShape.setTexture(&_gameSheetTexture);
    scrollDownShape.setTextureRect(scrollDownFrameRect);
    target.draw(scrollDownShape);

    // inventory frame
    auto inventoryFrameRect = getGameSheetRect("inventory_frame");
    sf::RectangleShape inventoryShape;
    inventoryShape.setPosition(sf::Vector2f(scrollUpPosition.x + scrollUpSize.x, Screen::Height - 3 * Screen::Height / 14.f));
    inventoryShape.setSize(sf::Vector2f(Screen::Width / 2.f - scrollUpSize.x, 3 * Screen::Height / 14.f));
    inventoryShape.setTexture(&_gameSheetTexture);
    inventoryShape.setTextureRect(inventoryFrameRect);
    target.draw(inventoryShape);

    if (_pCurrentActor)
    {
        auto &objects = _pCurrentActor->getObjects();
        for (auto it = objects.begin(); it != objects.end(); it++)
        {
            auto rect = getInventoryItemsRect(*it);
            inventoryShape.setPosition(sf::Vector2f(scrollUpPosition.x + scrollUpSize.x, Screen::Height - 3 * Screen::Height / 14.f));
            inventoryShape.setSize(sf::Vector2f(rect.width, rect.height));
            inventoryShape.setTexture(&_inventoryItemsTexture);
            inventoryShape.setTextureRect(rect);
            target.draw(inventoryShape);
        }
    }
}

void GGEngine::playState(GGObject &object, int index)
{
    object.setStateAnimIndex(index);
}

} // namespace gg
