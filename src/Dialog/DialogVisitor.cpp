#include "Dialog/DialogManager.hpp"
#include "Engine/Engine.hpp"
#include "Entities/Actor/Actor.hpp"
#include "System/Logger.hpp"
#include "_SayFunction.hpp"
#include "_ExecuteCodeFunction.hpp"
#include "_ShutupFunction.hpp"
#include "_PauseFunction.hpp"
#include "_WaitWhileFunction.hpp"
#include "../System/_Util.hpp"

namespace ng {
DialogVisitor::DialogVisitor(DialogManager &dialogManager)
    : _dialogManager(dialogManager) {
}
void DialogVisitor::visit(const Ast::Statement &node) {
  _pStatement = &node;
  if (!acceptConditions(node))
    return;
  node.expression->accept(*this);
}

DialogVisitor::ConditionStateVisitor::ConditionStateVisitor(DialogManager &dialogManager, DialogSelectMode selectMode)
    : _dialogManager(dialogManager), _selectMode(selectMode) {
}

void DialogVisitor::ConditionStateVisitor::visit(const Ast::OnceCondition &condition) {
  if (_selectMode == DialogSelectMode::Choose) {
    setState(condition.getLine(), DialogConditionMode::Once);
  }
}

void DialogVisitor::ConditionStateVisitor::visit(const Ast::ShowOnceCondition &condition) {
  if (_selectMode == DialogSelectMode::Show) {
    setState(condition.getLine(), DialogConditionMode::ShowOnce);
  }
}

void DialogVisitor::ConditionStateVisitor::visit(const Ast::OnceEverCondition &condition) {
  if (_selectMode == DialogSelectMode::Choose) {
    setState(condition.getLine(), DialogConditionMode::OnceEver);
  }
}

void DialogVisitor::ConditionStateVisitor::visit(const Ast::TempOnceCondition &condition) {
  if (_selectMode == DialogSelectMode::Show) {
    setState(condition.getLine(), DialogConditionMode::TempOnce);
  }
}

void DialogVisitor::ConditionStateVisitor::setState(int32_t line, DialogConditionMode mode) {
  DialogConditionState state;
  state.mode = mode;
  state.line = line;
  state.dialog = _dialogManager.getDialogName();
  state.actorKey = _dialogManager.getTalkingActor()->getKey();
  _state = state;
}

DialogVisitor::ConditionVisitor::ConditionVisitor(DialogVisitor &dialogVisitor)
    : _dialogVisitor(dialogVisitor), _isAccepted(true) {
}

void DialogVisitor::ConditionVisitor::visit(const Ast::CodeCondition &node) {
  const auto &actors = _dialogVisitor._pEngine->getActors();

  // check if the code corresponds to an actor name
  const auto name = node.code;
  auto it = std::find_if(actors.cbegin(), actors.cend(), [&name](const std::unique_ptr<Actor> &actor) {
    return actor->getKey() == name;
  });
  if (it != actors.cend()) {
    // yes, so we check if the current actor is the given actor name
    std::string code("currentActor==");
    code.append(name);
    _isAccepted = _dialogVisitor._pEngine->executeCondition(code);
    return;
  }

  // no it's not an actor, executes the code
  _isAccepted = _dialogVisitor._pEngine->executeCondition(node.code);
}

void DialogVisitor::ConditionVisitor::visit(const Ast::OnceCondition &condition) {
  auto it =
      std::find_if(_dialogVisitor._dialogManager.getStates().cbegin(),
                   _dialogVisitor._dialogManager.getStates().cend(),
                   [this, &condition](const auto &state) {
                     return state.mode == DialogConditionMode::Once &&
                         state.actorKey == _dialogVisitor._dialogManager.getTalkingActor()->getKey() &&
                         state.dialog == _dialogVisitor._dialogManager.getDialogName() &&
                         state.line == condition.getLine();
                   });
  _isAccepted = it == _dialogVisitor._dialogManager.getStates().cend();
}

void DialogVisitor::ConditionVisitor::visit(const Ast::ShowOnceCondition &condition) {
  auto it =
      std::find_if(_dialogVisitor._dialogManager.getStates().cbegin(),
                   _dialogVisitor._dialogManager.getStates().cend(),
                   [this, &condition](const auto &state) {
                     return state.mode == DialogConditionMode::ShowOnce &&
                         state.actorKey == _dialogVisitor._dialogManager.getTalkingActor()->getKey() &&
                         state.dialog == _dialogVisitor._dialogManager.getDialogName() &&
                         state.line == condition.getLine();
                   });
  _isAccepted = it == _dialogVisitor._dialogManager.getStates().cend();
}

void DialogVisitor::ConditionVisitor::visit(const Ast::OnceEverCondition &condition) {
  auto it =
      std::find_if(_dialogVisitor._dialogManager.getStates().cbegin(),
                   _dialogVisitor._dialogManager.getStates().cend(),
                   [this, &condition](const auto &state) {
                     return state.mode == DialogConditionMode::OnceEver &&
                         state.dialog == _dialogVisitor._dialogManager.getDialogName() &&
                         state.line == condition.getLine();
                   });
  _isAccepted = it == _dialogVisitor._dialogManager.getStates().cend();
}

void DialogVisitor::ConditionVisitor::visit(const Ast::TempOnceCondition &condition) {
  auto it =
      std::find_if(_dialogVisitor._dialogManager.getStates().cbegin(),
                   _dialogVisitor._dialogManager.getStates().cend(),
                   [this, &condition](const auto &state) {
                     return state.mode == DialogConditionMode::TempOnce &&
                         state.actorKey == _dialogVisitor._dialogManager.getTalkingActor()->getKey() &&
                         state.dialog == _dialogVisitor._dialogManager.getDialogName() &&
                         state.line == condition.getLine();
                   });
  _isAccepted = it == _dialogVisitor._dialogManager.getStates().cend();
}

void DialogVisitor::select(const Ast::Statement &node) {
  for (auto &condition : node.conditions) {
    ConditionStateVisitor visitor(_dialogManager, DialogSelectMode::Choose);
    condition->accept(visitor);
    auto state = visitor.getState();
    if (state.has_value()) {
      _dialogManager.getStates().push_back(state.value());
    }
  }
}

bool DialogVisitor::acceptConditions(const Ast::Statement &statement) {
  ConditionVisitor conditionVisitor(*this);
  for (auto &condition : statement.conditions) {
    condition->accept(conditionVisitor);
    if (!conditionVisitor.isAccepted())
      break;
  }
  if (conditionVisitor.isAccepted()) {
    for (auto &condition : statement.conditions) {
      ConditionStateVisitor visitor(_dialogManager, DialogSelectMode::Show);
      condition->accept(visitor);
      auto state = visitor.getState();
      if (state.has_value()) {
        _dialogManager.getStates().push_back(state.value());
      }
    }
  }
  return conditionVisitor.isAccepted();
}

void DialogVisitor::visit(const Ast::Say &node) {
  auto &actors = _pEngine->getActors();
  Actor *pActor = nullptr;
  if (node.actor == "agent") {
    pActor = _pEngine->getCurrentActor();
  } else {
    auto name = node.actor;
    for (auto &actor : actors) {
      if (actor->getKey() == name) {
        pActor = actor.get();
        break;
      }
    }
  }
  auto id = getId(node.text);
  if (id > 0) {
    auto say = std::make_unique<_SayFunction>(*pActor, node.text);
    _dialogManager.addFunction(std::move(say));
    return;
  }

  auto anim = node.text.substr(2, node.text.length() - 3);
  std::stringstream s;
  s << "actorPlayAnimation(" << node.actor << ", \"" << anim << "\", NO)";
  auto executeCode = std::make_unique<_ExecuteCodeFunction>(*_pEngine, s.str());
  _dialogManager.addFunction(std::move(executeCode));
}

void DialogVisitor::visit(const Ast::Choice &node) {
  if (_dialogManager.getDialog()[node.number - 1].id != 0)
    return;

  auto text = node.text;
  if (text[0] == '$') {
    text = _pEngine->executeDollar(text);
  }
  auto id = getId(text);
  _dialogManager.getDialog()[node.number - 1].id = id;
  _dialogManager.getDialog()[node.number - 1].text = _pEngine->getText(id);
  _dialogManager.getDialog()[node.number - 1].label = node.gotoExp->name;
  _dialogManager.getDialog()[node.number - 1].pChoice = _pStatement;
  _dialogManager.getDialog()[node.number - 1].pos = {0, 0};
}

void DialogVisitor::visit(const Ast::Code &node) {
  auto executeCode = std::make_unique<_ExecuteCodeFunction>(*_pEngine, node.code);
  _dialogManager.addFunction(std::move(executeCode));
}

void DialogVisitor::visit(const Ast::Goto &node) {
  _dialogManager.selectLabel(node.name);
}

void DialogVisitor::visit(const Ast::Shutup &) {
  auto actor = _pEngine->getCurrentActor();
  if (actor) {
    auto shutup = std::make_unique<_ShutupFunction>(*_pEngine);
    _dialogManager.addFunction(std::move(shutup));
  }
}

void DialogVisitor::visit(const Ast::Pause &node) {
  auto pause = std::make_unique<_PauseFunction>(sf::seconds(node.time));
  _dialogManager.addFunction(std::move(pause));
}

void DialogVisitor::visit(const Ast::WaitFor &) {
  // TODO: waitfor
  trace("TODO: waitfor");
}

void DialogVisitor::visit(const Ast::Override &node) {
  _dialogManager.setOverride(node.node);
}

void DialogVisitor::visit(const Ast::Parrot &node) {
  _dialogManager.enableParrotMode(node.active);
}

void DialogVisitor::visit(const Ast::Dialog &node) {
  _dialogManager.setActorName(node.actor);
}

void DialogVisitor::visit(const Ast::AllowObjects &) {
  // TODO: allowObjects
  trace("TODO: allowObjects");
}

void DialogVisitor::visit(const Ast::WaitWhile &node) {
  auto waitWhile = std::make_unique<_WaitWhileFunction>(*_pEngine, node.condition);
  _dialogManager.addFunction(std::move(waitWhile));
}

void DialogVisitor::visit(const Ast::Limit &node) {
  _dialogManager.setLimit(node.max);
}

int DialogVisitor::getId(const std::string &text) {
  std::string s(text);
  if (s[0] == '@') {
    s = s.substr(1);

    auto id = std::strtol(s.c_str(), nullptr, 10);
    return id;
  }
  return -1;
}
} // namespace ng
