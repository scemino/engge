#pragma once
#include <random>
#include "squirrel.h"
#include "Engine.h"

namespace ng
{
class _NoTrigger : public Trigger
{
private:
    void trig() override
    {
    }
};
class _SoundTrigger : public Trigger
{
public:
    _SoundTrigger(Engine &engine, const std::vector<SoundDefinition *> &sounds)
        : _engine(engine), _distribution(0, sounds.size() - 1)
    {
        _soundsDefinitions.resize(sounds.size());
        for (size_t i = 0; i < sounds.size(); i++)
        {
            _soundsDefinitions[i] = std::shared_ptr<SoundDefinition>(sounds[i]);
        }
        _sounds.resize(sounds.size());
        for (size_t i = 0; i < sounds.size(); i++)
        {
            _sounds[i] = nullptr;
        }
    }

private:
    void trig() override
    {
        int i = _distribution(_generator);
        if (_sounds[i])
        {
            _sounds[i]->play();
            return;
        }
        _sounds[i] = _engine.getSoundManager().playSound(_soundsDefinitions[i]);
    }

private:
    Engine &_engine;
    std::vector<std::shared_ptr<SoundDefinition>> _soundsDefinitions;
    std::vector<std::shared_ptr<SoundId>> _sounds;
    std::default_random_engine _generator;
    std::uniform_int_distribution<int> _distribution;
};

class _SoundPack : public Pack
{
private:
    static Engine *g_pEngine;

private:
    void addTo(ScriptEngine &engine) const override
    {
        g_pEngine = &engine.getEngine();
        engine.registerGlobalFunction(actorSound, "actorSound");
        engine.registerGlobalFunction(defineSound, "defineSound");
        engine.registerGlobalFunction(fadeOutSound, "fadeOutSound");
        engine.registerGlobalFunction(isSoundPlaying, "isSoundPlaying");
        engine.registerGlobalFunction(loopSound, "loopSound");
        engine.registerGlobalFunction(loopObjectSound, "loopObjectSound");
        engine.registerGlobalFunction(loopMusic, "loopMusic");
        engine.registerGlobalFunction(playMusic, "playMusic");
        engine.registerGlobalFunction(playSound, "playSound");
        engine.registerGlobalFunction(playSoundVolume, "playSoundVolume");
        engine.registerGlobalFunction(playObjectSound, "playObjectSound");
        engine.registerGlobalFunction(soundVolume, "soundVolume");
        engine.registerGlobalFunction(soundMixVolume, "soundMixVolume");
        engine.registerGlobalFunction(musicMixVolume, "musicMixVolume");
        engine.registerGlobalFunction(talkieMixVolume, "talkieMixVolume");
        engine.registerGlobalFunction(stopAllSounds, "stopAllSounds");
        engine.registerGlobalFunction(stopSound, "stopSound");
    }

    template <typename T>
    static bool _getArray(HSQUIRRELVM v, SQInteger index, SQInteger size, std::vector<T *> &array)
    {
        T *ptr = nullptr;
        for (size_t i = 0; i < size; i++)
        {
            if (SQ_FAILED(sq_getuserpointer(v, index + i, (SQUserPointer *)&ptr)))
            {
                return false;
            }
            array.push_back(ptr);
        }
        return true;
    }

    template <typename T>
    static bool _getArray(HSQUIRRELVM v, SQInteger index, std::vector<T *> &array)
    {
        HSQOBJECT paramObj;
        sq_resetobject(&paramObj);
        sq_getstackobj(v, index, &paramObj);
        if (!sq_isarray(paramObj))
            return false;

        T *ptr = nullptr;
        sq_push(v, 3);
        sq_pushnull(v); //null iterator
        while (SQ_SUCCEEDED(sq_next(v, -2)))
        {
            if (SQ_FAILED(sq_getuserpointer(v, -1, (SQUserPointer *)&ptr)))
                return false;
            array.emplace_back(ptr);
            sq_pop(v, 2);
        }
        sq_pop(v, 1); //pops the null iterator
        return true;
    }

    static SQInteger actorSound(HSQUIRRELVM v)
    {
        auto pEntity = ScriptEngine::getEntity<Entity>(v, 2);
        if (!pEntity)
        {
            return sq_throwerror(v, _SC("failed to get actor or object"));
        }
        SQInteger triggerNumber;
        if (SQ_FAILED(sq_getinteger(v, 3, &triggerNumber)))
        {
            return sq_throwerror(v, _SC("failed to get triggerNumber"));
        }

        auto numSounds = sq_gettop(v) - 5;
        if (numSounds == 0)
        {
            pEntity->setTrigger(triggerNumber, std::make_shared<_NoTrigger>());
            return 0;
        }

        std::vector<SoundDefinition *> sounds;
        if (numSounds > 1 || !_getArray(v, 4, sounds))
        {
            if (!_getArray(v, 4, numSounds, sounds))
            {
                return sq_throwerror(v, _SC("failed to get sounds"));
            }
        }

        pEntity->setTrigger(triggerNumber, std::make_shared<_SoundTrigger>(*g_pEngine, sounds));
        return 0;
    }

