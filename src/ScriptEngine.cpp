#include <memory>

#include <string>
#include <memory>
#include <iostream>
#include <squirrel3/squirrel.h>
#include <squirrel3/sqstdio.h>
#include <squirrel3/sqstdaux.h>
#include <squirrel3/sqstdblob.h>
#include <squirrel3/sqstdstring.h>
#include <squirrel3/sqstdmath.h>
#include <squirrel3/sqstdsystem.h>
#include "GGEngine.h"
#include "Screen.h"
#include "ScriptEngine.h"

#ifdef SQUNICODE
#define scvprintf vfwprintf
#else
#define scvprintf vfprintf
#endif

namespace gg
{
static GGEngine *g_pEngine;

class _BreakHereFunction : public Function
{
  private:
    HSQUIRRELVM _vm;

  public:
    explicit _BreakHereFunction(HSQUIRRELVM vm)
        : _vm(vm)
    {
    }

    void operator()() override
    {
        if (sq_getvmstate(_vm) != SQ_VMSTATE_SUSPENDED)
        {
            std::cerr << "_BreakHereFunction: thread not suspended" << std::endl;
            sqstd_printcallstack(_vm);
        }
        if (SQ_FAILED(sq_wakeupvm(_vm, SQFalse, SQFalse, SQTrue, SQFalse)))
        {
            std::cerr << "_BreakHereFunction: failed to wakeup: " << _vm << std::endl;
            sqstd_printcallstack(_vm);
        }
    }
};

class _BreakTimeFunction : public TimeFunction
{
  private:
    HSQUIRRELVM _vm;

  public:
    _BreakTimeFunction(HSQUIRRELVM vm, const sf::Time &time)
        : TimeFunction(time), _vm(vm)
    {
    }

    void operator()() override
    {
        if (isElapsed())
        {
            if (sq_getvmstate(_vm) != SQ_VMSTATE_SUSPENDED)
            {
                std::cerr << "_BreakTimeFunction: thread not suspended" << std::endl;
                sqstd_printcallstack(_vm);
            }
            if (SQ_FAILED(sq_wakeupvm(_vm, SQFalse, SQFalse, SQTrue, SQFalse)))
            {
                std::cerr << "_BreakTimeFunction: failed to wakeup: " << _vm << std::endl;
                sqstd_printcallstack(_vm);
            }
        }
    }
};

template <typename Value>
class _ChangeProperty : public TimeFunction
{
  public:
    _ChangeProperty(std::function<Value()> get, std::function<void(const Value &)> set, Value destination, const sf::Time &time)
        : TimeFunction(time),
          _get(get),
          _set(set),
          _destination(destination),
          _init(get()),
          _delta(_destination - _init),
          _current(_init)
    {
    }

    void operator()() override
    {
        _set(_current);
        if (!isElapsed())
        {
            _current = _init + (_clock.getElapsedTime().asSeconds() / _time.asSeconds()) * _delta;
        }
    }

