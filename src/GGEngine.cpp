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
      _verbSheet(_textureManager, settings),
      _gameSheet(_textureManager, settings),
      _inventoryItems(_textureManager, settings),
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

    _verbSheet.load("VerbSheet");
    _gameSheet.load("GameSheet");
    _inventoryItems.load("InventoryItems");
}

GGEngine::~GGEngine() = default;

sf::IntRect GGEngine::getVerbRect(const std::string &name, std::string lang, bool isRetro) const
{
    std::ostringstream s;
    s << name << (isRetro ? "_retro" : "") << "_" << lang;
    return _verbSheet.getRect(s.str());
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
    states.texture = &_verbSheet.getTexture();

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
            verbShape.setFillColor(_verbUiColors[0].verbNormal);
            verbShape.setPosition(left, top);
            verbShape.setSize(verbSize);
            verbShape.setTexture(&_verbSheet.getTexture());
            verbShape.setTextureRect(rect);
            target.draw(verbShape);
        }
    }

    // inventory arrows
    auto scrollUpFrameRect = _gameSheet.getRect("scroll_up");
    sf::RectangleShape scrollUpShape;
    sf::Vector2f scrollUpPosition(Screen::Width / 2.f, Screen::Height - 3 * Screen::Height / 14.f);
    sf::Vector2f scrollUpSize(scrollUpFrameRect.width * ratio.x, scrollUpFrameRect.height * ratio.y);
    scrollUpShape.setFillColor(_verbUiColors[0].verbNormal);
    scrollUpShape.setPosition(scrollUpPosition);
    scrollUpShape.setSize(scrollUpSize);
    scrollUpShape.setTexture(&_gameSheet.getTexture());
    scrollUpShape.setTextureRect(scrollUpFrameRect);
    target.draw(scrollUpShape);

    auto scrollDownFrameRect = _gameSheet.getRect("scroll_down");
    sf::RectangleShape scrollDownShape;
    scrollDownShape.setFillColor(_verbUiColors[0].verbNormal);
    scrollDownShape.setPosition(scrollUpPosition.x, scrollUpPosition.y + scrollUpFrameRect.height * ratio.y);
    scrollDownShape.setSize(scrollUpSize);
    scrollDownShape.setTexture(&_gameSheet.getTexture());
    scrollDownShape.setTextureRect(scrollDownFrameRect);
    target.draw(scrollDownShape);

    // inventory frame
    // auto inventoryFrameRect = _gameSheet.getRect("inventory_frame");
    // sf::RectangleShape inventoryShape;
    // inventoryShape.setFillColor(_verbUiColors[0].inventoryFrame);
    // inventoryShape.setPosition(sf::Vector2f(scrollUpPosition.x + scrollUpSize.x, Screen::Height - 3 * Screen::Height / 14.f));
    // inventoryShape.setSize(sf::Vector2f(Screen::Width / 2.f - scrollUpSize.x, 3 * Screen::Height / 14.f));
    // inventoryShape.setTexture(&_gameSheet.getTexture());
    // inventoryShape.setTextureRect(inventoryFrameRect);
    // target.draw(inventoryShape);

    auto inventoryFrameRect = _gameSheet.getRect("inventory_background");
    sf::RectangleShape inventoryShape;
    sf::Color c(_verbUiColors[0].inventoryBackground);
    c.a = 128;
    inventoryShape.setFillColor(c);
    inventoryShape.setTexture(&_gameSheet.getTexture());
    inventoryShape.setTextureRect(inventoryFrameRect);
    auto sizeBack = sf::Vector2f(206.f * Screen::Width / 1920.f, 112.f * Screen::Height / 1080.f);
    inventoryShape.setSize(sizeBack);
    auto gapX = 10.f * Screen::Width / 1920.f;
    auto gapY = 10.f * Screen::Height / 1080.f;
    for (auto i = 0; i < 8; i++)
    {
        auto x = (i % 4) * (sizeBack.x + gapX);
        auto y = (i / 4) * (sizeBack.y + gapY);
        inventoryShape.setPosition(sf::Vector2f(scrollUpPosition.x + scrollUpSize.x + x, y + Screen::Height - 3 * Screen::Height / 14.f));
        target.draw(inventoryShape);
    }

    // draw inventory objects
    if (_pCurrentActor)
    {
        auto x = 0;
        auto &objects = _pCurrentActor->getObjects();
        for (auto it = objects.begin(); it != objects.end(); it++)
        {
            auto rect = _inventoryItems.getRect(*it);
            sf::RectangleShape inventoryShape;
            inventoryShape.setPosition(sf::Vector2f(x + scrollUpPosition.x + scrollUpSize.x, Screen::Height - 3 * Screen::Height / 14.f));
            inventoryShape.setSize(sf::Vector2f(rect.width, rect.height));
            inventoryShape.setTexture(&_inventoryItems.getTexture());
            inventoryShape.setTextureRect(rect);
            target.draw(inventoryShape);
            x += rect.width;
        }
    }
}

void GGEngine::playState(GGObject &object, int index)
{
    object.setStateAnimIndex(index);
}

} // namespace gg
