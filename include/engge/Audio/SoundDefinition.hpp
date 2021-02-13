#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <SFML/Audio.hpp>
#include "engge/Engine/Function.hpp"
#include "engge/Scripting/ScriptObject.hpp"

namespace ng {
class SoundId;

class Sound : public ScriptObject {
public:
  ~Sound() override;
};

class SoundDefinition final : public Sound {
  friend class SoundId;

public:
  explicit SoundDefinition(std::string path);
  ~SoundDefinition() final;

  const std::string &getPath() const { return m_path; };

  void load();

private:
  std::string m_path;
  bool m_isLoaded{false};
  sf::SoundBuffer m_buffer;
};
} // namespace ng