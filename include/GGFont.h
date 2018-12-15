#pragma once
#include "nlohmann/json.hpp"
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

  const sf::Texture &getTexture() const { return _texture; }
  sf::IntRect getRect(char letter) const;
  sf::IntRect getSize(char letter) const;

private:
  const GGEngineSettings *_settings;
  TextureManager *_textureManager;
  std::string _path;
  std::string _jsonFilename;
  nlohmann::json _json;
  sf::Texture _texture;
};

enum class GGTextAlignment
{
  Center,
  Left
};

class GGText : public sf::Drawable, public sf::Transformable
{
public:
  GGText();
  void setFont(const GGFont &font) { _font = font; }
  void setColor(const sf::Color &color) { _color = color; }
  void setText(const std::string &text) { _text = text; }
  void setAlignment(GGTextAlignment alignment) { _alignment = alignment; }
  sf::FloatRect getBoundRect() const;

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states = sf::RenderStates::Default) const override;

private:
  GGFont _font;
  sf::Color _color;
  std::string _text;
  GGTextAlignment _alignment;
};

} // namespace gg