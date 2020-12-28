#pragma once
#include <array>
#include <optional>
#include <ngf/Graphics/Drawable.h>
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
  mutable glm::vec2 pos;
};

class DialogManager : public ngf::Drawable {
public:
  void setEngine(Engine *pEngine);
  void start(const std::string &actor, const std::string &name, const std::string &node);
  void update(const ngf::TimeSpan &elapsed);

  void setMousePosition(glm::vec2 pos);

  const std::vector<DialogConditionState>& getStates() const { return _pPlayer->getStates(); }
  std::vector<DialogConditionState>& getStates() { return _pPlayer->getStates(); }
  [[nodiscard]] DialogManagerState getState() const { return _state; }
  void choose(int choice);

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const override;
private:
  void updateChoices(const ngf::TimeSpan &elapsed);
  void updateDialogSlots();
  static void onDialogEnded();

private:
  Engine *_pEngine{nullptr};
  DialogManagerState _state{DialogManagerState::None};
  glm::vec2 _mousePos;
  std::unique_ptr<EngineDialogScript> _pEngineDialogScript;
  std::unique_ptr<DialogPlayer> _pPlayer;
  std::array<DialogSlot, 9> _slots;
};
} // namespace ng
