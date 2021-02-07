#include "ConsoleTools.hpp"
#include <engge/Scripting/ScriptEngine.hpp>

namespace ng {
ConsoleTools::ConsoleTools(Engine &engine) : m_console(engine) {
  ScriptEngine::registerErrorCallback([this](HSQUIRRELVM, const SQChar *s) {
    m_console.AddLog("[error] %s", s);
  });
  ScriptEngine::registerPrintCallback([this](HSQUIRRELVM, const SQChar *s) {
    m_console.AddLog("%s", s);
  });
}

void ConsoleTools::render() {
  if (!consoleVisible)
    return;
  m_console.Draw("Console", &consoleVisible);
}
}