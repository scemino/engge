#pragma once

namespace ng {
class Engine;
class Object;

class ObjectTools final {
public:
  explicit ObjectTools(Engine &engine);

  void render();

private:
  Engine &m_engine;
  Object *m_pSelectedObject{nullptr};
};
}