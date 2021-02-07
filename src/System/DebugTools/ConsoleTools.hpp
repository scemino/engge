#pragma once
#include "Console.hpp"

namespace ng {
class Engine;

class ConsoleTools final {
public:
  explicit ConsoleTools(Engine &engine);
  void render();

public:
  bool consoleVisible{false};

private:
  Console m_console;
};
}