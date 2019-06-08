#include <regex>
#include "Actor.h"
#include "Engine.h"
#include "InventoryObject.h"
#include "Lip.h"
#include "PathFinder.h"
#include "Room.h"
#include "RoomScaling.h"
#include "SoundDefinition.h"
#include "SoundManager.h"
#include "Text.h"

namespace ng
{
struct Actor::Impl
{
    class WalkingState
    {
    public:
        WalkingState();

        void setActor(Actor *pActor);
        void setDestination(const std::vector<sf::Vector2i> &path, std::optional<Facing> facing);
        void update(const sf::Time &elapsed);
        void stop();
        bool isWalking() const { return _isWalking; }

    private:
        Facing getFacing();

    private:
        Actor *_pActor;
        std::vector<sf::Vector2i> _path;
        std::optional<Facing> _facing;
        bool _isWalking;
    };

    class TalkingState : public sf::Drawable
    {
    public:
        TalkingState();

        void setActor(Actor *pActor);
        void setTalkOffset(const sf::Vector2i &offset) { _talkOffset = offset; }
        void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
        void update(const sf::Time &elapsed);
        void say(int id);
        void stop();
        bool isTalking() const { return _isTalking; }
        bool isTalkingIdDone(int id) const { return _id != id && std::find(_ids.begin(), _ids.end(), id) == _ids.end(); }
        void setTalkColor(sf::Color color) { _talkColor = color; }

    private:
        void load(int id);

    private:
        Actor *_pActor;
        FntFont _font;
        bool _isTalking;
        std::wstring _sayText;
        Lip _lip;
        int _index;
        sf::Vector2i _talkOffset{0, 90};
        sf::Color _talkColor;
        sf::Clock _clock;
        std::vector<int> _ids;
        int _id{0};
        SoundId *_pSound{nullptr};
    };

    explicit Impl(Engine &engine)
        : _engine(engine),
          _settings(engine.getSettings()),
          _costume(engine.getTextureManager()),
          _color(sf::Color::White),
          _zorder(0),
          _use(true),
          _pRoom(nullptr),
          _speed(30, 15),
          _volume(1.f)
    {
    }

