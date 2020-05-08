#pragma once
#include <functional>
#include <map>
#include <string>
#include "Parsers/GGPack.hpp"
#include "Parsers/JsonTokenReader.hpp"

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
static const std::string GameSpeedFactor = "gameSpeedFactor"; // engge only
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
static const float GameSpeedFactor = 1.f;
static const bool AnnoyingInJokes = false;
}
class Preferences {
public:
  Preferences();

  void save();

  void setForceTalkieText(bool forceTalkieText) { _forceTalkieText = forceTalkieText; }
  [[nodiscard]] bool getForceTalkieText() const { return _forceTalkieText; }

  template<typename T>
  void setUserPreference(const std::string &name, T value);
  template<typename T>
  T getUserPreference(const std::string &name, T value) const;

  void removeUserPreference(const std::string &name);
  void removePrivatePreference(const std::string &name);

  template<typename T>
  void setPrivatePreference(const std::string &name, T value);
  template<typename T>
  T getPrivatePreference(const std::string &name, T value) const;

  void subscribe(const std::function<void(const std::string &)> &function);

  template<typename T>
  static GGPackValue toGGPackValue(T value);

  template<typename T>
  static T fromGGPackValue(const GGPackValue &value);

private:
  [[nodiscard]] GGPackValue getUserPreferenceCore(const std::string &name, const GGPackValue &defaultValue) const;
  [[nodiscard]] GGPackValue getPrivatePreferenceCore(const std::string &name, const GGPackValue &defaultValue) const;

private:
  GGPackValue _values;
  GGPackValue _privateValues;
  std::vector<std::function<void(const std::string &)>> _functions;
  bool _forceTalkieText{false};
};

template<typename T>
void Preferences::setUserPreference(const std::string &name, T value) {
  _values.hash_value[name] = toGGPackValue(value);
  for (auto &&func : _functions) {
    func(name);
  }
}

template<typename T>
void Preferences::setPrivatePreference(const std::string &name, T value) {
  _privateValues.hash_value[name] = toGGPackValue(value);
}

template<typename T>
T Preferences::getUserPreference(const std::string &name, T value) const {
  return Preferences::fromGGPackValue<T>(getUserPreferenceCore(name, Preferences::toGGPackValue<T>(value)));
}

template<typename T>
T Preferences::getPrivatePreference(const std::string &name, T value) const {
  return Preferences::fromGGPackValue<T>(getPrivatePreferenceCore(name, Preferences::toGGPackValue<T>(value)));
}

} // namespace ng
