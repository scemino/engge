#pragma once

namespace ng {
class Engine;

class SoundTools final {
public:
  explicit SoundTools(Engine &engine);
  void render();

public:
  bool soundsVisible{false};

private:
  Engine &m_engine;
};
}