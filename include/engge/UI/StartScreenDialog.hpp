#pragma once
#include <SFML/Graphics.hpp>
#include "SaveLoadDialog.hpp"

namespace ng {

class StartScreenDialog : public sf::Drawable {
public:
  typedef std::function<void()> Callback;

public:
  StartScreenDialog();
  ~StartScreenDialog() override;

  void setNewGameCallback(Callback callback);
  void setSlotCallback(SaveLoadDialog::SlotCallback callback);
  void setEngine(Engine *pEngine);
  void update(const sf::Time &elapsed);

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
  struct Impl;
  std::unique_ptr<Impl> _pImpl;
};
}
