#include "GGActor.h"

namespace gg
{
GGActor::GGActor(TextureManager &textureManager)
    : _settings(textureManager.getSettings()),
      _costume(textureManager),
      _color(sf::Color::White),
      _talkColor(sf::Color::White)
{
    _font.setSettings(&_settings);
    _font.setTextureManager(&textureManager);
    _font.load("FontModernSheet");
}

GGActor::~GGActor() = default;

void GGActor::move(const sf::Vector2f &offset)
{
    _transform.translate(offset);
}

void GGActor::setPosition(const sf::Vector2f &pos)
{
    _transform = sf::Transform::Identity;
    _transform.translate(pos);
}

void GGActor::setCostume(const std::string &name, const std::string& sheet)
{
    std::string path(_settings.getGamePath());
    path.append(name).append(".json");
    _costume.loadCostume(path, sheet);
}

void GGActor::draw(sf::RenderWindow &window, const sf::Vector2f &cameraPos) const
{
    sf::RenderStates states;
    states.transform = _transform;
    states.transform.translate(-cameraPos - (sf::Vector2f)_renderOffset);
    _costume.draw(window, states);
    if(_sayText.empty()) return;
    _font.draw(_sayText, window, _talkColor, states);
}

void GGActor::update(const sf::Time &time)
{
    _costume.update(time);
}
} // namespace gg
