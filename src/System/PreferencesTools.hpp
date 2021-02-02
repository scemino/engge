#pragma once
#include <ngf/Graphics/ImGuiExtensions.h>
#include <imgui.h>

namespace ng {
class PreferencesTools final {
public:
  explicit PreferencesTools(Engine &engine) : _engine(engine) {}

  void render() {
    if (!ImGui::CollapsingHeader("Preferences"))
      return;

    auto selectedLang = getSelectedLang();
    if (ImGui::Combo("Language", &selectedLang, _langs, 5)) {
      setSelectedLang(selectedLang);
    }
    auto retroVerbs = _engine.getPreferences().getUserPreference(PreferenceNames::RetroVerbs,
                                                                 PreferenceDefaultValues::RetroVerbs) != 0;
    if (ImGui::Checkbox("Retro Verbs", &retroVerbs)) {
      _engine.getPreferences().setUserPreference(PreferenceNames::RetroVerbs, retroVerbs ? 1 : 0);
    }
    auto retroFonts = _engine.getPreferences().getUserPreference(PreferenceNames::RetroFonts,
                                                                 PreferenceDefaultValues::RetroFonts);
    if (ImGui::Checkbox("Retro Fonts", &retroFonts)) {
      _engine.getPreferences().setUserPreference(PreferenceNames::RetroFonts, retroFonts ? 1 : 0);
    }
    auto invertVerbHighlight =
        _engine.getPreferences().getUserPreference(PreferenceNames::InvertVerbHighlight,
                                                   PreferenceDefaultValues::InvertVerbHighlight);
    if (ImGui::Checkbox("Invert Verb Highlight", &invertVerbHighlight)) {
      _engine.getPreferences().setUserPreference(PreferenceNames::InvertVerbHighlight, invertVerbHighlight ? 1 : 0);
    }
    auto hudSentence = _engine.getPreferences().getUserPreference(PreferenceNames::HudSentence,
                                                                  PreferenceDefaultValues::HudSentence);
    if (ImGui::Checkbox("HUD Sentence", &hudSentence)) {
      _engine.getPreferences().setUserPreference(PreferenceNames::HudSentence, hudSentence ? 1 : 0);
    }
    auto uiBackingAlpha = _engine.getPreferences().getUserPreference(PreferenceNames::UiBackingAlpha,
                                                                     PreferenceDefaultValues::UiBackingAlpha) * 100.f;
    if (ImGui::SliderFloat("UI Backing Alpha", &uiBackingAlpha, 0.f, 100.f)) {
      _engine.getPreferences().setUserPreference(PreferenceNames::UiBackingAlpha, uiBackingAlpha * 0.01f);
    }
  }

private:
  int getSelectedLang() {
    auto lang =
        _engine.getPreferences().getUserPreference(PreferenceNames::Language, PreferenceDefaultValues::Language);
    for (size_t i = 0; i < 5; ++i) {
      if (!strcmp(lang.c_str(), _langs[i]))
        return i;
    }
    return 0;
  }

  void setSelectedLang(int lang) {
    _engine.getPreferences().setUserPreference(PreferenceNames::Language, std::string(_langs[lang]));
  }

private:
  Engine &_engine;
  inline static const char *_langs[] = {"en", "fr", "de", "es", "it"};
};
}