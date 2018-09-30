class _TalkAnim : public TimeFunction
{
  public:
    _TalkAnim(GGActor &actor, std::unique_ptr<GGLip> lip)
        : TimeFunction((lip->getData().end() - 1)->time),
          _actor(actor),
          _lip(std::move(lip)),
          _index(0)
    {
    }

    void operator()() override
    {
        if (isElapsed())
        {
            _actor.say("");
            return;
        }
        auto time = _lip->getData()[_index + 1].time;
        if (_clock.getElapsedTime() > time)
        {
            _index = _index + 1;
        }
        auto index = _lip->getData()[_index].letter - 'A';
        // TODO: what is the correspondance between letter and head index ?
        _actor.getCostume().setHeadIndex(index % 6);
    }

  private:
    GGActor &_actor;
    std::unique_ptr<GGLip> _lip;
    int _index;
};

static SQInteger _actorAlpha(HSQUIRRELVM v)
{
    auto actor = _getActor(v, 2);
    if (!actor)
    {
        return sq_throwerror(v, _SC("failed to get actor"));
    }
    SQFloat transparency;
    if (SQ_FAILED(sq_getfloat(v, 3, &transparency)))
    {
        return sq_throwerror(v, _SC("failed to get transparency"));
    }
    auto alpha = static_cast<sf::Uint8>(transparency * 255);
    actor->setColor(sf::Color(static_cast<sf::Uint32>(actor->getColor().toInteger() << 8 | alpha)));
    return 0;
}

static SQInteger _actorAnimationNames(HSQUIRRELVM v)
{
    const SQChar *head = nullptr;
    const SQChar *stand = nullptr;
    const SQChar *walk = nullptr;
    const SQChar *reach = nullptr;
    auto *pActor = _getActor(v, 2);
    if (!pActor)
    {
        return sq_throwerror(v, _SC("failed to get actor"));
    }
    sq_getstring(v, 3, &head);
    sq_getstring(v, 4, &stand);
    sq_getstring(v, 5, &walk);
    sq_getstring(v, 6, &reach);
    pActor->getCostume().setAnimationNames(head ? head : "", stand ? stand : "", walk ? walk : "", reach ? reach : "");
    return 0;
}

static SQInteger _actorAt(HSQUIRRELVM v)
{
    auto *pActor = _getActor(v, 2);
    if (!pActor)
    {
        return sq_throwerror(v, _SC("failed to get actor"));
    }
    auto *pObj = _getObject(v, 3);
    if (!pObj)
    {
        return sq_throwerror(v, _SC("failed to get object"));
    }
    auto pos = pObj->getPosition();
    pActor->setPosition(pos);
    return 0;
}

static SQInteger _actorColor(HSQUIRRELVM v)
{
    auto actor = _getActor(v, 2);
    if (!actor)
    {
        return sq_throwerror(v, _SC("failed to get actor"));
    }
    SQInteger color;
    if (SQ_FAILED(sq_getinteger(v, 3, &color)))
    {
        return sq_throwerror(v, _SC("failed to get fps"));
    }
    auto alpha = actor->getColor().toInteger() & 0x000000FF;
    actor->setColor(sf::Color(static_cast<sf::Uint32>(color << 8 | alpha)));
    return 0;
}

static SQInteger _actorCostume(HSQUIRRELVM v)
{
    const SQChar *name;
    auto actor = _getActor(v, 2);
    if (!actor)
    {
        return sq_throwerror(v, _SC("failed to get actor"));
    }
    if (SQ_FAILED(sq_getstring(v, 3, &name)))
    {
        return sq_throwerror(v, _SC("failed to get name"));
    }
    const SQChar *pSheet = nullptr;
    sq_getstring(v, 4, &pSheet);
    actor->setCostume(name, pSheet ? pSheet : "");
    return 0;
}

static SQInteger _actorDistanceTo(HSQUIRRELVM v)
{
    auto actor = _getActor(v, 2);
    if (!actor)
    {
        return sq_throwerror(v, _SC("failed to get actor"));
    }
    auto object = _getObject(v, 3);
    if (!object)
    {
        return sq_throwerror(v, _SC("failed to get object"));
    }
    auto pos = actor->getPosition() - object->getPosition();
    auto dist = sqrt(pos.x * pos.x + pos.y * pos.y);
    sq_pushinteger(v, dist);
    return 1;
}

static SQInteger _actorDistanceWithin(HSQUIRRELVM v)
{
    auto actor = _getActor(v, 2);
    if (!actor)
    {
        return sq_throwerror(v, _SC("failed to get actor"));
    }
    auto object = _getObject(v, 3);
    if (!object)
    {
        return sq_throwerror(v, _SC("failed to get object"));
    }
    SQInteger distance;
    if (SQ_FAILED(sq_getinteger(v, 4, &distance)))
    {
        return sq_throwerror(v, _SC("failed to get distance"));
    }
    auto pos = actor->getPosition() - object->getPosition();
    auto dist = sqrt(pos.x * pos.x + pos.y * pos.y);
    sq_pushbool(v, dist < distance);
    return 1;
}

