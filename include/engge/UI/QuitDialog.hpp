#pragma once

namespace ng {
class QuitDialog final : public ngf::Drawable {
public:
  typedef std::function<void(bool)> Callback;

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const final;
public:
  QuitDialog();
  ~QuitDialog() final;

  void setCallback(Callback callback);
  void setEngine(Engine *pEngine);
  void update(const ngf::TimeSpan &elapsed);
  void updateLanguage();

private:
  struct Impl;
  std::unique_ptr<Impl> m_pImpl;
};

}
