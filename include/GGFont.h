#pragma once
#include "SFML/Graphics.hpp"
#include "TextureManager.h"

namespace gg
{
class GGFont
{
  public:
    GGFont();
    ~GGFont();

    void setSettings(const GGEngineSettings *settings);
    void setTextureManager(TextureManager *textureManager);

    void load(const std::string &path);
    void draw(const std::string &text, sf::RenderTarget &target, sf::RenderStates states = sf::RenderStates::Default);

  private:
    const GGEngineSettings *_settings;
    TextureManager *_textureManager;
    sf::Sprite _sprite;
    std::string _jsonFilename;
};
} // namespace gg