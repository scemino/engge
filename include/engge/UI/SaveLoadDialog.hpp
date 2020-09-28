#pragma once
#include <functional>
#include <SFML/Graphics.hpp>

namespace ng {

class Engine;

class SaveLoadDialog : public sf::Drawable {
public:
  typedef std::function<void()> Callback;
  typedef std::function<void(int slot)> SlotCallback;

public:
  SaveLoadDialog();
  ~SaveLoadDialog() override;

  void setSaveMode(bool saveMode);
  [[nodiscard]] bool getSaveMode() const;
  void setCallback(Callback callback);
  void setSlotCallback(SlotCallback callback);
  void setEngine(Engine *pEngine);
  void update(const sf::Time &elapsed);
  void updateLanguage();

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
  struct Impl;
  std::unique_ptr<Impl> _pImpl;
};
}
