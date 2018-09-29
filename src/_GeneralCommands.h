static SQInteger _cameraAt(HSQUIRRELVM v)
{
    SQInteger x, y;
    if (SQ_FAILED(sq_getinteger(v, 2, &x)))
    {
        return sq_throwerror(v, _SC("failed to get x"));
    }
    if (SQ_FAILED(sq_getinteger(v, 3, &y)))
    {
        return sq_throwerror(v, _SC("failed to get y"));
    }
    g_pEngine->setCameraAt(sf::Vector2f(x - Screen::HalfWidth, y - Screen::HalfHeight));
    return 0;
}

static SQInteger _cameraPanTo(HSQUIRRELVM v)
{
    SQInteger x, y;
    SQFloat t;
    if (SQ_FAILED(sq_getinteger(v, 2, &x)))
    {
        return sq_throwerror(v, _SC("failed to get x"));
    }
    if (SQ_FAILED(sq_getinteger(v, 3, &y)))
    {
        return sq_throwerror(v, _SC("failed to get y"));
    }
    if (SQ_FAILED(sq_getfloat(v, 4, &t)))
    {
        return sq_throwerror(v, _SC("failed to get time"));
    }

    auto get = std::bind(&GGEngine::getCameraAt, g_pEngine);
    auto set = std::bind(&GGEngine::setCameraAt, g_pEngine, std::placeholders::_1);

    auto cameraPanTo = std::make_unique<_ChangeProperty<sf::Vector2f>>(get, set, sf::Vector2f(x - Screen::HalfWidth, y - Screen::HalfHeight), sf::seconds(t));
    g_pEngine->addFunction(std::move(cameraPanTo));
    return 0;
}

static SQInteger _int_rand(SQInteger min, SQInteger max)
{
    max++;
    auto value = rand() % (max - min) + min;
    return value;
}

static float float_rand(float min, float max)
{
    float scale = rand() / (float)RAND_MAX; /* [0, 1.0] */
    return min + scale * (max - min);       /* [min, max] */
}

static SQInteger _random(HSQUIRRELVM v)
{
    if (sq_gettype(v, 2) == OT_INTEGER)
    {
        SQInteger min = 0;
        SQInteger max = 0;
        sq_getinteger(v, 2, &min);
        sq_getinteger(v, 3, &max);
        auto value = _int_rand(min, max);
        sq_pushinteger(v, value);

        return 1;
    }
    {
        SQFloat min = 0;
        SQFloat max = 0;
        sq_getfloat(v, 2, &min);
        sq_getfloat(v, 3, &max);
        auto value = float_rand(min, max);
        sq_pushfloat(v, value);
        return 1;
    }
}

static SQInteger _randomOdds(HSQUIRRELVM v)
{
    SQFloat value = 0;
    if (SQ_FAILED(sq_getfloat(v, 2, &value)))
    {
        return sq_throwerror(v, _SC("failed to get value"));
    }
    auto rnd = float_rand(0, 1);
    sq_pushbool(v, static_cast<SQBool>(rnd <= value));
    return 1;
}

static SQInteger _randomFrom(HSQUIRRELVM v)
{
    auto size = sq_gettop(v);
    auto index = _int_rand(0, size - 2);
    sq_push(v, 2 + index);
    return 1;
}

static SQInteger _cameraInRoom(HSQUIRRELVM v)
{
    HSQOBJECT table;
    sq_getstackobj(v, 2, &table);

    // get room instance
    sq_pushobject(v, table);
    sq_pushstring(v, _SC("instance"), -1);
    if (SQ_FAILED(sq_get(v, -2)))
    {
        return sq_throwerror(v, _SC("can't find instance entry"));
    }
    const SQChar *name;
    GGRoom *pRoom = nullptr;
    sq_getuserpointer(v, -1, (SQUserPointer *)&pRoom);
    sq_pop(v, 2);

    // set camera in room
    g_pEngine->setRoom(pRoom);

    // call enter room function
    sq_pushobject(v, table);
    sq_pushstring(v, "enter", 5);
    if (SQ_FAILED(sq_get(v, -2)))
    {
        return sq_throwerror(v, _SC("can't find enter function"));
    }

    sq_remove(v, -2);
    sq_pushobject(v, table);
    if (SQ_FAILED(sq_call(v, 1, SQTrue, SQTrue)))
    {
        return sq_throwerror(v, _SC("function enter call failed"));
    }
    return 0;
}

static SQInteger _translate(HSQUIRRELVM v)
{
    const SQChar *idText;
    if (SQ_FAILED(sq_getstring(v, 2, &idText)))
    {
        return sq_throwerror(v, _SC("failed to get idText"));
    }
    std::string s(idText);
    s = s.substr(1);
    auto id = std::strtol(s.c_str(), nullptr, 10);
    auto text = g_pEngine->getText(id);
    sq_pushstring(v, text.c_str(), -1);
    return 1;
}

