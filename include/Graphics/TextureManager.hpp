#pragma once
#include <memory>
#include "SFML/Graphics.hpp"
#include "System/NonCopyable.hpp"

namespace ng
{
class FntFont;
class GGFont;

// should be renamed to ResourceManager
class TextureManager : public NonCopyable
{
private:
  std::map<std::string, std::shared_ptr<sf::Texture>> _textureMap;
  std::map<std::string, std::shared_ptr<GGFont>> _fontMap;
  std::map<std::string, std::shared_ptr<FntFont>> _fntFontMap;

public:
  TextureManager();
  ~TextureManager();

  const sf::Texture &get(const std::string &id);
  const GGFont &getFont(const std::string &id);
  const FntFont &getFntFont(const std::string &id);

private:
  void load(const std::string &id);
  void loadFont(const std::string &id);
  void loadFntFont(const std::string &id);
};
} // namespace ng