    static SQInteger loopMusic(HSQUIRRELVM v)
    {
        auto pSound = _getSoundDefinition(v, 2);
        if (!pSound)
        {
            return sq_throwerror(v, _SC("failed to get music"));
        }
        g_pEngine->getSoundManager().loopMusic(pSound);
        return 0;
    }

    static SQInteger defineSound(HSQUIRRELVM v)
    {
        const SQChar *filename;
        if (SQ_FAILED(sq_getstring(v, 2, &filename)))
        {
            return sq_throwerror(v, _SC("failed to get filename"));
        }
        auto sound = g_pEngine->getSoundManager().defineSound(filename);
        sq_pushuserpointer(v, sound.get());
        return 1;
    }

    static SQInteger loopObjectSound(HSQUIRRELVM v)
    {
        std::cerr << "TODO: loopObjectSound: not implemented" << std::endl;
        auto pSound = _getSoundDefinition(v, 2);
        if (!pSound)
        {
            return sq_throwerror(v, _SC("failed to get sound"));
        }
        SQInteger loopTimes = -1;
        sq_getinteger(v, 4, &loopTimes);
        SQFloat fadeInTime = 0;
        sq_getfloat(v, 5, &fadeInTime);
        auto pSoundId = g_pEngine->getSoundManager().playSound(pSound, true);
        if (loopTimes != -1)
        {
            // TODO: loopTimes
        }
        if (fadeInTime != 0)
        {
            pSoundId->setVolume(0.f);
            auto get = std::bind(&SoundId::getVolume, pSoundId);
            auto set = std::bind(&SoundId::setVolume, pSoundId, std::placeholders::_1);
            auto fadeTo = std::make_unique<ChangeProperty<float>>(get, set, 1.f, sf::seconds(fadeInTime));
            g_pEngine->addFunction(std::move(fadeTo));
        }
        sq_pushuserpointer(v, pSoundId.get());
        return 1;
    }

    static SQInteger loopSound(HSQUIRRELVM v)
    {
        auto pSound = _getSoundDefinition(v, 2);
        if (!pSound)
        {
            return sq_throwerror(v, _SC("failed to get sound"));
        }
        SQInteger loopTimes = -1;
        sq_getinteger(v, 3, &loopTimes);
        SQFloat fadeInTime = 0;
        sq_getfloat(v, 4, &fadeInTime);
        if (loopTimes != -1)
        {
            // TODO: loopTimes
        }
        auto pSoundId = g_pEngine->getSoundManager().playSound(pSound, true);
        if (fadeInTime != 0)
        {
            pSoundId->setVolume(0.f);
            auto get = std::bind(&SoundId::getVolume, pSoundId);
            auto set = std::bind(&SoundId::setVolume, pSoundId, std::placeholders::_1);
            auto fadeTo = std::make_unique<ChangeProperty<float>>(get, set, 1.f, sf::seconds(fadeInTime));
            g_pEngine->addFunction(std::move(fadeTo));
        }
        sq_pushuserpointer(v, pSoundId.get());
        return 1;
    }

    static void _fadeOutSound(std::shared_ptr<SoundId> pSound, const sf::Time& time)
    {
        std::cout << "fadeOutSound " << pSound->getSoundDefinition()->getPath() << " in " << time.asSeconds() << " seconds" << std::endl;
        auto get = std::bind(&SoundId::getVolume, pSound);
        auto set = std::bind(&SoundId::setVolume, pSound, std::placeholders::_1);
        auto fadeTo = std::make_unique<ChangeProperty<float>>(get, set, 0.f, time);
        g_pEngine->addFunction(std::move(fadeTo));
    }

    static SQInteger fadeOutSound(HSQUIRRELVM v)
    {
        auto pSound = _getSound(v, 2);
        float t;
        if (SQ_FAILED(sq_getfloat(v, 3, &t)))
        {
            return sq_throwerror(v, _SC("failed to get fadeOut time"));
        }
        auto time = sf::seconds(t);
        if (pSound == nullptr)
        {
            auto pSoundDefinition = _getSoundDefinition(v, 2);
            if (pSoundDefinition == nullptr)
            {
                std::cerr << "no sound to fadeOutSound" << std::endl;
                return 0;
            }
            auto size = g_pEngine->getSoundManager().getSize();
            for (size_t i = 1; i <= size; i++)
            {
                pSound = g_pEngine->getSoundManager().getSound(i);
                if (pSound && pSound->getSoundDefinition() == pSoundDefinition)
                {
                    _fadeOutSound(pSound, time);
                }
            }
            return 0;
        }
        _fadeOutSound(pSound, time);
        return 0;
    }

