#include "GGActor.h"
#include "GGRoom.h"

namespace gg
{
GGActor::GGActor(TextureManager &textureManager)
    : _settings(textureManager.getSettings()),
      _costume(textureManager),
      _color(sf::Color::White),
      _talkColor(sf::Color::White),
      _zorder(0),
      _isVisible(true),
      _use(true),
      _pRoom(nullptr)
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

    _font.draw(_sayText, target, _talkColor, states);
}

void GGActor::update(const sf::Time &time)
{
    _costume.update(time);
}
} // namespace gg
