#include <optional>
#include "engge/Dialog/ConditionVisitor.hpp"
#include "engge/System/Logger.hpp"
#include "engge/Dialog/ExpressionVisitor.hpp"
#include "engge/Dialog/DialogPlayer.hpp"
#include "engge/Dialog/DialogScriptAbstract.hpp"

namespace ng {

enum class DialogSelectMode {
  Show,
  Choose
};

class ConditionStateVisitor : public Ast::AstVisitor {
public:
  explicit ConditionStateVisitor(DialogPlayer& dialogPlayer, DialogSelectMode selectMode);
  ~ConditionStateVisitor() override = default;
  [[nodiscard]] std::optional<DialogConditionState> getState() const { return m_state; }

private:
  void visit(const Ast::OnceCondition &node) override;
  void visit(const Ast::ShowOnceCondition &node) override;
  void visit(const Ast::OnceEverCondition &node) override;
  void visit(const Ast::TempOnceCondition &node) override;

  void setState(int32_t line, DialogConditionMode mode);

private:
  DialogPlayer& m_dialogPlayer;
  DialogSelectMode m_selectMode;
  std::optional<DialogConditionState> m_state;
};

ConditionStateVisitor::ConditionStateVisitor(DialogPlayer &dialogPlayer, DialogSelectMode selectMode)
    : m_dialogPlayer(dialogPlayer), m_selectMode(selectMode) {
}

void ConditionStateVisitor::visit(const Ast::OnceCondition &condition) {
  if (m_selectMode == DialogSelectMode::Choose) {
    setState(condition.getLine(), DialogConditionMode::Once);
  }
}

void ConditionStateVisitor::visit(const Ast::ShowOnceCondition &condition) {
  if (m_selectMode == DialogSelectMode::Show) {
    setState(condition.getLine(), DialogConditionMode::ShowOnce);
  }
}

void ConditionStateVisitor::visit(const Ast::OnceEverCondition &condition) {
  if (m_selectMode == DialogSelectMode::Choose) {
    setState(condition.getLine(), DialogConditionMode::OnceEver);
  }
}

void ConditionStateVisitor::visit(const Ast::TempOnceCondition &condition) {
  if (m_selectMode == DialogSelectMode::Show) {
    setState(condition.getLine(), DialogConditionMode::TempOnce);
  }
}

void ConditionStateVisitor::setState(int32_t line, DialogConditionMode mode) {
  DialogConditionState state;
  state.mode = mode;
  state.line = line;
  state.dialog = m_dialogPlayer.getDialogName();
  state.actorKey = m_dialogPlayer.getActor();
  m_state = state;
}

DialogPlayer::DialogPlayer(DialogScriptAbstract &script) : _script(script) {}
DialogPlayer::~DialogPlayer() = default;

void DialogPlayer::start(const std::string &actor, const std::string &name, const std::string &node) {
  resetState();
  m_actor = actor;
  m_dialogName = name;
  std::string path;
  path.append(name).append(".byack");

  YackTokenReader reader;
  reader.load(path);
  YackParser parser(reader);
  m_pCompilationUnit = parser.parse();
  selectLabel(node);
}

void DialogPlayer::choose(int choiceId) {
  if (m_state != DialogPlayerState::WaitingForChoice)
    return;
  int i = 1;
  for (const auto &choice : m_choices) {
    if (!choice)
      continue;
    if (i++ == choiceId) {
      auto pChoice = dynamic_cast<ng::Ast::Choice *>(choice->expression.get());

      for(const auto& cond : choice->conditions) {
        ConditionStateVisitor visitor(*this, DialogSelectMode::Choose);
        cond->accept(visitor);
        auto state = visitor.getState();
        if (state.has_value()) {
          m_states.push_back(state.value());
        }
      }

      if (m_parrot) {
        m_state = DialogPlayerState::WaitingForSayingChoice;
        m_pWaitAction = say(m_actor, pChoice->text);
        m_nextLabel = pChoice->gotoExp->name;
        return;
      }
      selectLabel(pChoice->gotoExp->name);
      return;
    }
  }
}

void DialogPlayer::update() {
  switch (m_state) {
  case DialogPlayerState::None:break;
  case DialogPlayerState::Start: running(); break;
  case DialogPlayerState::Running: running(); break;
  case DialogPlayerState::WaitingForChoice:break;
  case DialogPlayerState::WaitingForSayingChoice: {
    if (!m_pWaitAction || m_pWaitAction()) {
      m_pWaitAction = nullptr;
      selectLabel(m_nextLabel);
    }
    break;
  }
  case DialogPlayerState::WaitingEndAnimation:
    if (!m_pWaitAction || m_pWaitAction()) {
      m_pWaitAction = nullptr;
      m_currentStatement++;
      m_state = DialogPlayerState::Running;
    }
    break;
  }
}

DialogManagerState DialogPlayer::getState() const {
  switch (m_state) {
  case DialogPlayerState::None:return DialogManagerState::None;
  case DialogPlayerState::WaitingForChoice: return DialogManagerState::WaitingForChoice;
  case DialogPlayerState::Running: return DialogManagerState::Active;
  case DialogPlayerState::WaitingEndAnimation:return DialogManagerState::Active;
  case DialogPlayerState::WaitingForSayingChoice:return DialogManagerState::Active;
  case DialogPlayerState::Start:return DialogManagerState::Active;
  }
}

void DialogPlayer::running() {
  if (!m_pLabel) {
    m_state = DialogPlayerState::None;
    return;
  }
  auto count = static_cast<int>(m_pLabel->statements.size());
  if (m_currentStatement == count) {
    gotoNextLabel();
    return;
  }
  m_state = DialogPlayerState::Running;
  while (m_currentStatement < count && m_state == DialogPlayerState::Running) {
    auto pCurrentStatement = m_pLabel->statements.at(m_currentStatement).get();
    if (!acceptConditions(pCurrentStatement)) {
      m_currentStatement++;
      continue;
    }
    auto pChoice = dynamic_cast<Ast::Choice *>(pCurrentStatement->expression.get());
    if (pChoice) {
      addChoice(pCurrentStatement, pChoice);
      m_currentStatement++;
      continue;
    }
    if (choicesReady()) {
      m_state = DialogPlayerState::WaitingForChoice;
      return;
    }
    run(pCurrentStatement);
    count = m_pLabel ? static_cast<int>(m_pLabel->statements.size()) : 0;
    if(m_state != DialogPlayerState::WaitingEndAnimation) {
      m_currentStatement++;
    }
  }
  if (choicesReady()) {
    m_state = DialogPlayerState::WaitingForChoice;
    return;
  }
  if(m_state == DialogPlayerState::Running) {
    gotoNextLabel();
  }
}

void DialogPlayer::resetState() {
  m_parrot = true;
  m_limit = 6;
  m_overrideLabel.clear();
  m_states.erase(std::remove_if(m_states.begin(), m_states.end(), [](const auto &state) {
    return state.mode == DialogConditionMode::TempOnce;
  }), m_states.end());
}

void DialogPlayer::selectLabel(const std::string &name) {
  trace("select label {}", name);
  auto it = std::find_if(m_pCompilationUnit->labels.rbegin(),
                         m_pCompilationUnit->labels.rend(),
                         [&name](const std::unique_ptr<Ast::Label> &label) {
                           return label->name == name;
                         });
  m_pLabel = it != m_pCompilationUnit->labels.rend() ? it->get() : nullptr;
  m_currentStatement = 0;
  clearChoices();
  if (m_pLabel) {
    m_state = DialogPlayerState::Start;
    update();
    return;
  }
  m_state = DialogPlayerState::None;
}

void DialogPlayer::endDialog() {
  m_state = DialogPlayerState::None;
  m_pLabel = nullptr;
}

bool DialogPlayer::gotoNextLabel() {
  if (!m_pCompilationUnit) {
    endDialog();
    return false;
  }
  if (!m_pLabel) {
    endDialog();
    return false;
  }
  auto it =
      std::find_if(m_pCompilationUnit->labels.cbegin(), m_pCompilationUnit->labels.cend(), [this](const auto &pLabel) {
        return pLabel.get() == m_pLabel;
      });
  if (it == m_pCompilationUnit->labels.cend()) {
    endDialog();
    return false;
  }
  it++;
  if (it == m_pCompilationUnit->labels.cend()) {
    endDialog();
    return false;
  }
  selectLabel((*it)->name);
  return true;
}

void DialogPlayer::run(Ast::Statement *pStatement) {
  if (!acceptConditions(pStatement))
    return;
  ExpressionVisitor visitor(*this);
  pStatement->expression->accept(visitor);
  m_pWaitAction = visitor.getWaitAction();
  if (m_pWaitAction) {
    m_state = DialogPlayerState::WaitingEndAnimation;
  }
}

void DialogPlayer::addChoice(const Ast::Statement *pStatement, const Ast::Choice *pChoice) {
  if (m_choices[pChoice->number - 1])
    return;
  int count = 0;
  for (auto &_choice : m_choices) {
    if(_choice) count++;
  }
  if(count >= m_limit) return;
  trace("Add choice {}", pChoice->text);
  m_choices[pChoice->number - 1] = pStatement;
}

void DialogPlayer::clearChoices() {
  for (auto &choice : m_choices) {
    choice = nullptr;
  }
}

bool DialogPlayer::choicesReady() const {
  return std::any_of(m_choices.cbegin(), m_choices.cend(), [](const auto &pStatement) {
    return pStatement != nullptr;
  });
}

bool DialogPlayer::acceptConditions(const Ast::Statement *pStatement) {
  ConditionVisitor visitor(*this);
  for (const auto &cond : pStatement->conditions) {
    cond->accept(visitor);
    if (!visitor.isAccepted())
      return false;
  }

  for (const auto &cond : pStatement->conditions) {
    ConditionStateVisitor stateVisitor(*this, DialogSelectMode::Show);
    cond->accept(stateVisitor);
    auto state = stateVisitor.getState();
    if (state.has_value()) {
      m_states.push_back(state.value());
    }
  }

  return true;
}

void DialogPlayer::allowObjects(bool allow) { m_allowObjects = allow; }
void DialogPlayer::dialog(const std::string &actor) { m_actor = actor; }
void DialogPlayer::execute(const std::string &code) { _script.execute(code); }
void DialogPlayer::gotoLabel(const std::string &label) { selectLabel(label); }
void DialogPlayer::limit(int max) { m_limit = max; }
void DialogPlayer::override(const std::string &label) { m_overrideLabel = label; }
void DialogPlayer::parrot(bool enabled) { m_parrot = enabled; }
std::function<bool()> DialogPlayer::pause(ngf::TimeSpan seconds) { return _script.pause(seconds); }
std::function<bool()> DialogPlayer::say(const std::string &actor, const std::string &text) {
  return _script.say(actor, text);
}
void DialogPlayer::shutup() { return _script.shutup(); }
std::function<bool()> DialogPlayer::waitFor(const std::string &actor) { return _script.waitFor(actor); }
std::function<bool()> DialogPlayer::waitWhile(const std::string &condition) { return _script.waitWhile(condition); }

bool DialogPlayer::isOnce(int32_t line) const {
  auto it =
      std::find_if(m_states.cbegin(), m_states.cend(),
                   [this, line](const auto &state) {
                     return state.mode == DialogConditionMode::Once &&
                         state.actorKey == m_actor &&
                         state.dialog == m_dialogName &&
                         state.line == line;
                   });
  return it == m_states.cend();
}

bool DialogPlayer::isShowOnce(int32_t line) const {
  auto it =
      std::find_if(m_states.cbegin(), m_states.cend(),
                   [this, line](const auto &state) {
                     return state.mode == DialogConditionMode::ShowOnce &&
                         state.actorKey == m_actor &&
                         state.dialog == m_dialogName &&
                         state.line == line;
                   });
  return it == m_states.cend();
}

bool DialogPlayer::isOnceEver(int32_t line) const {
  auto it =
      std::find_if(m_states.cbegin(), m_states.cend(),
                   [this, line](const auto &state) {
                     return state.mode == DialogConditionMode::OnceEver &&
                         state.dialog == m_dialogName &&
                         state.line == line;
                   });
  return it == m_states.cend();
}

bool DialogPlayer::isTempOnce(int32_t line) const {
  auto it =
      std::find_if(m_states.cbegin(), m_states.cend(),
                   [this, line](const auto &state) {
                     return state.mode == DialogConditionMode::TempOnce &&
                         state.actorKey == m_actor &&
                         state.dialog == m_dialogName &&
                         state.line == line;
                   });
  return it == m_states.cend();
}

bool DialogPlayer::executeCondition(const std::string &condition) const { return _script.executeCondition(condition); }
}