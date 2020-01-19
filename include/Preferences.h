#pragma once
#include <any>
#include <functional>
#include <map>
#include <string>
#include "JsonTokenReader.h"

namespace ng
{
namespace PreferenceNames
{
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
    static const std::string TextSpeed = "sayLineSpeed";
    static const std::string ToiletPaperOver = "toiletPaperOver";
    static const std::string SafeArea = "safeScale";
    static const std::string Fullscreen = "windowFullscreen";
}

namespace PreferenceDefaultValues
{
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
    static const float TextSpeed = 1.f;
    static const bool ToiletPaperOver = true;
    static const float SafeArea = 1.f;
    static const bool Fullscreen = true;
}
class Preferences
{
public:
    Preferences();

    void save();

    void setUserPreference(const std::string &name, std::any value);
    std::any getUserPreferenceCore(const std::string &name, std::any value) const;
    std::any getPrivatePreferenceCore(const std::string &name, std::any value) const;
    template <typename T>
    T getUserPreference(const std::string &name, T value) const;
    void removeUserPreference(const std::string &name);
    void removePrivatePreference(const std::string &name);

    void setPrivatePreference(const std::string &name, std::any value);
    template <typename T>
    T getPrivatePreference(const std::string &name, T value) const;

    void subscribe(std::function<void(const std::string&,std::any)> function);

private:
    std::map<std::string, std::any> _values;
    std::map<std::string, std::any> _privateValues;
    std::vector<std::function<void(const std::string&,std::any)>> _functions;
};

template <typename T>
T Preferences::getUserPreference(const std::string &name, T value) const
{
    return std::any_cast<T>(getUserPreferenceCore(name, value));
}

template <typename T>
T Preferences::getPrivatePreference(const std::string &name, T value) const
{
    return std::any_cast<T>(getPrivatePreferenceCore(name, value));
}
} // namespace ng
