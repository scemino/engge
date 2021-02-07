#pragma once

namespace ng {
class Engine;

class PreferencesTools final {
public:
  explicit PreferencesTools(Engine &engine);

  void render();

private:
  int getSelectedLang();
  void setSelectedLang(int lang);

private:
  Engine &m_engine;
  inline static const char *langs[] = {"en", "fr", "de", "es", "it"};
};
}