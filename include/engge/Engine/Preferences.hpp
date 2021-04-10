#pragma once
#include <functional>
#include <map>
#include <string>
#include <ngf/IO/GGPackValue.h>

namespace ng {
namespace PreferenceNames {
static const std::string HudSentence = "hudSentence";
static const std::string UiBackingAlpha = "uiBackingAlpha";
static const std::string InvertVerbHighlight = "invertVerbHighlight";
static const std::string RetroVerbs = "retroVerbs";
static const std::string RetroFonts = "retroFonts";
static const std::string Language = "language";
static const std::string ClassicSentence = "hudSentence";
static const std::string Controller = "controller";
static const std::string ScrollSyncCursor = "controllerScollLockCursor";
static const std::string DisplayText = "talkiesShowText";
static const std::string HearVoice = "talkiesHearVoice";
static const std::string SayLineSpeed = "sayLineSpeed";
static const std::string SayLineBaseTime = "sayLineBaseTime";
static const std::string SayLineCharTime = "sayLineCharTime";
static const std::string SayLineMinTime = "sayLineMinTime";
static const std::string ToiletPaperOver = "toiletPaperOver";
static const std::string AnnoyingInJokes = "annoyingInJokes";
static const std::string SafeArea = "safeScale";
static const std::string Fullscreen = "windowFullscreen";
static const std::string RightClickSkipsDialog = "rightClickSkipsDialog";
static const std::string KeySkipText = "keySkipText";
static const std::string KeySelect1 = "keySelect1";
static const std::string KeySelect2 = "keySelect2";
static const std::string KeySelect3 = "keySelect3";
static const std::string KeySelect4 = "keySelect4";
static const std::string KeySelect5 = "keySelect5";
static const std::string KeySelect6 = "keySelect6";
static const std::string KeySelectPrev = "keySelectPrev";
static const std::string KeySelectNext = "keySelectNext";
// engge only
static const std::string EnggeGameSpeedFactor = "gameSpeedFactor";
static const std::string EnggeDevPath = "devPath";
static const bool EnggeDebug = false;
}

namespace PreferenceDefaultValues {
static const bool HudSentence = false;
static const float UiBackingAlpha = 0.33f;
static const bool InvertVerbHighlight = true;
static const bool RetroVerbs = false;
static const bool RetroFonts = false;
static const std::string Language = "en";
static const bool ClassicSentence = false;
static const bool Controller = false;
static const bool ScrollSyncCursor = true;
static const bool DisplayText = true;
static const bool HearVoice = true;
static const float SayLineSpeed = 0.5f;
static const float SayLineBaseTime = 1.5f;
static const float SayLineCharTime = 0.025f;
static const float SayLineMinTime = 0.2f;
static const bool ToiletPaperOver = true;
static const float SafeArea = 1.f;
static const bool Fullscreen = true;
static const bool RightClickSkipsDialog = false;
static const std::string KeySkipText = ".";
static const std::string KeySelect1 = "1";
static const std::string KeySelect2 = "2";
static const std::string KeySelect3 = "3";
static const std::string KeySelect4 = "4";
static const std::string KeySelect5 = "5";
static const std::string KeySelect6 = "6";
static const std::string KeySelectPrev = "9";
static const std::string KeySelectNext = "0";
static const bool AnnoyingInJokes = false;
static const std::string EnggeDevPath = "";
static const float EnggeGameSpeedFactor = 1.f;
static const bool EnggeDebug = false;
}

namespace TempPreferenceNames {
static const std::string ForceTalkieText = "forceTalkieText";
static const std::string ShowHotspot = "showHotspot";
}

namespace TempPreferenceDefaultValues {
static const int ForceTalkieText = 0;
static const int ShowHotspot = 0;
}

class Preferences {
public:
  Preferences();

  void save();

  template<typename T>
  void setTempPreference(const std::string &name, T value);
  template<typename T>
  T getTempPreference(const std::string &name, T value) const;

  template<typename T>
  void setUserPreference(const std::string &name, T value);
  template<typename T>
  T getUserPreference(const std::string &name, T value) const;

  void removeUserPreference(const std::string &name);

  void subscribe(const std::function<void(const std::string &)> &function);

  template<typename T>
  static ngf::GGPackValue toGGPackValue(T value);

  template<typename T>
  static T fromGGPackValue(const ngf::GGPackValue &value);

private:
  [[nodiscard]] ngf::GGPackValue getUserPreferenceCore(const std::string &name,
                                                       const ngf::GGPackValue &defaultValue) const;
  [[nodiscard]] ngf::GGPackValue getTempPreferenceCore(const std::string &name,
                                                       const ngf::GGPackValue &defaultValue) const;

private:
  ngf::GGPackValue m_values;
  ngf::GGPackValue m_tempValues;
  std::vector<std::function<void(const std::string &)>> m_functions;
};

template<typename T>
void Preferences::setUserPreference(const std::string &name, T value) {
  m_values[name] = value;
  for (auto &&func : m_functions) {
    func(name);
  }
}

template<>
void Preferences::setUserPreference(const std::string &name, bool value);

template<typename T>
T Preferences::getUserPreference(const std::string &name, T value) const {
  return Preferences::fromGGPackValue<T>(getUserPreferenceCore(name, Preferences::toGGPackValue<T>(value)));
}

template<typename T>
void Preferences::setTempPreference(const std::string &name, T value) {
  m_tempValues[name] = toGGPackValue(value);
}

template<typename T>
T Preferences::getTempPreference(const std::string &name, T value) const {
  return Preferences::fromGGPackValue<T>(getTempPreferenceCore(name, Preferences::toGGPackValue<T>(value)));
}

} // namespace ng
