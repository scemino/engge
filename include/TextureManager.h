#pragma once
#include <memory>
#include "SFML/Graphics.hpp"
#include "NGEngineSettings.h"
#include "NonCopyable.h"

namespace ng
{
class TextureManager : public NonCopyable
{
private:
  std::map<std::string, std::shared_ptr<sf::Texture>> _textureMap;
  const NGEngineSettings &_settings;

public:
  explicit TextureManager(const NGEngineSettings &settings);
  ~TextureManager();
  const sf::Texture &get(const std::string &id);
  const NGEngineSettings &getSettings() { return _settings; }

private:
  void load(const std::string &id);
};
} // namespace ng