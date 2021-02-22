#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <ngf/Audio/SoundBuffer.h>
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
  friend class SoundManager;

public:
  explicit SoundDefinition(std::string path);
  ~SoundDefinition() final;

  [[nodiscard]] std::string getPath() const { return m_path; };

  void load();

private:
  std::string m_path;
  bool m_isLoaded{false};
  ngf::SoundBuffer m_buffer;
};
} // namespace ng