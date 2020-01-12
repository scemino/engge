#pragma once
#include <memory>
#include "SFML/Graphics.hpp"
#include "NonCopyable.h"

namespace ng
{
class Font;

// should be renamed to ResourceManager
class TextureManager : public NonCopyable
{
private:
  std::map<std::string, std::shared_ptr<sf::Texture>> _textureMap;
  std::map<std::string, std::shared_ptr<Font>> _fontMap;

public:
  TextureManager();
  ~TextureManager();

  const sf::Texture &get(const std::string &id);
  const Font &getFont(const std::string &id);

private:
  void load(const std::string &id);
  void loadFont(const std::string &id);
};
} // namespace ng