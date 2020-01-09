#pragma once
#include <memory>
#include "SFML/Graphics.hpp"
#include "NonCopyable.h"

namespace ng
{
class TextureManager : public NonCopyable
{
private:
  std::map<std::string, std::shared_ptr<sf::Texture>> _textureMap;

public:
  TextureManager();
  ~TextureManager();

  const sf::Texture &get(const std::string &id);

private:
  void load(const std::string &id);
};
} // namespace ng