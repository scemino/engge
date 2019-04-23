#pragma once
#include "nlohmann/json.hpp"
#include "SFML/Graphics.hpp"
#include "TextureManager.h"

namespace ng
{
class Font
{
public:
  Font();
  ~Font();

  void setSettings(EngineSettings *settings);
  void setTextureManager(TextureManager *textureManager);

  void load(const std::string &path);

  const sf::Texture &getTexture() const { return _texture; }
  sf::IntRect getRect(char letter) const;
  sf::IntRect getSize(char letter) const;

private:
  EngineSettings *_settings{nullptr};
  TextureManager *_textureManager{nullptr};
  std::string _path;
  std::string _jsonFilename;
  nlohmann::json _json;
  sf::Texture _texture;
};

enum class NGTextAlignment
{
  Center,
  Left
};

class NGText : public sf::Drawable, public sf::Transformable
{
public:
  NGText();
  void setFont(const Font &font) { _font = font; }
  void setColor(const sf::Color &color) { _color = color; }
  void setText(const sf::String &text) { _text = text; }
  void setAlignment(NGTextAlignment alignment) { _alignment = alignment; }
  sf::FloatRect getBoundRect() const;

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
  Font _font;
  sf::Color _color;
  sf::String _text;
  NGTextAlignment _alignment;
};

} // namespace ng