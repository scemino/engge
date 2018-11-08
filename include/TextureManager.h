#pragma once
#include <memory>
#include "SFML/Graphics.hpp"
#include "GGEngineSettings.h"
#include "NonCopyable.h"

namespace gg
{
class TextureManager : public NonCopyable
{
private:
  std::map<std::string, std::shared_ptr<sf::Texture>> _textureMap;
  const GGEngineSettings &_settings;

public:
  explicit TextureManager(const GGEngineSettings &settings);
  ~TextureManager();
  const sf::Texture &get(const std::string &id);
  const GGEngineSettings &getSettings() { return _settings; }

private:
  void load(const std::string &id);
};
} // namespace gg