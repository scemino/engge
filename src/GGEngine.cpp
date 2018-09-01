#include "GGEngine.h"
#include <squirrel3/squirrel.h>

namespace gg
{
// class _PlayState : public Function
// {
//   private:
//     GGObject &_object;
//     sf::Clock _clock;
//     sf::Time _time;
//     const int _animIndex;

//   public:
//     _PlayState(GGObject &object, int animIndex)
//         : _object(object), _time(sf::milliseconds(100)), _animIndex(animIndex)
//     {
//         _object.setStateAnimIndex(_animIndex, 0);
//     }

//     bool isElapsed() override
//     {
//         return _object.getStateAnimIndex(_animIndex) == (_object.getStateAnimLength(_animIndex) - 1);
//     }

//     void operator()() override
//     {
//         if (_clock.getElapsedTime() > _time)
//         {
//             auto index = _object.getStateAnimIndex(_animIndex);
//             auto length = _object.getStateAnimLength(_animIndex);
//             index = (index + 1) % length;
//             _object.setStateAnimIndex(_animIndex, index);
//             _time = _clock.getElapsedTime() + sf::milliseconds(50);
//         }
//     }
// };

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

class _OffsetTo : public TimeFunction
{
    GGObject &_object;
    sf::Vector2f _dest;
    sf::Vector2f _delta;
    sf::Vector2f _pos;
    sf::Vector2f _initPos;

  public:
    _OffsetTo(GGObject &object, float x, float y, const sf::Time &time)
        : TimeFunction(time), _object(object), _dest(x, y)
    {
        _pos = object.getPosition();
        _initPos = _pos;
        _delta = _dest - _initPos;
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

sf::RectangleShape fadeShape;

GGEngine::GGEngine(const GGEngineSettings &settings)
    : _settings(settings),
      _textureManager(settings),
      _room(_textureManager, settings)
{
    time_t t;
    auto seed = (unsigned)time(&t);
    printf("seed: %u\n", seed);
    srand(seed);
    fadeShape.setSize(sf::Vector2f(320, 200));
    _fadeAlpha = 0;
}

GGEngine::~GGEngine()
{
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
    fadeShape.setFillColor(sf::Color(0, 0, 0, _fadeAlpha));
    _room.draw(window);
    for (auto &actor : _actors)
    {
        actor->draw(window);
    }
    window.draw(fadeShape);
}

void GGEngine::offsetTo(GGObject &object, float x, float y, float time)
{
    auto offsetTo = std::unique_ptr<_OffsetTo>(new _OffsetTo(object, x, y, sf::seconds(time)));
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
