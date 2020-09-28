#include "engge/System/Logger.hpp"
#include "engge/Input/CommandManager.hpp"

namespace ng {
void CommandManager::registerCommand(const std::string &command, CommandHandler handler) {
  _commandHandlers[command] = std::move(handler);
}

void CommandManager::registerPressedCommand(const std::string &command, PressedCommandHandler handler) {
  _pressedCommandHandlers[command] = std::move(handler);
}

void CommandManager::registerCommands(std::initializer_list<std::tuple<const char *, CommandHandler>> commands) {
  for (auto&[cmd, handler] : commands) {
    registerCommand(cmd, handler);
  }
}

void CommandManager::registerInputBinding(const Input& input, const std::string &command) {
  _inputBindings[input] = command;
}

void CommandManager::registerInputBindings(std::initializer_list<std::tuple<Input, const char *>> bindings){
  for (auto&[input, cmd] : bindings) {
    registerInputBinding(input, cmd);
  }
}

void CommandManager::execute(const std::string &command) const {
  trace("Execute command {}", command);
  auto it = _commandHandlers.find(command);
  if (it != _commandHandlers.end()) {
    it->second();
  }
}

void CommandManager::execute(const Input& input) const {
  auto it = _inputBindings.find(input);
  if (it != _inputBindings.end()) {
    execute(it->second);
  }
}

void CommandManager::execute(const Input& input, bool keyDown) const {
  auto it = _inputBindings.find(input);
  if (it != _inputBindings.end()) {
    auto it2 = _pressedCommandHandlers.find(it->second);
    if (it2 != _pressedCommandHandlers.end()) {
      it2->second(keyDown);
    }
  }
}
}