#pragma once
#include <memory>
#include "SFML/Graphics.hpp"
#include "EngineSettings.h"
#include "NonCopyable.h"

namespace ng
{
class TextureManager : public NonCopyable
{
private:
  std::map<std::string, std::shared_ptr<sf::Texture>> _textureMap;
  EngineSettings &_settings;

public:
  explicit TextureManager(EngineSettings &settings);
  ~TextureManager();

  const sf::Texture &get(const std::string &id);
  EngineSettings &getSettings() { return _settings; }

private:
  void load(const std::string &id);
};
} // namespace ng