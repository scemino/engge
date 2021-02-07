#include "PreferencesTools.hpp"
#include <engge/Engine/Engine.hpp>
#include <engge/Engine/Preferences.hpp>
#include <string>
#include <imgui.h>

namespace ng {
PreferencesTools::PreferencesTools(Engine &engine) : m_engine(engine) {}

void PreferencesTools::render() {
  if (!ImGui::CollapsingHeader("Preferences"))
    return;

  auto selectedLang = getSelectedLang();
  if (ImGui::Combo("Language", &selectedLang, langs, 5)) {
    setSelectedLang(selectedLang);
  }
  auto retroVerbs = m_engine.getPreferences().getUserPreference(PreferenceNames::RetroVerbs,
                                                               PreferenceDefaultValues::RetroVerbs) != 0;
  if (ImGui::Checkbox("Retro Verbs", &retroVerbs)) {
    m_engine.getPreferences().setUserPreference(PreferenceNames::RetroVerbs, retroVerbs ? 1 : 0);
  }
  auto retroFonts = m_engine.getPreferences().getUserPreference(PreferenceNames::RetroFonts,
                                                               PreferenceDefaultValues::RetroFonts);
  if (ImGui::Checkbox("Retro Fonts", &retroFonts)) {
    m_engine.getPreferences().setUserPreference(PreferenceNames::RetroFonts, retroFonts ? 1 : 0);
  }
  auto invertVerbHighlight =
      m_engine.getPreferences().getUserPreference(PreferenceNames::InvertVerbHighlight,
                                                 PreferenceDefaultValues::InvertVerbHighlight);
  if (ImGui::Checkbox("Invert Verb Highlight", &invertVerbHighlight)) {
    m_engine.getPreferences().setUserPreference(PreferenceNames::InvertVerbHighlight, invertVerbHighlight ? 1 : 0);
  }
  auto hudSentence = m_engine.getPreferences().getUserPreference(PreferenceNames::HudSentence,
                                                                PreferenceDefaultValues::HudSentence);
  if (ImGui::Checkbox("HUD Sentence", &hudSentence)) {
    m_engine.getPreferences().setUserPreference(PreferenceNames::HudSentence, hudSentence ? 1 : 0);
  }
  auto uiBackingAlpha = m_engine.getPreferences().getUserPreference(PreferenceNames::UiBackingAlpha,
                                                                   PreferenceDefaultValues::UiBackingAlpha) * 100.f;
  if (ImGui::SliderFloat("UI Backing Alpha", &uiBackingAlpha, 0.f, 100.f)) {
    m_engine.getPreferences().setUserPreference(PreferenceNames::UiBackingAlpha, uiBackingAlpha * 0.01f);
  }
}

int PreferencesTools::getSelectedLang() {
  auto lang =
      m_engine.getPreferences().getUserPreference(PreferenceNames::Language, PreferenceDefaultValues::Language);
  for (size_t i = 0; i < 5; ++i) {
    if (!strcmp(lang.c_str(), langs[i]))
      return i;
  }
  return 0;
}

void PreferencesTools::setSelectedLang(int lang) {
  m_engine.getPreferences().setUserPreference(PreferenceNames::Language, std::string(langs[lang]));
}
}