  private:
    std::function<Value()> _get;
    std::function<void(const Value &)> _set;
    Value _destination;
    Value _init;
    Value _delta;
    Value _current;
};

static void _errorHandler(HSQUIRRELVM v, const SQChar *desc, const SQChar *source, SQInteger line, SQInteger column)
{
    std::cerr << desc << source << '(' << line << ',' << column << ')' << std::endl;
}

static SQInteger _aux_printerror(HSQUIRRELVM v)
{
    auto pf = sq_geterrorfunc(v);
    if (!pf)
        return 0;

    if (sq_gettop(v) < 1)
        return 0;

    const SQChar *error = nullptr;
    if (SQ_FAILED(sq_getstring(v, 2, &error)))
    {
        error = "unknown";
    }
    pf(v, _SC("\nAn error occured in the script: %s\n"), error);
    sqstd_printcallstack(v);

    return 0;
}

void _printfunc(HSQUIRRELVM v, const SQChar *s, ...)
{
    va_list vl;
    va_start(vl, s);
    scvprintf(stdout, s, vl);
    va_end(vl);
}

void _errorfunc(HSQUIRRELVM v, const SQChar *s, ...)
{
    va_list vl;
    va_start(vl, s);
    scvprintf(stderr, s, vl);
    va_end(vl);
}

static GGObject *_getObject(HSQUIRRELVM v, int index)
{
    GGObject *obj = nullptr;
    const SQChar *name;

    sq_push(v, index);
    sq_pushnull(v); //null iterator
    while (SQ_SUCCEEDED(sq_next(v, -2)))
    {
        //here -1 is the value and -2 is the key
        sq_getstring(v, -2, &name);
        sq_getuserpointer(v, -1, (SQUserPointer *)&obj);
        sq_pop(v, 2); //pops key and val before the nex iteration
    }

    sq_pop(v, 1); //pops the null iterator
    return obj;
}

static GGActor *_getActor(HSQUIRRELVM v, int index)
{
    GGActor *actor = nullptr;
    const SQChar *name;

    sq_push(v, index);
    sq_pushnull(v); //null iterator
    while (SQ_SUCCEEDED(sq_next(v, -2)))
    {
        //here -1 is the value and -2 is the key
        sq_getstring(v, -2, &name);
        sq_getuserpointer(v, -1, (SQUserPointer *)&actor);
        sq_pop(v, 2); //pops key and val before the nex iteration
    }

    sq_pop(v, 1); //pops the null iterator
    return actor;
}

static void _pushActor(HSQUIRRELVM v, GGActor *pActor)
{
    sq_newtable(v);
    sq_pushstring(v, _SC("instance"), -1);
    sq_pushuserpointer(v, pActor);
    sq_newslot(v, -3, SQFalse);
}

static SQInteger _isObject(HSQUIRRELVM v)
{
    auto type = sq_gettype(v, 2);
    if (type != OT_TABLE)
    {
        sq_pushbool(v, SQFalse);
        return 1;
    }

    GGObject *obj = _getObject(v, 2);
    sq_pushbool(v, obj != nullptr ? SQTrue : SQFalse);
    return 1;
}

static SQInteger _scale(HSQUIRRELVM v)
{
    SQFloat s = 0;
    if (SQ_FAILED(sq_getfloat(v, 3, &s)))
    {
        return sq_throwerror(v, _SC("failed to get scale"));
    }
    GGObject *self = _getObject(v, 2);
    self->setScale(s);
    return 0;
}

static SQInteger _objectAlpha(HSQUIRRELVM v)
{
    GGObject *obj = _getObject(v, 2);
    SQFloat alpha = 0;
    if (SQ_FAILED(sq_getfloat(v, 3, &alpha)))
    {
        return sq_throwerror(v, _SC("failed to get alpha"));
    }
    alpha = alpha > 1.f ? 1.f : alpha;
    alpha = alpha < 0.f ? 0.f : alpha;
    auto a = (sf::Uint8)(alpha * 255);
    auto color = obj->getColor();
    obj->setColor(sf::Color(color.r, color.g, color.b, a));
    return 0;
}

static SQInteger _objectAlphaTo(HSQUIRRELVM v)
{
    GGObject *obj = _getObject(v, 2);
    SQFloat alpha = 0;
    if (SQ_FAILED(sq_getfloat(v, 3, &alpha)))
    {
        return sq_throwerror(v, _SC("failed to get alpha"));
    }
    alpha = alpha > 1.f ? 1.f : alpha;
    alpha = alpha < 0.f ? 0.f : alpha;
    SQFloat time = 0;
    if (SQ_FAILED(sq_getfloat(v, 4, &time)))
        time = 1.f;
    auto a = (sf::Uint8)(alpha * 255);

    auto getAlpha = [](const GGObject &o) {
        return o.getColor().a;
    };
    auto setAlpha = [](GGObject &o, sf::Uint8 a) {
        const auto &c = o.getColor();
        return o.setColor(sf::Color(c.r, c.g, c.g, a));
    };
    auto getalpha = std::bind(getAlpha, std::cref(*obj));
    auto setalpha = std::bind(setAlpha, std::ref(*obj), std::placeholders::_1);
    auto alphaTo = std::make_unique<_ChangeProperty<sf::Uint8>>(getalpha, setalpha, a, sf::seconds(time));
    g_pEngine->addFunction(std::move(alphaTo));

    return 0;
}

static SQInteger _objectHotspot(HSQUIRRELVM v)
{
    SQInteger left = 0;
    SQInteger top = 0;
    SQInteger right = 0;
    SQInteger bottom = 0;
    if (SQ_FAILED(sq_getinteger(v, 3, &left)))
    {
        return sq_throwerror(v, _SC("failed to get left"));
    }
    if (SQ_FAILED(sq_getinteger(v, 4, &top)))
    {
        return sq_throwerror(v, _SC("failed to get top"));
    }
    if (SQ_FAILED(sq_getinteger(v, 5, &right)))
    {
        return sq_throwerror(v, _SC("failed to get right"));
    }
    if (SQ_FAILED(sq_getinteger(v, 6, &bottom)))
    {
        return sq_throwerror(v, _SC("failed to get bottom"));
    }
    GGObject *obj = _getObject(v, 2);
    obj->setHotspot(sf::IntRect(static_cast<int>(left), static_cast<int>(top), static_cast<int>(right - left),
                                static_cast<int>(bottom - top)));
    return 0;
}

static SQInteger _objectOffset(HSQUIRRELVM v)
{
    SQInteger x = 0;
    SQInteger y = 0;
    GGObject *obj = _getObject(v, 2);
    if (SQ_FAILED(sq_getinteger(v, 3, &x)))
    {
        return sq_throwerror(v, _SC("failed to get x"));
    }
    if (SQ_FAILED(sq_getinteger(v, 4, &y)))
    {
        return sq_throwerror(v, _SC("failed to get y"));
    }
    obj->move(sf::Vector2f(x, y));
    return 0;
}

static SQInteger _objectState(HSQUIRRELVM v)
{
    GGObject *obj = _getObject(v, 2);
    auto numArgs = sq_gettop(v) - 2;
    if (numArgs == 1)
    {
        sq_pushinteger(v, obj->getStateAnimIndex());
        return 1;
    }

    SQInteger state;
    if (SQ_FAILED(sq_getinteger(v, 3, &state)))
    {
        return sq_throwerror(v, _SC("failed to get state"));
    }
    obj->setStateAnimIndex(state);
    std::cout << obj->getName() << "setStateAnimIndex(" << state << ")" << std::endl;

    return 0;
}

static SQInteger _objectOffsetTo(HSQUIRRELVM v)
{
    SQInteger x = 0;
    SQInteger y = 0;
    SQFloat t = 0;
    GGObject *obj = _getObject(v, 2);
    if (SQ_FAILED(sq_getinteger(v, 3, &x)))
    {
        return sq_throwerror(v, _SC("failed to get x"));
    }
    if (SQ_FAILED(sq_getinteger(v, 4, &y)))
    {
        return sq_throwerror(v, _SC("failed to get y"));
    }
    if (SQ_FAILED(sq_getfloat(v, 5, &t)))
    {
        return sq_throwerror(v, _SC("failed to get t"));
    }
    auto get = std::bind(&GGObject::getPosition, obj);
    auto set = std::bind(&GGObject::setPosition, obj, std::placeholders::_1);
    auto destination = obj->getPosition() + sf::Vector2f(x, y);
    auto offsetTo = std::make_unique<_ChangeProperty<sf::Vector2f>>(get, set, destination, sf::seconds(t));
    g_pEngine->addFunction(std::move(offsetTo));

    return 0;
}

static SQInteger _objectMoveTo(HSQUIRRELVM v)
{
    SQInteger x = 0;
    SQInteger y = 0;
    SQFloat t = 0;
    GGObject *obj = _getObject(v, 2);
    if (SQ_FAILED(sq_getinteger(v, 3, &x)))
    {
        return sq_throwerror(v, _SC("failed to get x"));
    }
    if (SQ_FAILED(sq_getinteger(v, 4, &y)))
    {
        return sq_throwerror(v, _SC("failed to get y"));
    }
    if (SQ_FAILED(sq_getfloat(v, 5, &t)))
    {
        return sq_throwerror(v, _SC("failed to get t"));
    }
    auto get = std::bind(&GGObject::getPosition, obj);
    auto set = std::bind(&GGObject::setPosition, obj, std::placeholders::_1);
    auto offsetTo = std::make_unique<_ChangeProperty<sf::Vector2f>>(get, set, sf::Vector2f(x, y), sf::seconds(t));
    g_pEngine->addFunction(std::move(offsetTo));

    return 0;
}

static SQInteger _play_state(HSQUIRRELVM v)
{
    SQInteger index;
    GGObject *obj = _getObject(v, 2);
    if (SQ_FAILED(sq_getinteger(v, 3, &index)))
    {
        return sq_throwerror(v, _SC("failed to get state"));
    }
    g_pEngine->playState(*obj, static_cast<int>(index));
    return 0;
}

static SQInteger _actorCostume(HSQUIRRELVM v)
{
    const SQChar *name;
    GGActor *actor = _getActor(v, 2);
    if (SQ_FAILED(sq_getstring(v, 3, &name)))
    {
        return sq_throwerror(v, _SC("failed to get name"));
    }
    actor->setCostume(name);
    return 0;
}

static SQInteger _actorLockFacing(HSQUIRRELVM v)
{
    SQInteger facing;
    GGActor *actor = _getActor(v, 2);
    if (SQ_FAILED(sq_getinteger(v, 3, &facing)))
    {
        return sq_throwerror(v, _SC("failed to get facing"));
    }
    actor->getCostume().lockFacing((Facing)facing);
    return 0;
}

static SQInteger _actorPlayAnimation(HSQUIRRELVM v)
{
    const SQChar *name;
    GGActor *actor = _getActor(v, 2);
    if (SQ_FAILED(sq_getstring(v, 3, &name)))
    {
        return sq_throwerror(v, _SC("failed to get name"));
    }
    actor->getCostume().setState(name);
    return 0;
}

static SQInteger _actorAt(HSQUIRRELVM v)
{
    GGActor *actor = _getActor(v, 2);
    GGObject *obj = _getObject(v, 3);
    auto pos = obj->getPosition();
    actor->setPosition(pos);
    return 0;
}

static SQInteger _sayLine(HSQUIRRELVM v)
{
    GGActor *actor = _getActor(v, 2);
    const SQChar *text;
    if (SQ_FAILED(sq_getstring(v, 3, &text)))
    {
        return sq_throwerror(v, _SC("failed to get text"));
    }
    // TOSO: actor->say(text);
    return 0;
}

static SQInteger _objectAt(HSQUIRRELVM v)
{
    SQInteger x, y;
    GGObject *obj = _getObject(v, 2);
    if (SQ_FAILED(sq_getinteger(v, 3, &x)))
    {
        return sq_throwerror(v, _SC("failed to get x"));
    }
    if (SQ_FAILED(sq_getinteger(v, 4, &y)))
    {
        return sq_throwerror(v, _SC("failed to get y"));
    }
    obj->setPosition(sf::Vector2f(x, y));
    return 0;
}

static SQInteger _objectScale(HSQUIRRELVM v)
{
    SQFloat scale;
    GGObject *obj = _getObject(v, 2);
    if (SQ_FAILED(sq_getfloat(v, 3, &scale)))
    {
        return sq_throwerror(v, _SC("failed to get scale"));
    }
    obj->setScale(scale);
    return 0;
}

static SQInteger _objectPosX(HSQUIRRELVM v)
{
    GGObject *obj = _getObject(v, 2);
    auto pos = obj->getPosition();
    sq_pushinteger(v, static_cast<SQInteger>(pos.x));
    return 1;
}

static SQInteger _objectPosY(HSQUIRRELVM v)
{
    GGObject *obj = _getObject(v, 2);
    auto pos = obj->getPosition();
    sq_pushinteger(v, static_cast<SQInteger>(pos.y));
    return 1;
}

static SQInteger _objectSort(HSQUIRRELVM v)
{
    SQInteger zOrder;
    GGObject *obj = _getObject(v, 2);
    if (SQ_FAILED(sq_getinteger(v, 3, &zOrder)))
    {
        return sq_throwerror(v, _SC("failed to get zOrder"));
    }
    obj->setZOrder(static_cast<int>(zOrder));
    return 0;
}

static SQInteger _objectRotate(HSQUIRRELVM v)
{
    SQInteger angle;
    GGObject *obj = _getObject(v, 2);
    if (SQ_FAILED(sq_getinteger(v, 3, &angle)))
    {
        return sq_throwerror(v, _SC("failed to get angle"));
    }
    obj->setRotation(static_cast<float>(angle));
    return 0;
}

static SQInteger _objectRotateTo(HSQUIRRELVM v)
{
    SQInteger dir;
    SQInteger t;
    GGObject *obj = _getObject(v, 2);
    if (SQ_FAILED(sq_getinteger(v, 3, &dir)))
    {
        return sq_throwerror(v, _SC("failed to get direction"));
    }
    if (SQ_FAILED(sq_getinteger(v, 4, &t)))
    {
        return sq_throwerror(v, _SC("failed to get time"));
    }

    auto get = std::bind(&GGObject::getRotation, obj);
    auto set = std::bind(&GGObject::setRotation, obj, std::placeholders::_1);
    auto rotateTo = std::make_unique<_ChangeProperty<float>>(get, set, dir, sf::seconds(t));
    g_pEngine->addFunction(std::move(rotateTo));
    return 0;
}

static SQInteger _objectParallaxLayer(HSQUIRRELVM v)
{
    SQInteger layer;
    GGObject *obj = _getObject(v, 2);
    if (SQ_FAILED(sq_getinteger(v, 3, &layer)))
    {
        return sq_throwerror(v, _SC("failed to get layer number"));
    }
    obj->setZOrder(static_cast<int>(layer));
    return 0;
}

static SQInteger _objectTouchable(HSQUIRRELVM v)
{
    SQBool isTouchable;
    GGObject *obj = _getObject(v, 2);
    if (SQ_FAILED(sq_getbool(v, 3, &isTouchable)))
    {
        return sq_throwerror(v, _SC("failed to get isTouchable parameter"));
    }
    obj->setTouchable(static_cast<bool>(isTouchable));
    return 0;
}

static SQInteger _objectLit(HSQUIRRELVM v)
{
    SQBool isLit;
    GGObject *obj = _getObject(v, 2);
    if (SQ_FAILED(sq_getbool(v, 3, &isLit)))
    {
        return sq_throwerror(v, _SC("failed to get isLit parameter"));
    }
    obj->setLit(static_cast<bool>(isLit));
    return 0;
}

static SQInteger _objectOwner(HSQUIRRELVM v)
{
    GGObject *obj = _getObject(v, 2);
    _pushActor(v, obj->getOwner());
    return 1;
}

static SQInteger _objectUsePos(HSQUIRRELVM v)
{
    GGObject *obj = _getObject(v, 2);
    SQInteger x, y, dir;
    if (SQ_FAILED(sq_getinteger(v, 3, &x)))
    {
        return sq_throwerror(v, _SC("failed to get x"));
    }
    if (SQ_FAILED(sq_getinteger(v, 4, &y)))
    {
        return sq_throwerror(v, _SC("failed to get y"));
    }
    if (SQ_FAILED(sq_getinteger(v, 5, &dir)))
    {
        return sq_throwerror(v, _SC("failed to get direction"));
    }
    obj->setUsePosition(sf::Vector2f(x, y));
    obj->setUseDirection(static_cast<UseDirection>(dir));
    return 0;
}

static SQInteger _objectColor(HSQUIRRELVM v)
{
    GGObject *obj = _getObject(v, 2);
    SQInteger color;
    if (SQ_FAILED(sq_getinteger(v, 3, &color)))
    {
        return sq_throwerror(v, _SC("failed to get color"));
    }
    sf::Uint8 r, g, b;
    r = (color & 0x00FF0000) >> 16;
    g = (color & 0x0000FF00) >> 8;
    b = (color & 0x000000FF);
    obj->setColor(sf::Color(r, g, b));
    return 0;
}

static SQInteger _objectIcon(HSQUIRRELVM v)
{
    GGObject *obj = _getObject(v, 2);
    const SQChar *icon;
    if (SQ_FAILED(sq_getstring(v, 3, &icon)))
    {
        return sq_throwerror(v, _SC("failed to get icon"));
    }
    // TODO: obj->setIcon(icon);
    return 0;
}

static SQInteger _objectFPS(HSQUIRRELVM v)
{
    GGObject *obj = _getObject(v, 2);
    SQInteger fps;
    if (SQ_FAILED(sq_getinteger(v, 3, &fps)))
    {
        return sq_throwerror(v, _SC("failed to get fps"));
    }
    // TODO: obj->setFps(icon);
    return 0;
}

static SQInteger _actorUsePos(HSQUIRRELVM v)
{
    GGActor *actor = _getActor(v, 2);
    GGObject *obj = _getObject(v, 3);
    auto pos = obj->getUsePosition();
    actor->setPosition(pos);
    return 0;
}

static SQInteger _actorTalkColors(HSQUIRRELVM v)
{
    auto actor = _getActor(v, 2);
    SQInteger color;
    if (SQ_FAILED(sq_getinteger(v, 3, &color)))
    {
        return sq_throwerror(v, _SC("failed to get fps"));
    }
    actor->setTalkColor(sf::Color(static_cast<sf::Uint32>(color << 8 | 0xff)));
    return 0;
}

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
    sf::View view(sf::FloatRect(0, 0, Screen::Width, Screen::Height));
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

static SQInteger _objectHidden(HSQUIRRELVM v)
{
    SQBool hidden;
    GGObject *obj = _getObject(v, 2);
    if (SQ_FAILED(sq_getbool(v, 3, &hidden)))
    {
        return sq_throwerror(v, _SC("failed to get hidden"));
    }
    obj->setVisible(!hidden);
    return 0;
}

static SQInteger _break_here(HSQUIRRELVM v)
{
    auto result = sq_suspendvm(v);
    g_pEngine->addFunction(std::make_unique<_BreakHereFunction>(v));
    return result;
}

static SQInteger _break_time(HSQUIRRELVM v)
{
    SQFloat time = 0;
    if (SQ_FAILED(sq_getfloat(v, 2, &time)))
    {
        return sq_throwerror(v, _SC("failed to get time"));
    }
    auto result = sq_suspendvm(v);
    g_pEngine->addFunction(std::make_unique<_BreakTimeFunction>(v, sf::seconds(time)));
    return result;
}

static void _push_object(HSQUIRRELVM v, GGObject &object)
{
    sq_newtable(v);
    sq_pushstring(v, _SC("instance"), -1);
    sq_pushuserpointer(v, &object);
    sq_newslot(v, -3, SQFalse);
}

static void _set_object_slot(HSQUIRRELVM v, const SQChar *name, GGObject &object)
{
    sq_pushstring(v, name, -1);
    _push_object(v, object);
    sq_newslot(v, -3, SQFalse);
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

static SQInteger _createTextObject(HSQUIRRELVM v)
{
    const SQChar *name;
    const SQChar *text;
    if (SQ_FAILED(sq_getstring(v, 2, &name)))
    {
        return sq_throwerror(v, _SC("failed to get name"));
    }
    std::string n(name);
    auto &obj = g_pEngine->getRoom().createTextObject(name, g_pEngine->getFont());

    if (SQ_FAILED(sq_getstring(v, 3, &text)))
    {
        return sq_throwerror(v, _SC("failed to get text"));
    }
    std::string s(text);
    obj.setText(s);

    _push_object(v, obj);
    return 1;
}

static SQInteger _deleteObject(HSQUIRRELVM v)
{
    auto *obj = _getObject(v, 2);
    g_pEngine->getRoom().deleteObject(*obj);
    return 0;
}

static SQInteger _createObject(HSQUIRRELVM v)
{
    auto numArgs = sq_gettop(v) - 1;
    if (numArgs == 1)
    {
        std::vector<std::string> anims;
        for (int i = 0; i < numArgs; i++)
        {
            const SQChar *animName;
            sq_getstring(v, 2 + i, &animName);
            anims.emplace_back(animName);
        }
        auto &obj = g_pEngine->getRoom().createObject(anims);
        _push_object(v, obj);
        return 1;
    }

    HSQOBJECT obj;
    const SQChar *sheet;
    sq_getstring(v, 2, &sheet);
    sq_getstackobj(v, 3, &obj);
    if (sq_isarray(obj))
    {
        const SQChar *name;
        std::vector<std::string> anims;
        sq_push(v, 3);
        sq_pushnull(v); //null iterator
        while (SQ_SUCCEEDED(sq_next(v, -2)))
        {
            sq_getstring(v, -1, &name);
            anims.emplace_back(name);
            sq_pop(v, 2);
        }
        sq_pop(v, 1); //pops the null iterator
        auto &object = g_pEngine->getRoom().createObject(sheet, anims);
        _push_object(v, object);
    }
    return 1;
}

static SQInteger _loadRoom(HSQUIRRELVM v)
{
    const SQChar *name;
    if (SQ_FAILED(sq_getstring(v, 2, &name)))
    {
        return sq_throwerror(v, _SC("failed to get room name"));
    }
    auto &room = g_pEngine->getRoom();
    room.load(name);

    sq_newtable(v);
    for (auto &obj : room.getObjects())
    {
        _set_object_slot(v, obj->getName().c_str(), *obj);
    }

    return 1;
}

static SQInteger _createActor(HSQUIRRELVM v)
{
    auto pActor = new GGActor(g_pEngine->getTextureManager());
    _pushActor(v, pActor);
    g_pEngine->addActor(*pActor);
    return 1;
}

static SQInteger _stop_thread(HSQUIRRELVM v)
{
    HSQOBJECT thread_obj;
    sq_resetobject(&thread_obj);
    if (SQ_FAILED(sq_getstackobj(v, 2, &thread_obj)))
    {
        return sq_throwerror(v, _SC("Couldn't get coroutine thread from stack"));
    }
    sq_release(thread_obj._unVal.pThread, &thread_obj);
    sq_pop(thread_obj._unVal.pThread, 1);
    sq_suspendvm(thread_obj._unVal.pThread);
    // sq_close(thread_obj._unVal.pThread);
    return 0;
}

static SQInteger _start_thread(HSQUIRRELVM v)
{
    SQInteger size = sq_gettop(v);

    // create thread and store it on the stack
    auto thread = sq_newthread(v, 1024);
    HSQOBJECT thread_obj;
    sq_resetobject(&thread_obj);
    if (SQ_FAILED(sq_getstackobj(v, -1, &thread_obj)))
    {
        return sq_throwerror(v, _SC("Couldn't get coroutine thread from stack"));
    }

    std::vector<HSQOBJECT> args;
    for (auto i = 0; i < size - 2; i++)
    {
        HSQOBJECT arg;
        sq_resetobject(&arg);
        if (SQ_FAILED(sq_getstackobj(v, 3 + i, &arg)))
        {
            return sq_throwerror(v, _SC("Couldn't get coroutine args from stack"));
        }
        args.push_back(arg);
    }

    // get the closure
    HSQOBJECT closureObj;
    sq_resetobject(&closureObj);
    if (SQ_FAILED(sq_getstackobj(v, 2, &closureObj)))
    {
        return sq_throwerror(v, _SC("Couldn't get coroutine thread from stack"));
    }

    // call the closure in the thread
    sq_pushobject(thread, closureObj);
    sq_pushroottable(thread);
    for (auto arg : args)
    {
        sq_pushobject(thread, arg);
    }
    if (SQ_FAILED(sq_call(thread, 1 + args.size(), SQFalse, SQTrue)))
    {
        sq_throwerror(v, _SC("call failed"));
        sq_pop(thread, 1); // pop the compiled closure
        return SQ_ERROR;
    }

    sq_addref(v, &thread_obj);
    sq_pushobject(v, thread_obj);

    return 1;
}

SQInteger _int_rand(SQInteger min, SQInteger max)
{
    max++;
    auto value = rand() % (max - min) + min;
    return value;
}

float float_rand(float min, float max)
{
    float scale = rand() / (float)RAND_MAX; /* [0, 1.0] */
    return min + scale * (max - min);       /* [min, max] */
}

SQInteger _random(HSQUIRRELVM v)
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

SQInteger _randomOdds(HSQUIRRELVM v)
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

SQInteger _randomFrom(HSQUIRRELVM v)
{
    auto size = sq_gettop(v);
    auto index = _int_rand(0, size - 2);
    sq_push(v, 2 + index);
    return 1;
}

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

static void _fadeTo(float a, const sf::Time &time)
{
    auto alpha = static_cast<sf::Uint8>(a * 255);
    auto get = std::bind(&GGEngine::getFadeAlpha, g_pEngine);
    auto set = std::bind(&GGEngine::setFadeAlpha, g_pEngine, std::placeholders::_1);
    auto fadeTo = std::make_unique<_ChangeProperty<sf::Uint8>>(get, set, alpha, time);
    g_pEngine->addFunction(std::move(fadeTo));
}

SQInteger _roomFade(HSQUIRRELVM v)
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

SQInteger _playSound(HSQUIRRELVM v)
{
    const SQChar *filename;
    if (SQ_FAILED(sq_getstring(v, 2, &filename)))
    {
        return sq_throwerror(v, _SC("failed to get filename"));
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

ScriptEngine::ScriptEngine(GGEngine &engine)
{
    g_pEngine = &engine;
    v = sq_open(1024);
    sq_setcompilererrorhandler(v, _errorHandler);
    sq_newclosure(v, _aux_printerror, 0);
    sq_seterrorhandler(v);
    sq_setprintfunc(v, _printfunc, _errorfunc); //sets the print function

    sq_pushroottable(v);
    sqstd_register_bloblib(v);
    sqstd_register_iolib(v);
    sqstd_register_systemlib(v);
    sqstd_register_mathlib(v);
    sqstd_register_stringlib(v);
    registerBoolConstant(_SC("NO"), false);
    registerBoolConstant(_SC("YES"), true);
    registerConstant(_SC("GONE"), 1);
    registerConstant(_SC("HERE"), 0);
    registerConstant(_SC("OFF"), 0);
    registerConstant(_SC("ON"), 1);
    registerConstant(_SC("FADE_IN"), 0);
    registerConstant(_SC("FADE_OUT"), 1);
    registerConstant(_SC("FACE_FRONT"), 0);
    registerConstant(_SC("FACE_BACK"), 1);
    registerConstant(_SC("FACE_LEFT"), 2);
    registerConstant(_SC("FACE_RIGHT"), 3);
    registerGlobalFunction(_random, "random");
    registerGlobalFunction(_randomFrom, "randomfrom");
    registerGlobalFunction(_randomOdds, "randomOdds");
    // registerGlobalFunction(_dump, "dump");
    registerGlobalFunction(_loopMusic, "loopMusic");
    registerGlobalFunction(_loopSound, "loopSound");
    registerGlobalFunction(_playSound, "playSound");
    registerGlobalFunction(_stopSound, "stopSound");
    registerGlobalFunction(_fadeOutSound, "fadeOutSound");
    registerGlobalFunction(_roomFade, "roomFade");

    registerGlobalFunction(_start_thread, "startthread");
    registerGlobalFunction(_stop_thread, "stopthread");
    registerGlobalFunction(_break_here, "breakhere");
    registerGlobalFunction(_break_time, "breaktime");
    registerGlobalFunction(_loadRoom, "loadRoom");

    registerGlobalFunction(_isObject, "isObject");
    registerGlobalFunction(_scale, "scale");
    registerGlobalFunction(_play_state, "playObjectState");
    registerGlobalFunction(_objectHidden, "objectHidden");
    registerGlobalFunction(_objectAlpha, "objectAlpha");
    registerGlobalFunction(_objectAlphaTo, "objectAlphaTo");
    registerGlobalFunction(_objectHotspot, "objectHotspot");
    registerGlobalFunction(_objectOffset, "objectOffset");
    registerGlobalFunction(_objectOffsetTo, "objectOffsetTo");
    registerGlobalFunction(_objectMoveTo, "objectMoveTo");
    registerGlobalFunction(_objectState, "objectState");
    registerGlobalFunction(_objectScale, "objectScale");
    registerGlobalFunction(_objectAt, "objectAt");
    registerGlobalFunction(_objectPosX, "_objectPosX");
    registerGlobalFunction(_objectPosY, "_objectPosY");
    registerGlobalFunction(_objectSort, "objectSort");
    registerGlobalFunction(_objectRotate, "objectRotate");
    registerGlobalFunction(_objectRotateTo, "objectRotateTo");
    registerGlobalFunction(_objectParallaxLayer, "objectParallaxLayer");
    registerGlobalFunction(_objectTouchable, "objectTouchable");
    registerGlobalFunction(_objectLit, "objectLit");
    registerGlobalFunction(_objectOwner, "objectOwner");
    registerGlobalFunction(_objectUsePos, "objectUsePos");
    registerGlobalFunction(_objectColor, "objectColor");
    registerGlobalFunction(_objectIcon, "objectIcon");
    registerGlobalFunction(_objectFPS, "objectFPS");
    // TODO: objectValidUsePos, objectBumperCycle, objectRenderOffset, objectDependentOn, objectCenter
    registerGlobalFunction(_createObject, "createObject");
    registerGlobalFunction(_createTextObject, "createTextObject");
    registerGlobalFunction(_deleteObject, "deleteObject");
    registerGlobalFunction(_translate, "translate");

    registerGlobalFunction(_createActor, "createActor");
    registerGlobalFunction(_actorCostume, "actorCostume");
    registerGlobalFunction(_actorLockFacing, "actorLockFacing");
    registerGlobalFunction(_actorLockFacing, "actorLockFacing");
    registerGlobalFunction(_actorPlayAnimation, "actorPlayAnimation");
    registerGlobalFunction(_actorAt, "actorAt");
    registerGlobalFunction(_sayLine, "sayLine");
    registerGlobalFunction(_actorUsePos, "actorUsePos");
    registerGlobalFunction(_actorTalkColors, "actorTalkColors");
    registerGlobalFunction(_cameraAt, "cameraAt");
    registerGlobalFunction(_cameraPanTo, "cameraPanTo");
}

ScriptEngine::~ScriptEngine()
{
    sq_pop(v, 1);
    sq_close(v);
}

void ScriptEngine::registerBoolConstant(const SQChar *name, bool value)
{
    sq_pushconsttable(v);
    sq_pushstring(v, name, -1);
    sq_pushbool(v, static_cast<SQBool>(value));
    sq_newslot(v, -3, SQTrue);
    sq_pop(v, 1);
}

void ScriptEngine::registerConstant(const SQChar *name, SQInteger value)
{
    sq_pushconsttable(v);
    sq_pushstring(v, name, -1);
    sq_pushinteger(v, value);
    sq_newslot(v, -3, SQTrue);
    sq_pop(v, 1);
}

void ScriptEngine::registerGlobalFunction(SQFUNCTION f, const SQChar *functionName, SQInteger nparamscheck, const SQChar *typemask)
{
    sq_pushroottable(v);
    sq_pushstring(v, functionName, -1);
    sq_newclosure(v, f, 0); //create a new function
    sq_setparamscheck(v, nparamscheck, typemask);
    sq_newslot(v, -3, SQFalse);
    sq_pop(v, 1); //pops the root table
}

void ScriptEngine::executeScript(const std::string &name)
{
    std::cout << "execute " << name << std::endl;
    if (SQ_FAILED(sqstd_dofile(v, name.c_str(), SQFalse, SQTrue)))
    {
        std::cerr << "failed to execute " << name << std::endl;
        return;
    }
}
} // namespace gg
