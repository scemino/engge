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
class _AlphaTo : public TimeFunction
{
    GGObject &_object;
    sf::Color _color;
    float _alpha;
    float _alphaDestination;
    float _delta;

  public:
    _AlphaTo(GGObject &object, float alphaDestination, const sf::Time &time)
        : TimeFunction(time), _object(object),
          _color(object.getColor()),
          _alpha(_color.a),
          _alphaDestination(alphaDestination),
          _delta(_alphaDestination - _alpha)
    {
    }

    void operator()() override
    {
        _object.setColor(sf::Color(_color.r, _color.g, _color.b, _alpha));
        if (!isElapsed())
        {
            _alpha = _color.a + (_clock.getElapsedTime().asSeconds() / _time.asSeconds()) * _delta;
        }
    }
};

class _FadeTo : public TimeFunction
{
    GGEngine &_engine;
    sf::Uint8 _alpha;
    sf::Uint8 _alphaInit;
    sf::Uint8 _alphaDest;
    float _delta;

  public:
    _FadeTo(GGEngine &engine, sf::Uint8 alphaDestination, const sf::Time &time)
        : TimeFunction(time), _engine(engine)
    {
        _alpha = engine.getFadeAlpha();
        _alphaInit = _alpha;
        _alphaDest = alphaDestination;
        _delta = _alphaDest - _alpha;
    }

    void operator()() override
    {
        _engine.setFadeAlpha(_alpha);
        if (!isElapsed())
        {
            _alpha = static_cast<sf::Uint8>(_alphaInit +
                                            (_clock.getElapsedTime().asSeconds() / _time.asSeconds()) * _delta);
        }
    }
};

class _CameraPanTo : public TimeFunction
{
    GGEngine &_engine;
    sf::Vector2f _posInit;
    sf::Vector2f _pos;
    sf::Vector2f _posDestination;
    sf::Vector2f _delta;

  public:
    _CameraPanTo(GGEngine &engine, const sf::Vector2f &destination, const sf::Time &time)
        : TimeFunction(time),
          _engine(engine),
          _posInit(engine.getCameraAt()),
          _pos(engine.getCameraAt()),
          _posDestination(destination),
          _delta(_posDestination - _pos)
    {
    }

    void operator()() override
    {
        _engine.setCameraAt(_pos);
        if (!isElapsed())
        {
            _pos = _posInit + (_clock.getElapsedTime().asSeconds() / _time.asSeconds()) * _delta;
        }
    }
};

class _OffsetTo : public TimeFunction
{
    GGObject &_object;
    sf::Vector2f _dest;
    sf::Vector2f _initPos;
    sf::Vector2f _delta;
    sf::Vector2f _pos;

  public:
    _OffsetTo(GGObject &object, sf::Vector2f destination, sf::Time time)
        : TimeFunction(time),
          _object(object),
          _dest(destination),
          _initPos(object.getPosition()),
          _delta(_dest - _initPos),
          _pos(object.getPosition())
    {
    }

    void operator()() override
    {
        _object.setPosition(_pos);
        if (!isElapsed())
        {
            _pos = _initPos + (_clock.getElapsedTime().asSeconds() / _time.asSeconds()) * _delta;
        }
    }
};

class _FadeSound : public TimeFunction
{
    GGEngine &_engine;
    SoundId &_sound;
    const float _initVolume;
    float _volume;
    float _volumeDestination;
    float _delta;

  public:
    _FadeSound(GGEngine &engine, SoundId &sound, float volumeDestination, const sf::Time &time)
        : TimeFunction(time), _engine(engine), _sound(sound), _initVolume(sound.sound.getVolume())
    {
        _volume = _initVolume;
        _volumeDestination = volumeDestination;
        _delta = _volumeDestination - _volume;
    }

    ~_FadeSound() override
    {
        _engine.stopSound(_sound);
    }

    void operator()() override
    {
        _sound.sound.setVolume(_volume);
        if (!isElapsed())
        {
            _volume = _initVolume + (_clock.getElapsedTime().asSeconds() / _time.asSeconds()) * _delta;
        }
    }
};

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

void GGEngine::cameraPanTo(const sf::Vector2f &pos, const sf::Time &time)
{
    auto cameraPanTo = std::make_unique<_CameraPanTo>(*this, pos, time);
    addFunction(std::move(cameraPanTo));
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

void GGEngine::fadeOutSound(SoundId &id, const sf::Time &time)
{
    auto fadeSound = std::make_unique<_FadeSound>(*this, id, 0.0f, time);
    addFunction(std::move(fadeSound));
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

void GGEngine::offsetTo(GGObject &object, const sf::Vector2f &offset, const sf::Time &time)
{
    const auto &pos = object.getPosition();
    auto offsetTo = std::make_unique<_OffsetTo>(object, pos + offset, time);
    addFunction(std::move(offsetTo));
}

void GGEngine::alphaTo(GGObject &object, sf::Uint8 a, const sf::Time &time)
{
    auto alphaTo = std::make_unique<_AlphaTo>(object, a, time);
    addFunction(std::move(alphaTo));
}

void GGEngine::fadeTo(float a, const sf::Time &time)
{
    auto fadeTo = std::make_unique<_FadeTo>(*this, a, time);
    addFunction(std::move(fadeTo));
}

void GGEngine::playState(GGObject &object, int index)
{
    object.setStateAnimIndex(index);
}
} // namespace gg
