#pragma once
#include <squirrel3/squirrel.h>
#include "GGEngine.h"

namespace gg
{
class _SoundPack : public Pack
{
  private:
    static GGEngine *g_pEngine;

  private:
    void addTo(ScriptEngine &engine) const override
    {
        g_pEngine = &engine.getEngine();
        engine.registerGlobalFunction(loopSound, "loopSound");
        engine.registerGlobalFunction(loopMusic, "loopMusic");
        engine.registerGlobalFunction(defineSound, "defineSound");
        engine.registerGlobalFunction(playSound, "playSound");
        engine.registerGlobalFunction(stopSound, "stopSound");
        engine.registerGlobalFunction(fadeOutSound, "fadeOutSound");
    }

    static SQInteger loopMusic(HSQUIRRELVM v)
    {
        const SQChar *filename;
        if (SQ_FAILED(sq_getstring(v, 2, &filename)))
        {
            return sq_throwerror(v, _SC("failed to get filename"));
        }
        g_pEngine->loopMusic(filename);
        return 0;
    }

    static SQInteger defineSound(HSQUIRRELVM v)
    {
        const SQChar *filename;
        if (SQ_FAILED(sq_getstring(v, 2, &filename)))
        {
            return sq_throwerror(v, _SC("failed to get filename"));
        }
        auto sound = g_pEngine->defineSound(filename);
        sq_pushuserpointer(v, sound.get());
        return 1;
    }

    static SQInteger loopSound(HSQUIRRELVM v)
    {
        const SQChar *filename;
        if (SQ_FAILED(sq_getstring(v, 2, &filename)))
        {
            return sq_throwerror(v, _SC("failed to get filename"));
        }
        auto ptr = g_pEngine->playSound(filename, true);
        sq_pushuserpointer(v, ptr.get());
        return 1;
    }

    static SQInteger fadeOutSound(HSQUIRRELVM v)
    {
        SQUserPointer ptr;
        if (SQ_FAILED(sq_getuserpointer(v, 2, &ptr)))
        {
            return sq_throwerror(v, _SC("failed to get sound"));
        }
        auto pSound = static_cast<SoundId *>(ptr);
        if (pSound == nullptr)
            return 0;
        float t;
        if (SQ_FAILED(sq_getfloat(v, 3, &t)))
        {
            return sq_throwerror(v, _SC("failed to get fadeOut time"));
        }

        auto get = std::bind(&sf::Sound::getVolume, &pSound->sound);
        auto set = std::bind(&sf::Sound::setVolume, &pSound->sound, std::placeholders::_1);
        auto fadeTo = std::make_unique<ChangeProperty<float>>(get, set, 0.f, sf::seconds(t));
        return 0;
    }

    static SQInteger playSound(HSQUIRRELVM v)
    {
        const SQChar *filename;
        if (SQ_FAILED(sq_getstring(v, 2, &filename)))
        {
            SoundId *pSound;
            if (SQ_FAILED(sq_getuserpointer(v, 2, (SQUserPointer *)&pSound)))
            {
                return sq_throwerror(v, _SC("failed to get sound"));
            }
            pSound->sound.play();
            return 0;
        }
        SQUserPointer ptr = g_pEngine->playSound(filename, false).get();
        sq_pushuserpointer(v, ptr);
        return 0;
    }

    static SQInteger stopSound(HSQUIRRELVM v)
    {
        SoundId *soundId;
        if (SQ_FAILED(sq_getuserpointer(v, 2, (SQUserPointer *)&soundId)))
        {
            return sq_throwerror(v, _SC("failed to get sound"));
        }
        g_pEngine->stopSound(*soundId);
        return 0;
    }
};

GGEngine *_SoundPack::g_pEngine = nullptr;

} // namespace gg
