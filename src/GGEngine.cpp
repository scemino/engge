#include <iomanip> // setprecision
#include <sstream> // stringstream
#include <fstream>
#include <regex>
#include <string>
#include "GGEngine.h"
#include "GGFont.h"

namespace gg
{
class _AlphaTo : public TimeFunction
{
    GGObject &_object;
    sf::Color _color;
    sf::Uint8 _alpha;
    sf::Uint8 _alphaDest;
    float _delta;

  public:
    _AlphaTo(GGObject &object, sf::Uint8 alphaDest, const sf::Time &time)
        : TimeFunction(time), _object(object)
    {
        _color = object.getColor();
        _alpha = _color.a;
        _alphaDest = alphaDest;
        _delta = _alphaDest - _alpha;
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
    _FadeTo(GGEngine &engine, sf::Uint8 alphaDest, const sf::Time &time)
        : TimeFunction(time), _engine(engine)
    {
        _alpha = engine.getFadeAlpha();
        _alphaInit = _alpha;
        _alphaDest = alphaDest;
        _delta = _alphaDest - _alpha;
    }

    void operator()() override
    {
        _engine.setFadeAlpha(_alpha);
        if (!isElapsed())
        {
            _alpha = _alphaInit + (_clock.getElapsedTime().asSeconds() / _time.asSeconds()) * _delta;
        }
    }
};

class _CameraPanTo : public TimeFunction
{
    GGEngine &_engine;
    sf::Vector2f _posInit;
    sf::Vector2f _pos;
    sf::Vector2f _posDest;
    sf::Vector2f _delta;

  public:
    _CameraPanTo(GGEngine &engine, const sf::Vector2f &dest, const sf::Time &time)
        : TimeFunction(time),
          _engine(engine),
          _posInit(engine.getCameraAt()),
          _pos(engine.getCameraAt()),
          _posDest(dest),
          _delta(_posDest - _pos)
    {
    }

    void operator()() override
    {
        _engine.setCameraAt(_pos.x, _pos.y);
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
    _OffsetTo(GGObject &object, float x, float y, const sf::Time &time)
        : TimeFunction(time),
          _object(object),
          _dest(x, y),
          _initPos(object.getPosition()),
          _delta(_dest - _initPos),
          _pos(object.getPosition())
    {
    }

    void operator()() override
    {
        _object.setPosition(_pos.x, _pos.y);
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
    float _volumeDest;
    float _delta;

  public:
    _FadeSound(GGEngine &engine, SoundId &sound, float volumeDest, const sf::Time &time)
        : TimeFunction(time), _engine(engine), _sound(sound), _initVolume(sound.sound.getVolume())
    {
        _volume = _initVolume;
        _volumeDest = volumeDest;
        _delta = _volumeDest - _volume;
    }

    virtual ~_FadeSound()
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
    printf("seed: %u\n", seed);
    srand(seed);

    _font.setTextureManager(&_textureManager);
    _font.setSettings(&_settings);
    _font.load("FontModernSheet");

    std::string path(settings.getGamePath());
    path.append("ThimbleweedText_en.tsv");
    _textDb.load(path);
}

GGEngine::~GGEngine()
{
}

void GGEngine::setCameraAt(float x, float y)
{
    _cameraPos = sf::Vector2f(x, y);
}

void GGEngine::moveCamera(float x, float y)
{
    _cameraPos += sf::Vector2f(x, y);
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

void GGEngine::cameraPanTo(float x, float y, float timeInSec)
{
    auto cameraPanTo = std::make_unique<_CameraPanTo>(*this, sf::Vector2f(x, y), sf::seconds(timeInSec));
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
    _functions.erase(std::remove_if(_functions.begin(), _functions.end(), [](std::unique_ptr<Function> &f) { return f->isElapsed(); }), _functions.end());
    for (auto &actor : _actors)
    {
        actor.get()->update(elapsed);
    }
    _room.update(elapsed);
}

void GGEngine::loopMusic(const std::string &name)
{
    std::string path(_settings.getGamePath());
    path.append(name).append(".ogg");
    if (!_music.openFromFile(path))
    {
        printf("Can't load the music\n");
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
        printf("Can't load the sound\n");
        return nullptr;
    }
    _sounds.push_back(std::unique_ptr<SoundId>(sound));
    sound->sound.setBuffer(sound->buffer);
    sound->sound.setLoop(loop);
    sound->sound.play();
    return sound;
}

void GGEngine::fadeOutSound(SoundId &id, float time)
{
    auto t = sf::seconds(time);
    auto fadeSound = std::unique_ptr<_FadeSound>(new _FadeSound(*this, id, 0.0f, t));
    addFunction(std::move(fadeSound));
}

void GGEngine::stopSound(SoundId &sound)
{
    printf("stopSound\n");
    sound.sound.stop();
    // auto it = std::find_if(_sounds.begin(), _sounds.end(), [sound](std::unique_ptr<SoundId> &ptr)
    // {
    //     return ptr.get() == &sound;
    // });
    // _sounds.erase(it);
}

void GGEngine::draw(sf::RenderWindow &window) const
{
    auto cameraPos = _cameraPos;
    _room.draw(window, cameraPos);
    for (auto &actor : _actors)
    {
        actor->draw(window);
    }

    sf::RectangleShape fadeShape;
    fadeShape.setSize(sf::Vector2f(320, 200));
    fadeShape.setFillColor(sf::Color(0, 0, 0, _fadeAlpha));
    window.draw(fadeShape);

    // std::stringstream s;
    // s << "camera: " << std::fixed << std::setprecision(0) << cameraPos.x << ", " << cameraPos.y;
    // g_ggfont.draw(s.str(), window);
}

void GGEngine::offsetTo(GGObject &object, float x, float y, float time)
{
    const auto &pos = object.getPosition();
    auto offsetTo = std::unique_ptr<_OffsetTo>(new _OffsetTo(object, pos.x + x, pos.y + y, sf::seconds(time)));
    addFunction(std::move(offsetTo));
}

void GGEngine::alphaTo(GGObject &object, float a, float time)
{
    auto alphaTo = std::unique_ptr<_AlphaTo>(new _AlphaTo(object, a, sf::seconds(time)));
    addFunction(std::move(alphaTo));
}

void GGEngine::fadeTo(float a, float time)
{
    auto fadeTo = std::unique_ptr<_FadeTo>(new _FadeTo(*this, a, sf::seconds(time)));
    addFunction(std::move(fadeTo));
}

void GGEngine::playState(GGObject &object, int index)
{
    object.setStateAnimIndex(index);
    // addFunction(std::unique_ptr<_PlayState>(new _PlayState(object, index)));
}
} // namespace gg
