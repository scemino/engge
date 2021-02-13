#pragma once
#include <functional>
#include <ngf/System/TimeSpan.h>
#include <ngf/Graphics/Drawable.h>
#include <ngf/Graphics/RenderStates.h>
#include <ngf/Graphics/RenderTarget.h>

namespace ng {

class Engine;

class SaveLoadDialog final : public ngf::Drawable {
public:
  using Callback =  std::function<void()>;
  using SlotCallback = std::function<void(int slot)>;

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const final;
public:
  SaveLoadDialog();
  ~SaveLoadDialog() final;

  void setSaveMode(bool saveMode);
  [[nodiscard]] bool getSaveMode() const;
  void setCallback(Callback callback);
  void setSlotCallback(SlotCallback callback);
  void setEngine(Engine *pEngine);
  void update(const ngf::TimeSpan &elapsed);
  void updateLanguage();

private:
  struct Impl;
  std::unique_ptr<Impl> m_pImpl;
};
}
