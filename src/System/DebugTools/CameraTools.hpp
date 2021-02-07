#pragma once
#include <imgui.h>

namespace ng {
class Engine;

class CameraTools final {
public:
  explicit CameraTools(Engine &engine);
  void render();

private:
  Engine &m_engine;
};
}