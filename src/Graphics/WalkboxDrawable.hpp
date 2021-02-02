#pragma once
#include <ngf/Graphics/Drawable.h>
#include <ngf/Graphics/RenderStates.h>
#include <ngf/Graphics/RenderTarget.h>
#include <ngf/Math/PathFinding/Walkbox.h>

namespace ng {

class WalkboxDrawable final : public ngf::Drawable {
public:
  WalkboxDrawable(const ngf::Walkbox &walkbox, float roomHeight);

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const override;

private:
  const ngf::Walkbox &m_walkbox;
  const float m_roomHeight{0.f};
};
}
