#pragma once
#include "SaveLoadDialog.hpp"

namespace ng {

class StartScreenDialog final : public ngf::Drawable {
public:
  typedef std::function<void()> Callback;

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const final;
public:
  StartScreenDialog();
  ~StartScreenDialog() final;

  void setNewGameCallback(Callback callback);
  void setSlotCallback(SaveLoadDialog::SlotCallback callback);
  void setEngine(Engine *pEngine);
  void update(const ngf::TimeSpan &elapsed);

private:
  struct Impl;
  std::unique_ptr<Impl> _pImpl;
};
}
