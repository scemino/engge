#pragma once
#include <functional>
#include <ngf/Graphics/Drawable.h>
#include <ngf/Graphics/RenderTarget.h>
#include <ngf/Graphics/RenderStates.h>
#include <ngf/System/TimeSpan.h>

namespace ng {
class Engine;

class HelpDialog final : public ngf::Drawable {
public:
  using Callback = std::function<void()>;

public:
  HelpDialog();
  ~HelpDialog() final;

  void init(Engine *pEngine, Callback exitCallback, std::initializer_list<int> pages);

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const final;
  void update(const ngf::TimeSpan &elapsed);

private:
  struct Impl;
  std::unique_ptr<Impl> m_pImpl;
};

}
