#pragma once
#include <string>
#include <fstream>
#include <map>
#include <any>
#include "JsonTokenReader.h"

namespace ng
{
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
