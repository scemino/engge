#pragma once
#include <imgui.h>
#include <functional>
#include <ngf/Graphics/RenderTarget.h>
#include <ngf/Graphics/RenderStates.h>
#include <engge/Graphics/Text.hpp>
#include "UI/Control.hpp"

namespace ng {
class Engine;

class Button final : public Control {
public:
  using Callback = std::function<void()>;
  enum class Size { Large, Medium };

  Button(int id, float y, Callback callback, bool enabled = true, Size size = Size::Large);
  ~Button() final;

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const override;
  void update(const ngf::TimeSpan &elapsed, glm::vec2 pos) final;

private:
  bool contains(glm::vec2 pos) const final;
  void onStateChanged() final;
  void onEngineSet() final;
  void onClick() final;

private:
  Callback m_callback{nullptr};
  int m_id{0};
  float m_y{0};
  ng::Text m_text;
  Size m_size{Size::Large};
};
}
