#pragma once

namespace ng {
class QuitDialog : public ngf::Drawable {
public:
  typedef std::function<void(bool)> Callback;

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const override;
public:
  QuitDialog();
  ~QuitDialog() override;

  void setCallback(Callback callback);
  void setEngine(Engine *pEngine);
  void update(const ngf::TimeSpan &elapsed);
  void updateLanguage();

private:
  struct Impl;
  std::unique_ptr<Impl> m_pImpl;
};

}
