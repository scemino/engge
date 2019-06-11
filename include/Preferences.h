#pragma once
#include <string>
#include <map>
#include <any>
#include "JsonTokenReader.h"

namespace ng
{
class Preferences
{
public:
    Preferences()
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

    void setUserPreference(const std::string &name, std::any value)
    {
        _values[name] = value;
    }

    void removeUserPreference(const std::string &name)
    {
        auto it = _values.find(name);
        if (it != _values.end())
        {
            _values.erase(it);
        }
    }

    std::any getUserPreference(const std::string &name, std::any value) const
    {
        auto it = _values.find(name);
        if (it == _values.end())
        {
            return value;
        }
        return it->second;
    }
    
private:
    std::map<std::string, std::any> _values;
};
} // namespace ng
