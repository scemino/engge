#pragma once
#include <utility>
#include <ngf/Graphics/Drawable.h>
#include <ngf/Graphics/Sprite.h>
#include <engge/Graphics/Text.hpp>
#include "UI/Control.hpp"

namespace ng {
class Engine;
class SpriteSheet;

class Checkbox final : public Control {
public:
  using Callback = std::function<void(bool)>;
  Checkbox(int id, float y, bool enabled = true, bool checked = false, Callback callback = nullptr);

  void setSpriteSheet(SpriteSheet *pSpriteSheet);
  void setChecked(bool checked);
  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const override;
  void update(const ngf::TimeSpan &elapsed, glm::vec2 pos) final;

private:
  bool contains(glm::vec2 pos) const final;
  void onStateChanged() final;
  void onEngineSet() final;
  void onClick() final;
  void updateCheckState();

private:
  Callback m_callback{nullptr};
  int m_id{0};
  float m_y{0};
  bool m_isChecked{false};
  ng::Text m_text;
  ngf::Sprite m_sprite;
  SpriteSheet *m_pSpriteSheet{nullptr};
};
}