    Engine &_engine;
    const EngineSettings &_settings;
    Costume _costume;
    std::string _name, _icon;
    sf::Color _color;
    sf::Vector2i _renderOffset;
    int _zorder;
    bool _use;
    Room *_pRoom;
    sf::IntRect _hotspot;
    std::vector<std::unique_ptr<InventoryObject>> _objects;
    WalkingState _walkingState;
    TalkingState _talkingState;
    sf::Vector2i _speed;
    float _volume;
    std::shared_ptr<Path> _path;
    HSQOBJECT _table;
};

void Actor::setName(const std::string &name)
{
    pImpl->_name = name;
}

const std::string &Actor::getName() const
{
    return pImpl->_name;
}

void Actor::setIcon(const std::string &icon)
{
    pImpl->_icon = icon;
}

const std::string &Actor::getIcon() const
{
    return pImpl->_icon;
}

void Actor::useWalkboxes(bool use)
{
    pImpl->_use = use;
}

Costume &Actor::getCostume()
{
    return pImpl->_costume;
}

Costume &Actor::getCostume() const
{
    return pImpl->_costume;
}

void Actor::setTalkColor(sf::Color color)
{
    pImpl->_talkingState.setTalkColor(color);
}

void Actor::setTalkOffset(const sf::Vector2i &offset)
{
    pImpl->_talkingState.setTalkOffset(offset);
}

void Actor::say(int id)
{
    pImpl->_talkingState.say(id);
}

void Actor::stopTalking()
{
    pImpl->_talkingState.stop();
}

bool Actor::isTalking() const
{
    return pImpl->_talkingState.isTalking();
}

bool Actor::isTalkingIdDone(int id) const
{
    return pImpl->_talkingState.isTalkingIdDone(id);
}

void Actor::setColor(sf::Color color)
{
    pImpl->_color = color;
}

sf::Color Actor::getColor()
{
    return pImpl->_color;
}

void Actor::setRenderOffset(const sf::Vector2i &offset)
{
    pImpl->_renderOffset = offset;
}

Room *Actor::getRoom()
{
    return pImpl->_pRoom;
}

void Actor::setHotspot(const sf::IntRect &hotspot)
{
    pImpl->_hotspot = hotspot;
}

bool Actor::contains(const sf::Vector2f &pos) const
{
    auto pAnim = pImpl->_costume.getAnimation();
    if (!pAnim)
        return false;

    auto size = pImpl->_pRoom->getRoomSize();
    auto scale = pImpl->_pRoom->getRoomScaling().getScaling(size.y - getPosition().y);
    auto transform = _transform;
    transform.scale(scale, scale);
    transform.move((sf::Vector2f)-pImpl->_renderOffset * scale);
    auto t = transform.getInverseTransform();
    auto pos2 = t.transformPoint(pos);
    return pAnim->contains(pos2);
}

void Actor::pickupObject(std::unique_ptr<InventoryObject> pObject)
{
    pImpl->_objects.push_back(std::move(pObject));
}

const std::vector<std::unique_ptr<InventoryObject>> &Actor::getObjects() const
{
    return pImpl->_objects;
}

void Actor::setWalkSpeed(const sf::Vector2i &speed)
{
    pImpl->_speed = speed;
}

const sf::Vector2i &Actor::getWalkSpeed() const
{
    return pImpl->_speed;
}

void Actor::stopWalking()
{
    pImpl->_walkingState.stop();
}

bool Actor::isWalking() const
{
    return pImpl->_walkingState.isWalking();
}

void Actor::setVolume(float volume)
{
    pImpl->_volume = volume;
}

HSQOBJECT &Actor::getTable()
{
    return pImpl->_table;
}

Actor::Impl::WalkingState::WalkingState()
    : _pActor(nullptr), _facing(Facing::FACE_FRONT), _isWalking(false)
{
}

void Actor::Impl::WalkingState::setActor(Actor *pActor)
{
    _pActor = pActor;
}

void Actor::Impl::WalkingState::setDestination(const std::vector<sf::Vector2i> &path, std::optional<Facing> facing)
{
    _path = path;
    _facing = facing;
    _path.erase(_path.begin());
    _pActor->getCostume().setFacing(getFacing());
    _pActor->getCostume().setState("walk");
    _pActor->getCostume().getAnimation()->play(true);
    _isWalking = true;
}

void Actor::Impl::WalkingState::stop()
{
    _isWalking = false;
}

Facing Actor::Impl::WalkingState::getFacing()
{
    auto pos = _pActor->getPosition();
    auto dx = _path[0].x - pos.x;
    auto dy = _path[0].y - pos.y;
    if (fabs(dx) > fabs(dy))
        return (dx > 0) ? Facing::FACE_RIGHT : Facing::FACE_LEFT;
    return (dy > 0) ? Facing::FACE_FRONT : Facing::FACE_BACK;
}

void Actor::Impl::WalkingState::update(const sf::Time &elapsed)
{
    if (!_isWalking)
        return;

    auto pos = _pActor->getPosition();
    auto delta = (_path[0] - (sf::Vector2i)pos);
    auto speed = _pActor->getWalkSpeed();
    auto offset = sf::Vector2f(speed) * elapsed.asSeconds();
    if (delta.x > 0)
    {
        if (offset.x > delta.x)
            offset.x = delta.x;
    }
    else
    {
        offset.x = -offset.x;
        if (offset.x < delta.x)
            offset.x = delta.x;
    }
    if (delta.y < 0)
    {
        offset.y = -offset.y;
        if (offset.y < delta.y)
            offset.y = delta.y;
    }
    else
    {
        if (offset.y > delta.y)
            offset.y = delta.y;
    }
    _pActor->setPosition(pos + offset);
    if (fabs(_path[0].x - pos.x) <= 1 && fabs(_path[0].y - pos.y) <= 1)
    {
        _path.erase(_path.begin());
        if (_path.empty())
        {
            _isWalking = false;
            std::cout << "Play anim stand" << std::endl;
            if (_facing.has_value())
            {
                _pActor->getCostume().setFacing(_facing.value());
            }
            _pActor->getCostume().setState("stand");
        }
        else
        {
            _pActor->getCostume().setFacing(getFacing());
            _pActor->getCostume().setState("walk");
            _pActor->getCostume().getAnimation()->play(true);
            std::cout << "go to : " << _path[0].x << "," << _path[0].y << std::endl;
        }
    };
}

Actor::Impl::TalkingState::TalkingState()
    : _pActor(nullptr), _isTalking(false),
      _index(0), _talkColor(sf::Color::White)
{
}

void Actor::Impl::TalkingState::setActor(Actor *pActor)
{
    _pActor = pActor;
    if (!_pActor)
        return;

    _font.setSettings(&_pActor->pImpl->_engine.getSettings());
    _font.loadFromFile("SayLineFont.fnt");
}

static std::string str_toupper(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return ::toupper(c); } // correct
    );
    return s;
}

