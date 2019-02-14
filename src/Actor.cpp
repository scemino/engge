#include <regex>
#include "Engine.h"
#include "Actor.h"
#include "Room.h"
#include "Text.h"
#include "PathFinder.h"

namespace ng
{
Actor::WalkingState::WalkingState(Actor &actor)
    : _actor(actor), _isWalking(false), _facing(Facing::FACE_FRONT)
{
}

void Actor::WalkingState::setDestination(const std::vector<sf::Vector2i> &path, Facing facing)
{
    _path = path;
    _facing = facing;
    auto pos = _actor.getPosition();
    _actor.getCostume().setFacing(((_path[0].x - pos.x) > 0) ? Facing::FACE_RIGHT : Facing::FACE_LEFT);
    _actor.getCostume().setState("walk");
    _actor.getCostume().getAnimation()->play(true);
    _isWalking = true;
}

void Actor::WalkingState::stop()
{
    _isWalking = false;
}

void Actor::WalkingState::update(const sf::Time &elapsed)
{
    if (!_isWalking)
        return;

    auto pos = _actor.getPosition();
    auto delta = (_path[0] - (sf::Vector2i)pos);
    auto speed = _actor.getWalkSpeed();
    auto offset = sf::Vector2f(speed) * elapsed.asSeconds();
    if (delta.x > 0)
    {
        _actor.getCostume().setFacing(Facing::FACE_RIGHT);
        if (offset.x > delta.x)
            offset.x = delta.x;
    }
    else
    {
        _actor.getCostume().setFacing(Facing::FACE_LEFT);
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
    _actor.setPosition(pos + offset);
    if (fabs(_path[0].x - pos.x) <= 1 && fabs(_path[0].y - pos.y) <= 1)
    {
        _path.erase(_path.begin());
        if (_path.size() == 0)
        {
            _isWalking = false;
            std::cout << "Play anim stand" << std::endl;
            _actor.getCostume().setState("stand");
            _actor.getCostume().setFacing(_facing);
        }
        else
        {
            auto pos = _actor.getPosition();
            _actor.getCostume().setFacing(((_path[0].x - pos.x) > 0) ? Facing::FACE_RIGHT : Facing::FACE_LEFT);
            std::cout << "go to : " << _path[0].x << "," << _path[0].y << std::endl;
        }
    };
}

Actor::TalkingState::TalkingState(Actor &actor)
    : _actor(actor), _isTalking(false),
      _talkColor(sf::Color::White), _index(0)
{
    std::string path;
    path.append(_actor._engine.getSettings().getGamePath()).append("SayLineFont.fnt");
    _font.loadFromFile(path);
}

static std::string str_toupper(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::toupper(c); } // correct
    );
    return s;
}

void Actor::TalkingState::say(int id)
{
    if (_isTalking)
    {
        _ids.push_back(id);
        return;
    }

    load(id);
}

void Actor::TalkingState::stop()
{
    _ids.clear();
    if (_sound)
    {
        _sound->stop();
    }
    _isTalking = false;
}

void Actor::TalkingState::load(int id)
{
    std::string name = str_toupper(_actor.getName()).append("_").append(std::to_string(id));
    auto soundDefinition = _actor._engine.getSoundManager().defineSound(name + ".ogg");
    if (!soundDefinition)
    {
        std::cerr << "File " << name << ".ogg not found" << std::endl;
        return;
    }
    _sound = _actor._engine.getSoundManager().playSound(*soundDefinition);
    _sound->setVolume(_actor._volume);

    std::string path;
    path.append(_actor._engine.getSettings().getGamePath()).append(name).append(".lip");
    std::cout << "load lip " << path << std::endl;
    _lip.load(path);

    _sayText = _actor._engine.getText(id);
    std::regex re(R"(\{([^\}]*)\})");
    std::smatch matches;
    if (std::regex_search(_sayText, matches, re))
    {
        auto anim = matches[1].str();
        std::cout << "talk anim " << anim << std::endl;
        _actor.getCostume().setState(anim);
        _sayText = matches.suffix();
    }
    _isTalking = true;
    _index = 0;
    _clock.restart();
}

void Actor::TalkingState::update(const sf::Time &elapsed)
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
            _actor.getCostume().setHeadIndex(0);
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
    _actor.getCostume().setHeadIndex(index);
}

void Actor::TalkingState::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    Text text;
    auto scale = Screen::HalfHeight / 512.f;
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
    : _engine(engine),
      _settings(engine.getSettings()),
      _costume(engine.getTextureManager()),
      _color(sf::Color::White),
      _zorder(0),
      _isVisible(true),
      _use(true),
      _pRoom(nullptr),
      _walkingState(*this),
      _talkingState(*this),
      _speed(30, 15),
      _volume(1.f)
{
    _costume.setActor(this);
}

Actor::~Actor() = default;

int Actor::getZOrder() const
{
    return getRoom()->getRoomSize().y - getPosition().y;
}

void Actor::setRoom(Room *pRoom)
{
    if (_pRoom)
    {
        _pRoom->removeEntity(this);
    }
    _pRoom = pRoom;
    _pRoom->setAsParallaxLayer(this, 0);
}

void Actor::move(const sf::Vector2f &offset)
{
    _transform.move(offset);
}

void Actor::setCostume(const std::string &name, const std::string &sheet)
{
    std::string path(_settings.getGamePath());
    path.append(name).append(".json");
    _costume.loadCostume(path, sheet);
}

void Actor::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    auto size = _pRoom->getRoomSize();
    auto scale = _pRoom->getRoomScaling().getScaling(size.y - getPosition().y);
    auto transform = _transform;
    transform.scale(scale, scale);
    transform.move((sf::Vector2f)-_renderOffset * scale);
    states.transform *= transform.getTransform();
    target.draw(_costume, states);
}

void Actor::drawForeground(sf::RenderTarget &target, sf::RenderStates states) const
{
    if (_path && _pRoom && _pRoom->walkboxesVisible())
    {
        target.draw(*_path, states);
    }

    if (!_talkingState.isTalking())
        return;

    auto actorTransform = states.transform;
    states.transform = actorTransform * _transform.getTransform();
    _talkingState.draw(target, states);
}

void Actor::update(const sf::Time &elapsed)
{
    _costume.update(elapsed);
    _walkingState.update(elapsed);
    _talkingState.update(elapsed);
}

void Actor::walkTo(const sf::Vector2f &destination, Facing facing)
{
    if (_pRoom == nullptr)
        return;

    auto path = _pRoom->calculatePath((sf::Vector2i)getPosition(), (sf::Vector2i)destination);
    _path = std::make_unique<Path>(path);

    if (path.size() < 2)
        return;

    _walkingState.setDestination(path, facing);
}

void Actor::walkTo(const sf::Vector2f &destination)
{
    walkTo(destination, getCostume().getFacing());
}

void Actor::trigSound(const std::string &name)
{
    auto soundId = _engine.getSoundDefinition(name);
    if (!soundId)
        return;
    _engine.getSoundManager().playSound(*soundId);
}

} // namespace ng
