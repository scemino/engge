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
  void draw(const std::string &text, sf::RenderTarget &target, const sf::Color& color, sf::RenderStates states = sf::RenderStates::Default) const;

private:
  const GGEngineSettings *_settings;
  TextureManager *_textureManager;
  std::string _path;
  std::string _jsonFilename;
};
} // namespace gg