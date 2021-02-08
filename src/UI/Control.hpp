#pragma once
#include <functional>
#include <glm/vec2.hpp>
#include <ngf/Graphics/Drawable.h>
#include <ngf/Graphics/RenderTarget.h>
#include <ngf/Graphics/RenderStates.h>

namespace ng {

enum class ControlState {
  None,
  Hover,
  Disabled
};

class Engine;

class Control : public ngf::Drawable {
public:
  void setEngine(Engine *pEngine);
  virtual void update(glm::vec2 pos);

protected:
  explicit Control(bool enabled = true);
  ~Control() override;

  [[nodiscard]] virtual bool contains(glm::vec2 pos) const = 0;

  virtual void onClick() {}
  virtual void onStateChanged() {}
  virtual void onEngineSet() {}

protected:
  Engine *m_pEngine{nullptr};
  ControlState m_state{ControlState::None};
  bool m_wasMouseDown{false};
};
}