static SQInteger _actorFace(HSQUIRRELVM v)
{
    auto actor = _getActor(v, 2);
    if (!actor)
    {
        return sq_throwerror(v, _SC("failed to get actor"));
    }
    SQInteger dir;
    if (SQ_FAILED(sq_getinteger(v, 3, &dir)))
    {
        return sq_throwerror(v, _SC("failed to get direction"));
    }
    actor->getCostume().setFacing((Facing)dir);
    return 0;
}

static SQInteger _actorHidden(HSQUIRRELVM v)
{
    auto *pActor = _getActor(v, 2);
    if (!pActor)
    {
        return sq_throwerror(v, _SC("failed to get actor"));
    }
    pActor->setVisible(false);
    return 0;
}

static SQInteger _actorShowHideLayer(HSQUIRRELVM v, bool isVisible)
{
    auto *pActor = _getActor(v, 2);
    if (!pActor)
    {
        return sq_throwerror(v, _SC("failed to get actor"));
    }
    const SQChar *layerName;
    if (SQ_FAILED(sq_getstring(v, 3, &layerName)))
    {
        return sq_throwerror(v, _SC("failed to get layerName"));
    }
    pActor->getCostume().setLayerVisible(layerName, isVisible);
    return 0;
}

static SQInteger _actorHideLayer(HSQUIRRELVM v)
{
    return _actorShowHideLayer(v, false);
}

static SQInteger _actorInTrigger(HSQUIRRELVM v)
{
    auto *actor = _getActor(v, 2);
    if (!actor)
    {
        return sq_throwerror(v, _SC("failed to get actor"));
    }
    auto *object = _getObject(v, 3);
    if (!object)
    {
        return sq_throwerror(v, _SC("failed to get object"));
    }
    bool isInside = object->getRealHotspot().contains((sf::Vector2i)(actor->getPosition()));
    sq_pushbool(v, isInside);
    return 1;
}

static SQInteger _actorInWalkbox(HSQUIRRELVM v)
{
    auto *actor = _getActor(v, 2);
    if (!actor)
    {
        return sq_throwerror(v, _SC("failed to get actor"));
    }
    const auto &walkboxes = g_pEngine->getRoom().getWalkboxes();
    for (const auto &walkbox : walkboxes)
    {
        if (walkbox.contains(actor->getPosition()))
        {
            sq_pushbool(v, SQTrue);
            return 1;
        }
    }
    sq_pushbool(v, SQFalse);
    return 1;
}

static SQInteger _actorLockFacing(HSQUIRRELVM v)
{
    SQInteger facing;
    GGActor *actor = _getActor(v, 2);
    if (!actor)
    {
        return sq_throwerror(v, _SC("failed to get actor"));
    }
    if (SQ_FAILED(sq_getinteger(v, 3, &facing)))
    {
        return sq_throwerror(v, _SC("failed to get facing"));
    }
    actor->getCostume().lockFacing((Facing)facing);
    return 0;
}

static SQInteger _actorPlayAnimation(HSQUIRRELVM v)
{
    const SQChar *animation = nullptr;
    GGActor *pActor = _getActor(v, 2);
    if (!pActor)
    {
        return sq_throwerror(v, _SC("failed to get actor"));
    }
    if (SQ_FAILED(sq_getstring(v, 3, &animation)))
    {
        return sq_throwerror(v, _SC("failed to get animation"));
    }
    SQBool loop = false;
    sq_getbool(v, 4, &loop);
    std::cout << "Play anim " << animation << (loop ? " (loop)" : "") << std::endl;
    pActor->getCostume().setState(animation);
    pActor->getCostume().getAnimation()->play(loop);
    return 0;
}

static SQInteger _actorPosX(HSQUIRRELVM v)
{
    auto *pActor = _getActor(v, 2);
    if (!pActor)
    {
        return sq_throwerror(v, _SC("failed to get actor"));
    }
    sq_pushinteger(v, pActor->getPosition().x);
    return 1;
}

static SQInteger _actorPosY(HSQUIRRELVM v)
{
    auto *pActor = _getActor(v, 2);
    if (!pActor)
    {
        return sq_throwerror(v, _SC("failed to get actor"));
    }
    sq_pushinteger(v, pActor->getPosition().y);
    return 1;
}

