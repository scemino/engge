#include "Preferences.h"

namespace ng
{
Preferences::Preferences()
{
    ng::Json::Parser parser;
    ng::GGPackValue hash;
    parser.parse("Prefs.json", hash);

    for (auto &&pref : hash.hash_value)
    {
        std::any prefValue;
        if (pref.second.isDouble())
        {
            prefValue = pref.second.double_value;
        }
        else if (pref.second.isString())
        {
            auto value = pref.second.string_value;
            if (!value.empty() && value[0] == '\"' && value[value.size() - 1] == '\"')
            {
                value = value.substr(1, value.size() - 2);
            }
            prefValue = value;
        }
        setUserPreference(pref.first, prefValue);
    }
}

void Preferences::save()
{
    std::ofstream os;
    os.open("Prefs.json");
    for (auto &&pref : _values)
    {
        os << pref.first << ": ";
        if (pref.second.type() == typeid(std::string))
        {
            os << "\"" << std::any_cast<std::string>(pref.second) << "\"";
        }
        else if (pref.second.type() == typeid(double))
        {
            os << std::any_cast<double>(pref.second);
        }
        else if (pref.second.type() == typeid(int))
        {
            os << std::any_cast<int>(pref.second);
        }
        else if (pref.second.type() == typeid(bool))
        {
            os << (std::any_cast<bool>(pref.second) ? 1 : 0);
        }
        os << std::endl;
    }
}

void Preferences::setUserPreference(const std::string &name, std::any value)
{
    _values[name] = value;
    for (auto &&func : _functions)
    {
        func(name, value);
    }
}

void Preferences::removeUserPreference(const std::string &name)
{
    auto it = _values.find(name);
    if (it != _values.end())
    {
        _values.erase(it);
    }
}

void Preferences::removePrivatePreference(const std::string &name)
{
    auto it = _privateValues.find(name);
    if (it != _privateValues.end())
    {
        _privateValues.erase(it);
    }
}

void Preferences::setPrivatePreference(const std::string &name, std::any value)
{
    _privateValues[name] = value;
}

std::any Preferences::getUserPreferenceCore(const std::string &name, std::any value) const
{
    auto it = _values.find(name);
    if (it == _values.end())
    {
        return value;
    }
    return it->second;
}

std::any Preferences::getPrivatePreferenceCore(const std::string &name, std::any value) const
{
    auto it = _privateValues.find(name);
    if (it == _privateValues.end())
    {
        return value;
    }
    return it->second;
}

void Preferences::subscribe(std::function<void(const std::string &, std::any)> function)
{
    _functions.emplace_back(function);
}
} // namespace ng
