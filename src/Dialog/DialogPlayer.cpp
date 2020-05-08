#include <optional>
#include "Dialog/ConditionVisitor.hpp"
#include "System/Logger.hpp"
#include "Dialog/ExpressionVisitor.hpp"
#include "Dialog/DialogPlayer.hpp"
#include "Dialog/DialogScriptAbstract.hpp"

namespace ng {

enum class DialogSelectMode {
  Show,
  Choose
};

class ConditionStateVisitor : public Ast::AstVisitor {
public:
  explicit ConditionStateVisitor(DialogPlayer& dialogPlayer, DialogSelectMode selectMode);
  ~ConditionStateVisitor() override = default;
  [[nodiscard]] std::optional<DialogConditionState> getState() const { return _state; }

private:
  void visit(const Ast::OnceCondition &node) override;
  void visit(const Ast::ShowOnceCondition &node) override;
  void visit(const Ast::OnceEverCondition &node) override;
  void visit(const Ast::TempOnceCondition &node) override;

  void setState(int32_t line, DialogConditionMode mode);

private:
  DialogPlayer& _dialogPlayer;
  DialogSelectMode _selectMode;
  std::optional<DialogConditionState> _state;
};

ConditionStateVisitor::ConditionStateVisitor(DialogPlayer &dialogPlayer, DialogSelectMode selectMode)
    : _dialogPlayer(dialogPlayer), _selectMode(selectMode) {
}

void ConditionStateVisitor::visit(const Ast::OnceCondition &condition) {
  if (_selectMode == DialogSelectMode::Choose) {
    setState(condition.getLine(), DialogConditionMode::Once);
  }
}

void ConditionStateVisitor::visit(const Ast::ShowOnceCondition &condition) {
  if (_selectMode == DialogSelectMode::Show) {
    setState(condition.getLine(), DialogConditionMode::ShowOnce);
  }
}

void ConditionStateVisitor::visit(const Ast::OnceEverCondition &condition) {
  if (_selectMode == DialogSelectMode::Choose) {
    setState(condition.getLine(), DialogConditionMode::OnceEver);
  }
}

void ConditionStateVisitor::visit(const Ast::TempOnceCondition &condition) {
  if (_selectMode == DialogSelectMode::Show) {
    setState(condition.getLine(), DialogConditionMode::TempOnce);
  }
}

void ConditionStateVisitor::setState(int32_t line, DialogConditionMode mode) {
  DialogConditionState state;
  state.mode = mode;
  state.line = line;
  state.dialog = _dialogPlayer.getDialogName();
  state.actorKey = _dialogPlayer.getActor();
  _state = state;
}

DialogPlayer::DialogPlayer(DialogScriptAbstract &script) : _script(script) {}
DialogPlayer::~DialogPlayer() = default;

void DialogPlayer::start(const std::string &actor, const std::string &name, const std::string &node) {
  resetState();
  _actor = actor;
  _dialogName = name;
  std::string path;
  path.append(name).append(".byack");

  YackTokenReader reader;
  reader.load(path);
  YackParser parser(reader);
  _pCompilationUnit = parser.parse();
  selectLabel(node);
}

void DialogPlayer::choose(int choiceId) {
  if (_state != DialogPlayerState::WaitingForChoice)
    return;
  int i = 1;
  for (const auto &choice : _choices) {
    if (!choice)
      continue;
    if (i++ == choiceId) {
      auto pChoice = dynamic_cast<ng::Ast::Choice *>(choice->expression.get());

      for(const auto& cond : choice->conditions) {
        ConditionStateVisitor visitor(*this, DialogSelectMode::Choose);
        cond->accept(visitor);
        auto state = visitor.getState();
        if (state.has_value()) {
          _states.push_back(state.value());
        }
      }

      if (_parrot) {
        _state = DialogPlayerState::WaitingForSayingChoice;
        _pWaitAction = say(_actor, pChoice->text);
        _nextLabel = pChoice->gotoExp->name;
        return;
      }
      selectLabel(pChoice->gotoExp->name);
      return;
    }
  }
}

void DialogPlayer::update() {
  switch (_state) {
  case DialogPlayerState::None:break;
  case DialogPlayerState::Start: running(); break;
  case DialogPlayerState::Running: running(); break;
  case DialogPlayerState::WaitingForChoice:break;
  case DialogPlayerState::WaitingForSayingChoice: {
    if (!_pWaitAction || _pWaitAction()) {
      _pWaitAction = nullptr;
      selectLabel(_nextLabel);
    }
    break;
  }
  case DialogPlayerState::WaitingEndAnimation:
    if (!_pWaitAction || _pWaitAction()) {
      _pWaitAction = nullptr;
      _state = DialogPlayerState::Running;
    }
    break;
  }
}

DialogManagerState DialogPlayer::getState() const {
  switch (_state) {
  case DialogPlayerState::None:return DialogManagerState::None;
  case DialogPlayerState::WaitingForChoice: return DialogManagerState::WaitingForChoice;
  case DialogPlayerState::Running: return DialogManagerState::Active;
  case DialogPlayerState::WaitingEndAnimation:return DialogManagerState::Active;
  case DialogPlayerState::WaitingForSayingChoice:return DialogManagerState::Active;
  case DialogPlayerState::Start:return DialogManagerState::Active;
  }
}

void DialogPlayer::running() {
  if (!_pLabel) {
    _state = DialogPlayerState::None;
    return;
  }
  auto count = static_cast<int>(_pLabel->statements.size());
  if (_currentStatement == count) {
    gotoNextLabel();
    return;
  }
  _state = DialogPlayerState::Running;
  while (_currentStatement < count && _state == DialogPlayerState::Running) {
    auto pCurrentStatement = _pLabel->statements[_currentStatement].get();
    if (!acceptConditions(pCurrentStatement)) {
      _currentStatement++;
      continue;
    }
    auto pChoice = dynamic_cast<Ast::Choice *>(pCurrentStatement->expression.get());
    if (pChoice) {
      addChoice(pCurrentStatement, pChoice);
      _currentStatement++;
      continue;
    }
    if (choicesReady()) {
      _state = DialogPlayerState::WaitingForChoice;
      return;
    }
    run(pCurrentStatement);
    _currentStatement++;
  }
  if (choicesReady()) {
    _state = DialogPlayerState::WaitingForChoice;
    return;
  }
}

void DialogPlayer::resetState() {
  _parrot = true;
  _limit = 6;
  _overrideLabel.clear();
  _states.erase(std::remove_if(_states.begin(), _states.end(), [](const auto &state) {
    return state.mode == DialogConditionMode::TempOnce;
  }), _states.end());
}

void DialogPlayer::selectLabel(const std::string &name) {
  trace("select label {}", name);
  auto it = std::find_if(_pCompilationUnit->labels.begin(),
                         _pCompilationUnit->labels.end(),
                         [&name](const std::unique_ptr<Ast::Label> &label) {
                           return label->name == name;
                         });
  _pLabel = it != _pCompilationUnit->labels.end() ? it->get() : nullptr;
  _currentStatement = 0;
  clearChoices();
  if (_pLabel) {
    _state = DialogPlayerState::Start;
    update();
    return;
  }
  _state = DialogPlayerState::None;
}

void DialogPlayer::endDialog() {
  _state = DialogPlayerState::None;
  _pLabel = nullptr;
}

bool DialogPlayer::gotoNextLabel() {
  if (!_pCompilationUnit) {
    endDialog();
    return false;
  }
  if (!_pLabel) {
    endDialog();
    return false;
  }
  auto it =
      std::find_if(_pCompilationUnit->labels.cbegin(), _pCompilationUnit->labels.cend(), [this](const auto &pLabel) {
        return pLabel.get() == _pLabel;
      });
  if (it == _pCompilationUnit->labels.cend()) {
    endDialog();
    return false;
  }
  it++;
  if (it == _pCompilationUnit->labels.cend()) {
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
  _pWaitAction = visitor.getWaitAction();
  if (_pWaitAction) {
    _state = DialogPlayerState::WaitingEndAnimation;
  }
}

void DialogPlayer::addChoice(const Ast::Statement *pStatement, const Ast::Choice *pChoice) {
  if (_choices[pChoice->number - 1])
    return;
  int count = 0;
  for (auto &_choice : _choices) {
    if(_choice) count++;
  }
  if(count >= _limit) return;
  trace("Add choice {}", pChoice->text);
  _choices[pChoice->number - 1] = pStatement;
}

void DialogPlayer::clearChoices() {
  for (auto &_choice : _choices) {
    _choice = nullptr;
  }
}

bool DialogPlayer::choicesReady() const {
  return std::any_of(_choices.cbegin(), _choices.cend(), [](const auto &pStatement) {
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
      _states.push_back(state.value());
    }
  }

  return true;
}

void DialogPlayer::allowObjects(bool allow) { _allowObjects = allow; }
void DialogPlayer::dialog(const std::string &actor) { _actor = actor; }
void DialogPlayer::execute(const std::string &code) { _script.execute(code); }
void DialogPlayer::gotoLabel(const std::string &label) { selectLabel(label); }
void DialogPlayer::limit(int max) { _limit = max; }
void DialogPlayer::override(const std::string &label) { _overrideLabel = label; }
void DialogPlayer::parrot(bool enabled) { _parrot = enabled; }
std::function<bool()> DialogPlayer::pause(sf::Time seconds) { return _script.pause(seconds); }
std::function<bool()> DialogPlayer::say(const std::string &actor, const std::string &text) {
  return _script.say(actor, text);
}
void DialogPlayer::shutup() { return _script.shutup(); }
std::function<bool()> DialogPlayer::waitFor(const std::string &actor) { return _script.waitFor(actor); }
std::function<bool()> DialogPlayer::waitWhile(const std::string &condition) { return _script.waitWhile(condition); }

bool DialogPlayer::isOnce(int32_t line) const {
  auto it =
      std::find_if(_states.cbegin(), _states.cend(),
                   [this, line](const auto &state) {
                     return state.mode == DialogConditionMode::Once &&
                         state.actorKey == _actor &&
                         state.dialog == _dialogName &&
                         state.line == line;
                   });
  return it == _states.cend();
}

bool DialogPlayer::isShowOnce(int32_t line) const {
  auto it =
      std::find_if(_states.cbegin(), _states.cend(),
                   [this, line](const auto &state) {
                     return state.mode == DialogConditionMode::ShowOnce &&
                         state.actorKey == _actor &&
                         state.dialog == _dialogName &&
                         state.line == line;
                   });
  return it == _states.cend();
}

bool DialogPlayer::isOnceEver(int32_t line) const {
  auto it =
      std::find_if(_states.cbegin(), _states.cend(),
                   [this, line](const auto &state) {
                     return state.mode == DialogConditionMode::OnceEver &&
                         state.dialog == _dialogName &&
                         state.line == line;
                   });
  return it == _states.cend();
}

bool DialogPlayer::isTempOnce(int32_t line) const {
  auto it =
      std::find_if(_states.cbegin(), _states.cend(),
                   [this, line](const auto &state) {
                     return state.mode == DialogConditionMode::TempOnce &&
                         state.actorKey == _actor &&
                         state.dialog == _dialogName &&
                         state.line == line;
                   });
  return it == _states.cend();
}

bool DialogPlayer::executeCondition(const std::string &condition) const { return _script.executeCondition(condition); }
}