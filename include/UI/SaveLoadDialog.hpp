#pragma once
#include "SFML/Graphics.hpp"

namespace ng {

class Engine;

class SaveLoadDialog : public sf::Drawable {
public:
  typedef std::function<void()> Callback;
  typedef std::function<void(int slot)> SlotCallback;

public:
  SaveLoadDialog();
  ~SaveLoadDialog() override;

  void setCallback(Callback callback);
  void setSlotCallback(SlotCallback callback);
  void setEngine(Engine *pEngine);
  void update(const sf::Time &elapsed);
  void updateLanguage();

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
  struct Impl;
  std::__1::unique_ptr<Impl> _pImpl;
};
}
