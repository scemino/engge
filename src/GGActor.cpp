#include "GGActor.h"
#include "GGRoom.h"

namespace gg
{
WalkingState::WalkingState(GGActor &actor)
    : _actor(actor), _isWalking(false)
{
}

void WalkingState::setDestination(const sf::Vector2f &destination)
{
    _destination = destination;
    auto pos = _actor.getPosition();
    _actor.getCostume().setFacing(((_destination.x - pos.x) > 0) ? Facing::FACE_RIGHT : Facing::FACE_LEFT);
    _actor.getCostume().setState("walk");
    _actor.getCostume().getAnimation()->play(true);
    _isWalking = true;
}

void WalkingState::update(const sf::Time &elapsed)
{
    if (!_isWalking)
        return;

    auto pos = _actor.getPosition();
    auto delta = (_destination - pos);
    auto speed = _actor.getWalkSpeed();
    auto offset = sf::Vector2f(speed) * elapsed.asSeconds();
    if (offset.x > delta.x)
        offset.x = delta.x;
    if (offset.x < -delta.x)
        offset.x = -delta.x;
    if (offset.y < -delta.y)
        offset.y = -delta.y;
    if (offset.y > delta.y)
        offset.y = delta.y;
    _actor.setPosition(pos + offset);
    if (fabs(_destination.x - pos.x) <= 1 && fabs(_destination.y - pos.y) <= 1)
    {
        _isWalking = false;
        std::cout << "Play anim stand" << std::endl;
        _actor.getCostume().setState("stand");
    };
}

GGActor::GGActor(TextureManager &textureManager)
    : _settings(textureManager.getSettings()),
      _costume(textureManager),
      _color(sf::Color::White),
      _talkColor(sf::Color::White),
      _zorder(0),
      _isVisible(true),
      _use(true),
      _pRoom(nullptr),
      _walkingState(*this),
      _speed(30, 15)
{
    _font.setSettings(&_settings);
    _font.setTextureManager(&textureManager);
    _font.load("FontModernSheet");
}

GGActor::~GGActor() = default;

int GGActor::getZOrder() const
{
    return getRoom()->getRoomSize().y - getPosition().y;
}

void GGActor::setRoom(GGRoom *pRoom)
{
    _pRoom = pRoom;
    _pRoom->setAsParallaxLayer(this, 0);
}

void GGActor::move(const sf::Vector2f &offset)
{
    _transform.move(offset);
}

void GGActor::setPosition(const sf::Vector2f &pos)
{
    _transform.setPosition(pos);
}

void GGActor::setCostume(const std::string &name, const std::string &sheet)
{
    std::string path(_settings.getGamePath());
    path.append(name).append(".json");
    _costume.loadCostume(path, sheet);
}

void GGActor::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    auto actorTransform = states.transform;
    auto transform = _transform;
    transform.move((sf::Vector2f)-_renderOffset);
    states.transform *= transform.getTransform();
    target.draw(_costume, states);
    if (_sayText.empty())
        return;

    transform = _transform;
    transform.move((sf::Vector2f)-_talkOffset);
    states.transform = actorTransform * transform.getTransform();

    GGText text;
    text.setFont(_font);
    text.setColor(_talkColor);
    text.setText(_sayText);
    target.draw(text, states);
}

void GGActor::update(const sf::Time &elapsed)
{
    _costume.update(elapsed);
    _walkingState.update(elapsed);
}
} // namespace gg
