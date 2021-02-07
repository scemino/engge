#pragma once
#include <imgui.h>

namespace ng {
class Engine;

struct Console {
  char InputBuf[256];
  ImVector<char *> Items;
  ImVector<const char *> Commands;
  ImVector<char *> History;
  int HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
  ImGuiTextFilter Filter;
  bool AutoScroll;
  bool ScrollToBottom;
  Engine &_engine;

  Console(Engine &engine);
  ~Console();

  // Portable helpers
  static int Stricmp(const char *str1, const char *str2);
  static int Strnicmp(const char *str1, const char *str2, int n);
  static char *Strdup(const char *str);
  static void Strtrim(char *str);

  void ClearLog();
  void PrintVar(const char *var);
  void DumpActors();
  void AddLog(const char *fmt, ...) IM_FMTARGS(2);
  void Draw(const char *title, bool *p_open);
  void ExecCommand(const char *command_line);
  static int TextEditCallbackStub(ImGuiInputTextCallbackData *data); // In C++11 you are better off using lambdas for this sort of forwarding callbacks
  int TextEditCallback(ImGuiInputTextCallbackData *data);
};
}