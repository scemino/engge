#pragma once
#include <ngf/Graphics/Color.h>
#include <ngf/Graphics/RenderTarget.h>
#include <ngf/Graphics/RenderStates.h>

namespace ng {
struct Animation;

class AnimDrawable {
public:
  void setAnim(const Animation *anim);
  void setFlipX(bool flipX);
  void setColor(const ngf::Color &color);

  void draw(const glm::vec2 &pos, ngf::RenderTarget &target, ngf::RenderStates states) const;

private:
  void draw(const glm::vec2 &pos,
            const Animation &anim,
            ngf::RenderTarget &target,
            ngf::RenderStates states) const;

private:
  const Animation *m_anim{nullptr};
  bool m_flipX{false};
  ngf::Color m_color{ngf::Colors::White};
};
}