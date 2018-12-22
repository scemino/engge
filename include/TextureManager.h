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
  const EngineSettings &_settings;

public:
  explicit TextureManager(const EngineSettings &settings);
  ~TextureManager();
  const sf::Texture &get(const std::string &id);
  const EngineSettings &getSettings() { return _settings; }

private:
  void load(const std::string &id);
};
} // namespace ng