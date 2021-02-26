#pragma once
#include <glm/vec2.hpp>
#include <functional>
#include <initializer_list>
#include <vector>
#include <ngf/Graphics/RenderTarget.h>
#include <ngf/Graphics/RenderStates.h>
#include <ngf/Graphics/Drawable.h>
#include <engge/Graphics/Text.hpp>
#include "UI/Control.hpp"

namespace ng {
class Engine;

class SwitchButton final : public Control {
public:
  using Callback = std::function<void(int)>;

  SwitchButton(std::initializer_list<int> ids,
               float y,
               bool enabled = true,
               int index = 0,
               Callback callback = nullptr);
  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const final;
  void update(const ngf::TimeSpan &elapsed, glm::vec2 pos) final;

private:
  bool contains(glm::vec2 pos) const final;
  void onEngineSet() final;
  void onClick() final;
  void onStateChanged() final;

private:
  std::vector<int> m_ids;
  int m_index{0};
  float m_y{0};
  Callback m_callback{nullptr};
  ng::Text m_text;
};
}