void Actor::Impl::TalkingState::say(int id)
{
    if (_isTalking)
    {
        _ids.push_back(id);
        return;
    }

    load(id);
}

void Actor::Impl::TalkingState::stop()
{
    _ids.clear();
    if (_pSound)
    {
        _pSound = _pActor->pImpl->_engine.getSoundManager().getSoundFromId(_pSound);
    }

    if(_pSound)
    {
        _pSound->stop();
        _pSound = nullptr;
    }
    _id = 0;
    _isTalking = false;
}

void Actor::Impl::TalkingState::load(int id)
{
    _id = id;
    std::string name = str_toupper(_pActor->getName()).append("_").append(std::to_string(id));
    auto soundDefinition = _pActor->pImpl->_engine.getSoundManager().defineSound(name + ".ogg");
    if (!soundDefinition)
    {
        std::cerr << "File " << name << ".ogg not found" << std::endl;
        return;
    }
    _pSound = _pActor->pImpl->_engine.getSoundManager().playSound(soundDefinition);
    if (_pSound)
        _pSound->setVolume(_pActor->pImpl->_volume);

    std::string path;
    path.append(name).append(".lip");
    std::cout << "load lip " << path << std::endl;
    _lip.setSettings(_pActor->pImpl->_engine.getSettings());
    _lip.load(path);

    _sayText = _pActor->pImpl->_engine.getText(id);
    std::wregex re(L"(\\{([^\\}]*)\\})");
    std::wsmatch matches;
    if (std::regex_search(_sayText, matches, re))
    {
        auto anim = matches[1].str();
        std::wcout << "talk anim " << anim << std::endl;
        _pActor->getCostume().setState((char *)anim.data());
        _sayText = matches.suffix();
    }
    _isTalking = true;
    _index = 0;
    _clock.restart();
}

void Actor::Impl::TalkingState::update(const sf::Time &elapsed)
{
    if (!_isTalking)
        return;

    auto time = _lip.getData()[_index].time;
    if (_clock.getElapsedTime() > time)
    {
        _index = _index + 1;
    }
    if (_index == _lip.getData().size())
    {
        if (_ids.empty())
        {
            _isTalking = false;
            _id = 0;
            _pActor->getCostume().setHeadIndex(0);
            return;
        }
        load(_ids.front());
        _ids.erase(_ids.begin());
        return;
    }
    auto letter = _lip.getData()[_index].letter;
    if (letter == 'X' || letter == 'G')
        letter = 'A';
    if (letter == 'H')
        letter = 'D';
    auto index = letter - 'A';
    // TODO: what is the correspondance between letter and head index ?
    _pActor->getCostume().setHeadIndex(index);
}

