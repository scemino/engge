#pragma once
#include <ngf/Graphics/Drawable.h>

namespace ng {
class OptionsDialog : public ngf::Drawable {
public:
  typedef std::function<void()> Callback;

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const override;
public:
  OptionsDialog();
  ~OptionsDialog() override;

  void setCallback(Callback callback);
  void setSaveEnabled(bool enabled);
  void setEngine(Engine *pEngine);
  void update(const ngf::TimeSpan &elapsed);

  void showHelp();

private:
  struct Impl;
  std::unique_ptr<Impl> _pImpl;
};
}
