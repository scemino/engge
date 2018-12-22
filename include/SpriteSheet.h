#pragma once
#include <string>
#include <map>
#include "TextureManager.h"
#include "EngineSettings.h"

namespace ng
{
class SpriteSheet
{
  public:
    SpriteSheet(TextureManager &textureManager, const EngineSettings &settings)
        : _textureManager(textureManager), _settings(settings)
    {
    }
    ~SpriteSheet() = default;

    void load(const std::string &name);
    const sf::Texture &getTexture() const{return _texture;}
    sf::IntRect getRect(const std::string& name) const;

  private:
    TextureManager &_textureManager;
    const EngineSettings &_settings;
    sf::Texture _texture;
    std::map<std::string, sf::IntRect> _rects;
};
} // namespace ng
