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

class DialogManager final : public ngf::Drawable {
public:
  void setEngine(Engine *pEngine);
  void start(const std::string &actor, const std::string &name, const std::string &node);
  void update(const ngf::TimeSpan &elapsed);

  void setMousePosition(glm::vec2 pos);

  const std::vector<DialogConditionState> &getStates() const { return m_pPlayer->getStates(); }
  std::vector<DialogConditionState> &getStates() { return m_pPlayer->getStates(); }
  [[nodiscard]] DialogManagerState getState() const { return m_state; }
  void choose(int choice);

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const override;

private:
  void updateChoices(const ngf::TimeSpan &elapsed);
  void updateDialogSlots();
  static void onDialogEnded();

private:
  Engine *m_pEngine{nullptr};
  DialogManagerState m_state{DialogManagerState::None};
  glm::vec2 m_mousePos{0, 0};
  std::unique_ptr<EngineDialogScript> m_pEngineDialogScript;
  std::unique_ptr<DialogPlayer> m_pPlayer;
  std::array<DialogSlot, 9> m_slots;
};
} // namespace ng