void Actor::Impl::TalkingState::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    Text text;
    auto screen = target.getView().getSize();
    auto scale = screen.y / 2.f / 512.f;
    text.setScale(scale, scale);
    text.setFont(_font);
    text.setFillColor(_talkColor);
    text.setString(_sayText);
    auto bounds = text.getLocalBounds();

    sf::Transformable t;
    t.move((sf::Vector2f)-_talkOffset - sf::Vector2f(bounds.width * scale / 2.f, 0));
    states.transform *= t.getTransform();

    target.draw(text, states);
}

Actor::Actor(Engine &engine)
    : pImpl(std::make_unique<Impl>(engine))
{
    pImpl->_talkingState.setActor(this);
    pImpl->_walkingState.setActor(this);
    pImpl->_costume.setActor(this);
}

Actor::~Actor() = default;

const Room *Actor::getRoom() const
{
    return pImpl->_pRoom;
}

int Actor::getZOrder() const
{
    return static_cast<int>(getRoom()->getRoomSize().y - getPosition().y);
}

void Actor::setRoom(Room *pRoom)
{
    if (pImpl->_pRoom)
    {
        pImpl->_pRoom->removeEntity(this);
    }
    pImpl->_pRoom = pRoom;
    pImpl->_pRoom->setAsParallaxLayer(this, 0);
}

void Actor::move(const sf::Vector2f &offset)
{
    _transform.move(offset);
}

void Actor::setCostume(const std::string &name, const std::string &sheet)
{
    std::string path;
    path.append(name).append(".json");
    pImpl->_costume.loadCostume(path, sheet);
    // don't know if it's necessary, reyes has no costume in the intro
    pImpl->_costume.setFacing(Facing::FACE_FRONT);
    pImpl->_costume.setAnimation("stand_front");
}

void Actor::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    if (!isVisible())
        return;
    auto size = pImpl->_pRoom->getRoomSize();
    auto scale = pImpl->_pRoom->getRoomScaling().getScaling(size.y - getPosition().y);
    auto transform = _transform;
    transform.scale(scale, scale);
    transform.move((sf::Vector2f)-pImpl->_renderOffset * scale);
    states.transform *= transform.getTransform();
    target.draw(pImpl->_costume, states);

    // draw actor position
    // sf::RectangleShape rectangle;
    // rectangle.setFillColor(sf::Color::Red);
    // rectangle.setSize(sf::Vector2f(2, 2));
    // rectangle.setOrigin(sf::Vector2f(1, 1));
    // target.draw(rectangle, states);
}

void Actor::drawForeground(sf::RenderTarget &target, sf::RenderStates states) const
{
    if (pImpl->_path && pImpl->_pRoom && pImpl->_pRoom->walkboxesVisible())
    {
        target.draw(*pImpl->_path, states);
    }

    if (!pImpl->_talkingState.isTalking())
        return;

    auto actorTransform = states.transform;
    states.transform = actorTransform * _transform.getTransform();
    pImpl->_talkingState.draw(target, states);
}

void Actor::update(const sf::Time &elapsed)
{
    pImpl->_costume.update(elapsed);
    pImpl->_walkingState.update(elapsed);
    pImpl->_talkingState.update(elapsed);
}

void Actor::walkTo(const sf::Vector2f &destination, std::optional<Facing> facing)
{
    if (pImpl->_pRoom == nullptr)
        return;

    auto path = pImpl->_pRoom->calculatePath((sf::Vector2i)getPosition(), (sf::Vector2i)destination);
    pImpl->_path = std::make_unique<Path>(path);

    if (path.size() < 2)
        return;

    pImpl->_walkingState.setDestination(path, facing);
}

void Actor::trigSound(const std::string &name)
{
    auto soundId = pImpl->_engine.getSoundDefinition(name);
    if (!soundId)
        return;
    pImpl->_engine.getSoundManager().playSound(soundId);
}

void Actor::setFps(int fps)
{
    auto pAnim = pImpl->_costume.getAnimation();
    if (pAnim)
    {
        pAnim->setFps(fps);
    }
}

} // namespace ng
