#pragma once
#include "SFML/Graphics.hpp"

namespace ng {
class OptionsDialog : public sf::Drawable {
public:
  typedef std::function<void()> Callback;

public:
  OptionsDialog();
  ~OptionsDialog() override;

  void setCallback(Callback callback);
  void setSaveEnabled(bool enabled);
  void setEngine(Engine *pEngine);
  void update(const sf::Time &elapsed);

  void showHelp();

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
  struct Impl;
  std::unique_ptr<Impl> _pImpl;
};
}
