#pragma once
#include <iostream>
#include <memory>
#include <string>
#include "SFML/Audio.hpp"
#include "Engine/Function.hpp"
#include "Scripting/ScriptObject.hpp"

namespace ng {
class SoundId;

class Sound : public ScriptObject {
public:
  ~Sound() override;
};

class SoundDefinition : public Sound {
  friend class SoundId;

public:
  explicit SoundDefinition(std::string path);
  ~SoundDefinition() override;

  const std::string &getPath() const { return _path; };

  void load();

private:
  std::string _path;
  bool _isLoaded;
  sf::SoundBuffer _buffer;
};
} // namespace ng