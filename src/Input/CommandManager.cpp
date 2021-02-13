#include "engge/System/Logger.hpp"
#include "engge/Input/CommandManager.hpp"

namespace ng {
void CommandManager::registerCommand(const std::string &command, CommandHandler handler) {
  m_commandHandlers[command] = std::move(handler);
}

void CommandManager::registerPressedCommand(const std::string &command, PressedCommandHandler handler) {
  m_pressedCommandHandlers[command] = std::move(handler);
}

void CommandManager::registerCommands(std::initializer_list<std::tuple<const char *, CommandHandler>> commands) {
  for (auto&[cmd, handler] : commands) {
    registerCommand(cmd, handler);
  }
}

void CommandManager::registerInputBinding(const Input& input, const std::string &command) {
  m_inputBindings[input] = command;
}

void CommandManager::registerInputBindings(std::initializer_list<std::tuple<Input, const char *>> bindings){
  for (auto&[input, cmd] : bindings) {
    registerInputBinding(input, cmd);
  }
}

void CommandManager::execute(const std::string &command) const {
  trace("Execute command {}", command);
  auto it = m_commandHandlers.find(command);
  if (it != m_commandHandlers.end()) {
    it->second();
  }
}

void CommandManager::execute(const Input& input) const {
  auto it = m_inputBindings.find(input);
  if (it != m_inputBindings.end()) {
    execute(it->second);
  }
}

void CommandManager::execute(const Input& input, bool keyDown) const {
  auto it = m_inputBindings.find(input);
  if (it != m_inputBindings.end()) {
    auto it2 = m_pressedCommandHandlers.find(it->second);
    if (it2 != m_pressedCommandHandlers.end()) {
      it2->second(keyDown);
    }
  }
}
}