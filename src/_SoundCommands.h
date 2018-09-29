SQInteger _loopMusic(HSQUIRRELVM v)
{
    const SQChar *filename;
    if (SQ_FAILED(sq_getstring(v, 2, &filename)))
    {
        return sq_throwerror(v, _SC("failed to get filename"));
    }
    g_pEngine->loopMusic(filename);
    return 0;
}

SQInteger _defineSound(HSQUIRRELVM v)
{
    const SQChar *filename;
    if (SQ_FAILED(sq_getstring(v, 2, &filename)))
    {
        return sq_throwerror(v, _SC("failed to get filename"));
    }
    auto sound = g_pEngine->defineSound(filename);
    sq_pushuserpointer(v, sound);
    return 1;
}

SQInteger _loopSound(HSQUIRRELVM v)
{
    const SQChar *filename;
    if (SQ_FAILED(sq_getstring(v, 2, &filename)))
    {
        return sq_throwerror(v, _SC("failed to get filename"));
    }
    auto ptr = g_pEngine->playSound(filename, true);
    sq_pushuserpointer(v, ptr);
    return 1;
}

SQInteger _fadeOutSound(HSQUIRRELVM v)
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
    auto fadeTo = std::make_unique<_ChangeProperty<float>>(get, set, 0.f, sf::seconds(t));
    return 0;
}

SQInteger _playSound(HSQUIRRELVM v)
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
    SQUserPointer ptr = g_pEngine->playSound(filename, false);
    sq_pushuserpointer(v, ptr);
    return 0;
}

SQInteger _stopSound(HSQUIRRELVM v)
{
    SoundId *soundId;
    if (SQ_FAILED(sq_getuserpointer(v, 2, (SQUserPointer *)&soundId)))
    {
        return sq_throwerror(v, _SC("failed to get sound"));
    }
    g_pEngine->stopSound(*soundId);
    return 0;
}
