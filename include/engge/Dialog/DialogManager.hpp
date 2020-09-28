#pragma once
#include <array>
#include <optional>
#include <SFML/Graphics.hpp>
#include "engge/Parsers/YackParser.hpp"
#include "engge/Engine/Function.hpp"
#include "DialogPlayer.hpp"
#include "engge/Graphics/GGFont.hpp"
#include "EngineDialogScript.hpp"

namespace ng {
class Actor;
class Engine;

struct DialogSlot {
  std::wstring text;
  const Ast::Statement *pChoice{nullptr};
  mutable sf::Vector2f pos;
};

class DialogManager : public sf::Drawable {
public:
  void setEngine(Engine *pEngine);
  void start(const std::string &actor, const std::string &name, const std::string &node);
  void update(const sf::Time &elapsed);

  void setMousePosition(sf::Vector2f pos);

  const std::vector<DialogConditionState>& getStates() const { return _pPlayer->getStates(); }
  std::vector<DialogConditionState>& getStates() { return _pPlayer->getStates(); }
  [[nodiscard]] DialogManagerState getState() const { return _state; }
  void choose(int choice);

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
  void updateChoices(const sf::Time &elapsed);
  void updateDialogSlots();
  static void onDialogEnded();

private:
  Engine *_pEngine{nullptr};
  DialogManagerState _state{DialogManagerState::None};
  sf::Vector2f _mousePos;
  std::unique_ptr<EngineDialogScript> _pEngineDialogScript;
  std::unique_ptr<DialogPlayer> _pPlayer;
  std::array<DialogSlot, 9> _slots;
};
} // namespace ng
