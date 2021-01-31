#include "engge/Engine/Preferences.hpp"

namespace ng {
Preferences::Preferences() {
  ng::Json::Parser::load("Prefs.json", _values);
}

void Preferences::save() {
  std::ofstream os;
  os.open("Prefs.json");
  for (auto &&pref : _values.hash_value) {
    os << pref.first << ": ";
    if (pref.second.isString()) {
      os << "\"" << pref.second.string_value << "\"";
    } else if (pref.second.isDouble()) {
      os << pref.second.double_value;
    } else if (pref.second.isInteger()) {
      os << pref.second.int_value;
    }
    os << std::endl;
  }
}

void Preferences::removeUserPreference(const std::string &name) {
  auto it = _values.hash_value.find(name);
  if (it != _values.hash_value.end()) {
    _values.hash_value.erase(it);
  }
}

void Preferences::subscribe(const std::function<void(const std::string &)> &function) {
  _functions.emplace_back(function);
}

GGPackValue Preferences::getUserPreferenceCore(const std::string &name, const GGPackValue &defaultValue) const {
  auto it = _values.hash_value.find(name);
  if (it != _values.hash_value.end()) {
    return it->second;
  }
  return defaultValue;
}

GGPackValue Preferences::getTempPreferenceCore(const std::string &name, const GGPackValue &defaultValue) const {
  auto it = _tempValues.hash_value.find(name);
  if (it != _tempValues.hash_value.end()) {
    return it->second;
  }
  return defaultValue;
}

template<>
int Preferences::fromGGPackValue<int>(const GGPackValue &value) {
  return value.int_value;
}

template<>
bool Preferences::fromGGPackValue<bool>(const GGPackValue &value) {
  if(value.isDouble())
    return value.double_value != 0;
  return value.int_value != 0;
}

template<>
std::string Preferences::fromGGPackValue<std::string>(const GGPackValue &value) {
  return value.string_value;
}

template<>
float Preferences::fromGGPackValue<float>(const GGPackValue &value) {
  return value.double_value;
}

template<>
GGPackValue Preferences::fromGGPackValue<GGPackValue>(const GGPackValue &value) {
  return value;
}

template<>
GGPackValue Preferences::toGGPackValue<GGPackValue>(GGPackValue value) {
  return value;
}

template<>
GGPackValue Preferences::toGGPackValue<int>(int value) {
  GGPackValue packValue;
  packValue.type = 5;
  packValue.int_value = value;
  return packValue;
}

template<>
GGPackValue Preferences::toGGPackValue<bool>(bool value) {
  GGPackValue packValue;
  packValue.type = 5;
  packValue.int_value = value ? 1 : 0;
  return packValue;
}

template<>
GGPackValue Preferences::toGGPackValue<float>(float value) {
  GGPackValue packValue;
  packValue.type = 6;
  packValue.double_value = value;
  return packValue;
}

template<>
GGPackValue Preferences::toGGPackValue<std::string>(std::string value) {
  GGPackValue packValue;
  packValue.type = 4;
  packValue.string_value = std::move(value);
  return packValue;
}

} // namespace ng