static SQInteger _actorRenderOffset(HSQUIRRELVM v)
{
    auto *pActor = _getActor(v, 2);
    if (!pActor)
    {
        return sq_throwerror(v, _SC("failed to get actor"));
    }
    SQInteger x, y;
    if (SQ_FAILED(sq_getinteger(v, 3, &x)))
    {
        return sq_throwerror(v, _SC("failed to get x"));
    }
    if (SQ_FAILED(sq_getinteger(v, 4, &y)))
    {
        return sq_throwerror(v, _SC("failed to get y"));
    }
    pActor->setRenderOffset(sf::Vector2i(x, y));
    return 0;
}

// TODO: static SQInteger _actorRoom(HSQUIRRELVM v)

static SQInteger _actorShowLayer(HSQUIRRELVM v)
{
    return _actorShowHideLayer(v, true);
}

// TODO: static SQInteger _actorSlotSelectable(HSQUIRRELVM v)
static SQInteger _actorTalking(HSQUIRRELVM v)
{
    // TODO: with no actor specified
    auto actor = _getActor(v, 2);
    if (!actor)
    {
        return sq_throwerror(v, _SC("failed to get actor"));
    }
    sq_pushbool(v, actor->isTalking());
    return 1;
}
// TODO: static SQInteger _actorStopWalking(HSQUIRRELVM v)

static SQInteger _actorTalkColors(HSQUIRRELVM v)
{
    auto actor = _getActor(v, 2);
    if (!actor)
    {
        return sq_throwerror(v, _SC("failed to get actor"));
    }
    SQInteger color;
    if (SQ_FAILED(sq_getinteger(v, 3, &color)))
    {
        return sq_throwerror(v, _SC("failed to get fps"));
    }
    actor->setTalkColor(sf::Color(static_cast<sf::Uint32>(color << 8 | 0xff)));
    return 0;
}

static SQInteger _actorTalkOffset(HSQUIRRELVM v)
{
    GGActor *actor = _getActor(v, 2);
    if (!actor)
    {
        return sq_throwerror(v, _SC("failed to get actor"));
    }
    SQInteger x;
    if (SQ_FAILED(sq_getinteger(v, 3, &x)))
    {
        return sq_throwerror(v, _SC("failed to get x"));
    }
    SQInteger y;
    if (SQ_FAILED(sq_getinteger(v, 4, &x)))
    {
        return sq_throwerror(v, _SC("failed to get y"));
    }
    actor->setTalkOffset(sf::Vector2i(x, y));
    return 0;
}
// TODO: static SQInteger _actorTurnTo(HSQUIRRELVM v)

static SQInteger _actorUsePos(HSQUIRRELVM v)
{
    GGActor *actor = _getActor(v, 2);
    if (!actor)
    {
        return sq_throwerror(v, _SC("failed to get actor"));
    }
    GGObject *obj = _getObject(v, 3);
    if (!obj)
    {
        return sq_throwerror(v, _SC("failed to get object"));
    }
    auto pos = obj->getUsePosition();
    actor->setPosition(pos);
    return 0;
}

static SQInteger _actorUseWalkboxes(HSQUIRRELVM v)
{
    GGActor *actor = _getActor(v, 2);
    if (!actor)
    {
        return sq_throwerror(v, _SC("failed to get actor"));
    }
    SQBool use;
    if (SQ_FAILED(sq_getbool(v, 3, &use)))
    {
        return sq_throwerror(v, _SC("failed to get useWalkboxes"));
    }
    actor->useWalkboxes(use);
    return 0;
}

static void _actorWalkTo(GGActor *pActor, sf::Vector2f destination)
{
    auto get = std::bind(&GGActor::getPosition, pActor);
    auto set = std::bind(&GGActor::setPosition, pActor, std::placeholders::_1);

    // yes I known this is not enough, I need to take into account the walkbox
    auto offsetTo = std::make_unique<_ChangeProperty<sf::Vector2f>>(get, set, destination, sf::seconds(4));
    std::cout << "Play anim walk (loop)" << std::endl;
    pActor->getCostume().setState("walk");
    pActor->getCostume().getAnimation()->play(true);
    offsetTo->callWhenElapsed([pActor] { 
        std::cout << "Play anim stand" << std::endl;
        pActor->getCostume().setState("stand"); });
    g_pEngine->addFunction(std::move(offsetTo));
}

static SQInteger _actorWalkForward(HSQUIRRELVM v)
{
    GGActor *actor = _getActor(v, 2);
    if (!actor)
    {
        return sq_throwerror(v, _SC("failed to get actor"));
    }
    SQInteger distance;
    if (SQ_FAILED(sq_getinteger(v, 3, &distance)))
    {
        return sq_throwerror(v, _SC("failed to get distance"));
    }
    _actorWalkTo(actor, actor->getPosition() + sf::Vector2f(distance, 0));
    return 0;
}
// TODO: static SQInteger actorWalking(HSQUIRRELVM v)
// TODO: static SQInteger actorWalkSpeed(HSQUIRRELVM v)

