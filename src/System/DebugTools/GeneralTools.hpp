#pragma once
#include <string>
#include <vector>

namespace ng {
class Engine;

class GeneralTools final {
public:
  explicit GeneralTools(Engine &engine, bool &textureVisible, bool &consoleVisible, bool &showGlobalsTable);

  void render();

private:
  void getStack(std::vector<std::string> &stack);

private:
  Engine &m_engine;
  int m_selectedStack{0};
  bool &m_textureVisible;
  bool &m_consoleVisible;
  bool &m_showGlobalsTable;
  int m_fadeEffect{0};
  float m_fadeDuration{3.f};
};
}