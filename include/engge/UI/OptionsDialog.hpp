#pragma once
#include <ngf/Graphics/Drawable.h>

namespace ng {
class OptionsDialog final : public ngf::Drawable {
public:
  using Callback = std::function<void()>;

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const final;
public:
  OptionsDialog();
  ~OptionsDialog() final;

  void setCallback(Callback callback);
  void setSaveEnabled(bool enabled);
  void setEngine(Engine *pEngine);
  void update(const ngf::TimeSpan &elapsed);

  void showHelp();

private:
  struct Impl;
  std::unique_ptr<Impl> m_pImpl;
};
}
