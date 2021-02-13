#pragma once
#include <ngf/Graphics/Drawable.h>
#include <ngf/Graphics/Sprite.h>
#include <engge/Graphics/Text.hpp>
#include <engge/Graphics/SpriteSheet.hpp>
#include <ngf/Graphics/RenderStates.h>
#include <ngf/Graphics/RenderTarget.h>
#include <functional>
#include <optional>
#include <glm/vec2.hpp>
#include "UI/Control.hpp"

namespace ng {
class Engine;

class Slider final : public Control {
public:
  using Callback = std::function<void(float)>;
  Slider(int id, float y, bool enabled, float value, Callback callback);

  void setSpriteSheet(SpriteSheet *pSpriteSheet);
  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const override;
  void update(const ngf::TimeSpan &elapsed, glm::vec2 pos) final;

private:
  bool contains(glm::vec2 pos) const final;

private:
  int m_id{0};
  float m_y{0};
  float m_min{0}, m_max{0}, m_value{0};
  bool m_isDragging{false};
  ngf::Sprite m_sprite;
  ngf::Sprite m_spriteHandle;
  ng::Text m_text;
  std::optional<Callback> m_onValueChanged;
};
}
