#pragma once
#include "Engine/Function.hpp"
#include "Dialog/DialogManager.hpp"

namespace ng
{
class _GotoFunction : public Function
{
public:
  explicit _GotoFunction(DialogVisitor &dialogVisitor, const std::string& name)
      : _dialogVisitor(dialogVisitor), _name(name)
  {
  }

  bool isElapsed() override { return _done; }

  void operator()(const sf::Time &) override
  {
    if (_done)
      return;
    _dialogVisitor.getDialogManager().selectLabel(_name);
    auto& dialog = _dialogVisitor.getDialogManager().getDialog();
    _dialogVisitor.setHasChoices(std::any_of(dialog.begin(),dialog.end(),[](auto& line){ return line.id!=0;}));
    _done = true;
  }

private:
  DialogVisitor &_dialogVisitor;
  std::string _name;
  bool _done{false};
};
} // namespace ng
