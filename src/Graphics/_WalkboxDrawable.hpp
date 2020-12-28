#pragma once
#include <ngf/Graphics/Drawable.h>
#include <ngf/Graphics/RenderStates.h>
#include <ngf/Graphics/RenderTarget.h>
#include <ngf/Math/PathFinding/Walkbox.h>

namespace ng {

class _WalkboxDrawable : public ngf::Drawable {
public:
  explicit _WalkboxDrawable(const ngf::Walkbox &walkbox);

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const override;

private:
  const ngf::Walkbox &m_walkbox;
};
}
