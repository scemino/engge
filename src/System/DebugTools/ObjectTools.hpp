#pragma once
#include <imgui.h>

namespace ng {
struct Animation;
class Engine;
class Object;

class ObjectTools final {
public:
  explicit ObjectTools(Engine &engine);

  void render();

private:
  void showProperties(Object *object);
  void showAnimations(Object *object);
  static void showAnimationNode(Animation *anim);

public:
  bool objectsVisible{false};

private:
  Engine &m_engine;
  int m_objectId{0};
  ImGuiTextFilter m_textFilter;
  bool m_showProperties{false};
  bool m_showAnimations{false};
};
}