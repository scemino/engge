#pragma once
#include <string>
#include <vector>

namespace ng {
class Engine;

class GeneralTools final {
public:
  explicit GeneralTools(Engine &engine, bool &textureVisible, bool &consoleVisible, bool &showGlobalsTable,
                        bool &soundsVisible, bool & threadsVisible, bool & actorsVisible, bool &objectsVisible);

  void render();

private:
  static void getStack(std::vector<std::string> &stack);

private:
  Engine &m_engine;
  int m_selectedStack{0};
  bool &m_textureVisible;
  bool &m_consoleVisible;
  bool &m_showGlobalsTable;
  bool& m_soundsVisible;
  bool& m_threadsVisible;
  bool& m_actorsVisible;
  bool& m_objectsVisible;
};
}