#pragma once
#include <functional>
#include <ngf/System/TimeSpan.h>
#include <ngf/Graphics/Drawable.h>
#include <ngf/Graphics/RenderStates.h>
#include <ngf/Graphics/RenderTarget.h>

namespace ng {

class Engine;

class SaveLoadDialog : public ngf::Drawable {
public:
  typedef std::function<void()> Callback;
  typedef std::function<void(int slot)> SlotCallback;

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const override;
public:
  SaveLoadDialog();
  ~SaveLoadDialog() override;

  void setSaveMode(bool saveMode);
  [[nodiscard]] bool getSaveMode() const;
  void setCallback(Callback callback);
  void setSlotCallback(SlotCallback callback);
  void setEngine(Engine *pEngine);
  void update(const ngf::TimeSpan &elapsed);
  void updateLanguage();

private:
  struct Impl;
  std::unique_ptr<Impl> _pImpl;
};
}