static SQInteger _actorWalkTo(HSQUIRRELVM v)
{
    auto *pActor = _getActor(v, 2);
    if (!pActor)
    {
        return sq_throwerror(v, _SC("failed to get actor"));
    }
    auto *pObject = _getObject(v, 3);
    if (!pObject)
    {
        return sq_throwerror(v, _SC("failed to get object"));
    }
    _actorWalkTo(pActor, pObject->getPosition());

    return 0;
}

// TODO: static SQInteger addSelectableActor(HSQUIRRELVM v)

static SQInteger _createActor(HSQUIRRELVM v)
{
    HSQOBJECT table;
    sq_resetobject(&table);
    sq_getstackobj(v, 2, &table);

    sq_pushobject(v, table);
    sq_pushstring(v, _SC("_key"), 4);
    if (SQ_FAILED(sq_get(v, -2)))
    {
        return sq_throwerror(v, _SC("can't find _key entry"));
    }
    const SQChar *key;
    if (SQ_FAILED(sq_getstring(v, -1, &key)))
    {
        return sq_throwerror(v, _SC("can't find _key entry"));
    }
    sq_pop(v, 2);

    // define instance
    auto pActor = std::make_unique<GGActor>(g_pEngine->getTextureManager());
    pActor->setName(key);
    pActor->setTable(table);
    sq_pushobject(v, table);
    sq_pushstring(v, _SC("instance"), -1);
    sq_pushuserpointer(v, pActor.get());
    sq_newslot(v, -3, SQFalse);

    g_pEngine->addActor(std::move(pActor));
    return 1;
}

static SQInteger _currentActor(HSQUIRRELVM v)
{
    auto actor = g_pEngine->getCurrentActor();
    if (!actor)
    {
        sq_pushnull(v);
        return 1;
    }
    _pushObject(v, *actor);
    return 1;
}

static SQInteger _isActor(HSQUIRRELVM v)
{
    auto actor = _getEntity<GGActor>(v, 2);
    sq_pushbool(v, actor ? SQTrue : SQFalse);
    return 1;
}

// TODO: static SQInteger isActorOnScreen(HSQUIRRELVM v)
static SQInteger _masterActorArray(HSQUIRRELVM v)
{
    auto &actors = g_pEngine->getActors();
    sq_newarray(v, 0);
    for (auto &actor : actors)
    {
        sq_pushobject(v, actor->getTable());
        sq_arrayappend(v, -2);
    }
    return 1;
}
// TODO: static SQInteger mumbleLine(HSQUIRRELVM v)

static std::string str_toupper(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::toupper(c); } // correct
    );
    return s;
}

static SQInteger _sayLine(HSQUIRRELVM v)
{
    GGActor *actor = _getActor(v, 2);
    if (!actor)
    {
        return sq_throwerror(v, _SC("failed to get actor"));
    }
    auto numArgs = sq_gettop(v) - 1;
    std::vector<int> ids;

    // TODO: say the other lines
    const SQChar *idText;
    if (SQ_FAILED(sq_getstring(v, 3, &idText)))
    {
        return sq_throwerror(v, _SC("failed to get text"));
    }

    std::string s(idText);
    s = s.substr(1);
    auto id = std::strtol(s.c_str(), nullptr, 10);
    std::cout << "Play anim talk (loop)" << std::endl;

    std::string path;
    std::string name = str_toupper(actor->getName()).append("_").append(s);
    path.append(g_pEngine->getSettings().getGamePath()).append(name).append(".lip");
    auto lip = std::make_unique<GGLip>();
    std::cout << "load lip " << path << std::endl;
    lip->load(path);

    g_pEngine->playSound(name + ".ogg", false);

    g_pEngine->addFunction(std::make_unique<_TalkAnim>(*actor, std::move(lip)));
    auto text = g_pEngine->getText(id);
    actor->say(text);
    return 0;
}

static SQInteger _selectActor(HSQUIRRELVM v)
{
    auto *actor = _getActor(v, 2);
    if (!actor)
    {
        return sq_throwerror(v, _SC("failed to get actor"));
    }
    g_pEngine->setCurrentActor(actor);
    return 0;
}

// TODO: static SQInteger stopTalking(HSQUIRRELVM v)

static SQInteger _triggerActors(HSQUIRRELVM v)
{
    auto *object = _getObject(v, 2);
    if (!object)
    {
        return sq_throwerror(v, _SC("failed to get object"));
    }
    sq_newarray(v, 0);
    for (const auto &actor : g_pEngine->getActors())
    {
        if (object->getRealHotspot().contains((sf::Vector2i)actor->getPosition()))
        {
            sq_pushobject(v, actor->getTable());
            sq_arrayappend(v, -2);
        }
    }
    return 1;
}

// TODO: static SQInteger verbUIColors(HSQUIRRELVM v)
