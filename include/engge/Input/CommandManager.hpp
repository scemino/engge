#pragma once
#include <functional>
#include <string>
#include <initializer_list>
#include <unordered_map>
#include "InputConstants.hpp"

namespace ng {
class CommandManager {
public:
  using CommandHandler = std::function<void()>;
  using PressedCommandHandler = std::function<void(bool)>;

  void registerCommand(const std::string &command, CommandHandler handler);
  void registerPressedCommand(const std::string &command, PressedCommandHandler handler);
  void registerCommands(std::initializer_list<std::tuple<const char *, CommandHandler>> commands);
  void registerInputBindings(std::initializer_list<std::tuple<Input, const char *>> bindings);
  void registerInputBinding(const Input& input, const std::string& command);

  void execute(const std::string &command) const;
  void execute(const Input& input) const;
  void execute(const Input& input, bool keyDown) const;

private:
  std::unordered_map<std::string, CommandHandler> _commandHandlers;
  std::unordered_map<std::string, PressedCommandHandler> _pressedCommandHandlers;
  std::unordered_map<Input, std::string, InputHash> _inputBindings;
};
}
