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
}
namespace PreferenceDefaultValues
{
    static const bool HudSentence = false;
    static const float UiBackingAlpha = 0.33f;
    static const bool InvertVerbHighlight = true;
    static const bool RetroVerbs = false;
    static const bool RetroFonts = false;
    static const std::string Language = "en";
}
class Preferences
{
public:
    Preferences();

    void save();

    void setUserPreference(const std::string &name, std::any value);
    std::any getUserPreferenceCore(const std::string &name, std::any value) const;
    template <typename T>
    T getUserPreference(const std::string &name, T value) const;
    void removeUserPreference(const std::string &name);

    void subscribe(std::function<void(const std::string&,std::any)> function);

private:
    std::map<std::string, std::any> _values;
    std::vector<std::function<void(const std::string&,std::any)>> _functions;
};
template <typename T>
T Preferences::getUserPreference(const std::string &name, T value) const
{
    return std::any_cast<T>(getUserPreferenceCore(name, value));
}
} // namespace ng
