static void _fadeTo(float a, const sf::Time &time)
{
    auto alpha = static_cast<sf::Uint8>(a * 255);
    auto get = std::bind(&GGEngine::getFadeAlpha, g_pEngine);
    auto set = std::bind(&GGEngine::setFadeAlpha, g_pEngine, std::placeholders::_1);
    auto fadeTo = std::make_unique<_ChangeProperty<sf::Uint8>>(get, set, alpha, time);
    g_pEngine->addFunction(std::move(fadeTo));
}

static SQInteger _roomFade(HSQUIRRELVM v)
{
    SQInteger type;
    SQFloat t;
    if (SQ_FAILED(sq_getinteger(v, 2, &type)))
    {
        return sq_throwerror(v, _SC("failed to get type"));
    }
    if (SQ_FAILED(sq_getfloat(v, 3, &t)))
    {
        return sq_throwerror(v, _SC("failed to get time"));
    }
    _fadeTo(type == 0 ? 0 : 255, sf::seconds(t));
    return 0;
}

static void _setObjectSlot(HSQUIRRELVM v, const SQChar *name, GGObject &object)
{
    sq_pushstring(v, name, -1);
    _pushObject(v, object);
    sq_newslot(v, -3, SQFalse);
}

static SQInteger _defineRoom(HSQUIRRELVM v)
{
    auto pRoom = new GGRoom(g_pEngine->getTextureManager(), g_pEngine->getSettings());
    HSQOBJECT table;
    sq_getstackobj(v, 2, &table);
    pRoom->setSquirrelObject(&table);

    // loadRoom
    sq_pushobject(v, table);
    sq_pushstring(v, _SC("background"), 10);
    if (SQ_FAILED(sq_get(v, -2)))
    {
        return sq_throwerror(v, _SC("can't find background entry"));
    }
    const SQChar *name;
    sq_getstring(v, -1, &name);
    sq_pop(v, 2);
    pRoom->load(name);

    // define instance
    sq_pushobject(v, table);
    sq_pushstring(v, _SC("instance"), -1);
    sq_pushuserpointer(v, pRoom);
    sq_newslot(v, -3, SQFalse);

    // define room objects
    for (auto &obj : pRoom->getObjects())
    {
        sq_pushobject(v, table);
        sq_pushstring(v, obj->getName().data(), obj->getName().length());
        if (SQ_FAILED(sq_get(v, -2)))
        {
            _setObjectSlot(v, obj->getName().data(), *obj);
        }
        else
        {
            HSQOBJECT object;
            sq_getstackobj(v, -1, &object);
            sq_pop(v, 2);
            sq_pushobject(v, object);
            sq_pushstring(v, _SC("instance"), -1);
            sq_pushuserpointer(v, &obj);
            sq_newslot(v, -3, SQFalse);
            sq_pop(v, 1);
        }
    }

    g_pEngine->addRoom(*pRoom);
    return 0;
}
