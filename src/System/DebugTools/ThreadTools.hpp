#pragma once

namespace ng {
class Engine;

class ThreadTools final {
public:
  explicit ThreadTools(Engine &engine);

  void render();

public:
  bool threadsVisible{false};

private:
  Engine &m_engine;
};
}