    static SQInteger isSoundPlaying(HSQUIRRELVM v)
    {
        std::cerr << "TODO: isSoundPlaying: not implemented" << std::endl;
        auto pSound = _getSound(v, 2);
        if (pSound)
        {
            sq_pushinteger(v, pSound->isPlaying() ? 1 : 0);
            return 1;
        }
        auto pSoundDef = _getSoundDefinition(v, 2);
        if (pSoundDef)
        {
            for (size_t i = 1; i <= g_pEngine->getSoundManager().getSize(); i++)
            {
                auto &&sound = g_pEngine->getSoundManager().getSound(i);
                if (pSoundDef == sound->getSoundDefinition())
                {
                    if (pSound->isPlaying())
                    {
                        sq_pushinteger(v, 1);
                        break;
                    }
                }
            }
        }
        sq_pushinteger(v, 0);
        return 1;
    }

    static SQInteger playObjectSound(HSQUIRRELVM v)
    {
        auto pSound = _getSoundDefinition(v, 2);
        if (!pSound)
        {
            return sq_throwerror(v, _SC("failed to get sound"));
        }
        // TODO: other params: object or actor, loopTimes, fadeInTime
        auto soundId = g_pEngine->getSoundManager().playSound(pSound);
        sq_pushuserpointer(v, soundId.get());

        return 1;
    }

    static SQInteger playMusic(HSQUIRRELVM v)
    {
        // TODO: set category to music
        auto pSound = _getSoundDefinition(v, 2);
        if (!pSound)
        {
            return sq_throwerror(v, _SC("failed to get music"));
        }
        auto soundId = g_pEngine->getSoundManager().playSound(pSound);
        sq_pushuserpointer(v, soundId.get());

        return 1;
    }

    static SQInteger playSound(HSQUIRRELVM v)
    {
        auto pSound = _getSoundDefinition(v, 2);
        if (!pSound)
        {
            return sq_throwerror(v, _SC("failed to get sound"));
        }
        auto soundId = g_pEngine->getSoundManager().playSound(pSound);
        sq_pushuserpointer(v, soundId.get());

        return 1;
    }

    static std::shared_ptr<SoundId> _getSound(HSQUIRRELVM v, SQInteger index)
    {
        SQUserPointer pSound = nullptr;
        if (SQ_FAILED(sq_getuserpointer(v, index, (SQUserPointer *)&pSound)))
        {
            SQInteger i = 0;
            if (SQ_FAILED(sq_getinteger(v, index, &i)))
            {
                return nullptr;
            }
            return g_pEngine->getSoundManager().getSound(i);
        }
        return g_pEngine->getSoundManager().getSound(pSound);
    }

    static std::shared_ptr<SoundDefinition> _getSoundDefinition(HSQUIRRELVM v, SQInteger index)
    {
        SQUserPointer pSound = nullptr;
        if (SQ_FAILED(sq_getuserpointer(v, index, (SQUserPointer *)&pSound)))
        {
            return nullptr;
        }
        return g_pEngine->getSoundManager().getSoundDefinition(pSound);
    }

    static SQInteger playSoundVolume(HSQUIRRELVM v)
    {
        auto pSound = _getSoundDefinition(v, 2);
        if (!pSound)
        {
            return sq_throwerror(v, _SC("failed to get sound"));
        }
        SQFloat volume = 0;
        if (SQ_FAILED(sq_getfloat(v, 3, &volume)))
        {
            return sq_throwerror(v, _SC("failed to get volume"));
        }
        auto soundId = g_pEngine->getSoundManager().playSound(pSound);
        soundId->setVolume(volume);
        sq_pushuserpointer(v, soundId.get());

        return 1;
    }

    static SQInteger soundMixVolume(HSQUIRRELVM v)
    {
        std::cerr << "TODO: soundMixVolume: not implemented" << std::endl;
        return 0;
    }

    static SQInteger musicMixVolume(HSQUIRRELVM v)
    {
        std::cerr << "TODO: musicMixVolume: not implemented" << std::endl;
        return 0;
    }

    static SQInteger talkieMixVolume(HSQUIRRELVM v)
    {
        std::cerr << "TODO: talkieMixVolume: not implemented" << std::endl;
        return 0;
    }

    static SQInteger soundVolume(HSQUIRRELVM v)
    {
        SQInteger channel;
        SQFloat volume = 0;
        if (SQ_FAILED(sq_getfloat(v, 3, &volume)))
        {
            return sq_throwerror(v, _SC("failed to get volume"));
        }
        auto pSound = _getSound(v, 2);
        if (pSound)
        {
            pSound->setVolume(volume);
            return 0;
        }
        auto pSoundDef = _getSoundDefinition(v, 2);
        if (pSoundDef)
        {
            g_pEngine->getSoundManager().setVolume(pSoundDef, volume);
        }
        return 0;
    }

    static SQInteger stopAllSounds(HSQUIRRELVM v)
    {
        g_pEngine->getSoundManager().stopAllSounds();
        return 0;
    }

    static SQInteger stopSound(HSQUIRRELVM v)
    {
        auto pSound = _getSound(v, 2);
        if (pSound)
        {
            g_pEngine->getSoundManager().stopSound(pSound);
            return 0;
        }
        auto pSoundDef = _getSoundDefinition(v, 2);
        if (pSoundDef)
        {
            g_pEngine->getSoundManager().stopSound(pSoundDef);
        }
        return 0;
    }
};

Engine *_SoundPack::g_pEngine = nullptr;

} // namespace ng
