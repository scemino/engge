#include "Engine/Preferences.hpp"

namespace ng
{
Preferences::Preferences()
{
    ng::Json::Parser::load("Prefs.json", _values);
}

void Preferences::save()
{
    std::ofstream os;
    os.open("Prefs.json");
    for (auto &&pref : _values.hash_value)
    {
        os << pref.first << ": ";
        if (pref.second.isString())
        {
            os << "\"" << pref.second.string_value << "\"";
        }
        else if (pref.second.isDouble())
        {
            os << pref.second.double_value;
        }
        else if (pref.second.isInteger())
        {
            os << pref.second.int_value;
        }
        os << std::endl;
    }
}

void Preferences::removeUserPreference(const std::string &name)
{
    auto it = _values.hash_value.find(name);
    if (it != _values.hash_value.end())
    {
        _values.hash_value.erase(it);
    }
}

void Preferences::removePrivatePreference(const std::string &name)
{
    auto it = _privateValues.hash_value.find(name);
    if (it != _privateValues.hash_value.end())
    {
        _privateValues.hash_value.erase(it);
    }
}

void Preferences::subscribe(std::function<void(const std::string &)> function)
{
    _functions.emplace_back(function);
}

GGPackValue Preferences::getUserPreferenceCore(const std::string &name, GGPackValue defaultValue) const
{
    auto it = _values.hash_value.find(name);
    if (it != _values.hash_value.end())
    {
        return it->second;
    }
    return defaultValue;
}

GGPackValue Preferences::getPrivatePreferenceCore(const std::string &name, GGPackValue defaultValue) const
{
    auto it = _privateValues.hash_value.find(name);
    if (it != _privateValues.hash_value.end())
    {
        return it->second;
    }
    return defaultValue;
}

template <>
int Preferences::fromGGPackValue<int>(GGPackValue value)
{
    return value.int_value;
}

template <>
bool Preferences::fromGGPackValue<bool>(GGPackValue value)
{
    return value.int_value?true:false;
}

template <>
std::string Preferences::fromGGPackValue<std::string>(GGPackValue value)
{
    return value.string_value;
}

template <>
float Preferences::fromGGPackValue<float>(GGPackValue value)
{
    return value.double_value;
}

template <>
GGPackValue Preferences::fromGGPackValue<GGPackValue>(GGPackValue value)
{
    return value;
}

template <>
GGPackValue Preferences::toGGPackValue<GGPackValue>(GGPackValue value)
{
    return value;
}

template <>
GGPackValue Preferences::toGGPackValue<int>(int value)
{
    GGPackValue packValue;
    packValue.type = 5;
    packValue.int_value = value;
    return packValue;
}

template <>
GGPackValue Preferences::toGGPackValue<bool>(bool value)
{
    GGPackValue packValue;
    packValue.type = 5;
    packValue.int_value = value?1:0;
    return packValue;
}

template <>
GGPackValue Preferences::toGGPackValue<float>(float value)
{
    GGPackValue packValue;
    packValue.type = 6;
    packValue.double_value = value;
    return packValue;
}

template <>
GGPackValue Preferences::toGGPackValue<std::string>(std::string value)
{
    GGPackValue packValue;
    packValue.type = 4;
    packValue.string_value = std::move(value);
    return packValue;
}

} // namespace ng
