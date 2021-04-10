#include <ngf/IO/Json/JsonParser.h>
#include "engge/Engine/Preferences.hpp"

namespace ng {
Preferences::Preferences() {
  m_values = ngf::Json::load("Prefs.json");
}

void Preferences::save() {
  std::ofstream os;
  os.open("Prefs.json");
  for (auto &&pref : m_values.items()) {
    os << pref.key() << ": ";
    if (pref.value().isString()) {
      os << "\"" << pref.value().getString() << "\"";
    } else if (pref.value().isDouble()) {
      os << pref.value().getDouble();
    } else if (pref.value().isInteger()) {
      os << pref.value().getInt();
    }
    os << std::endl;
  }
}

void Preferences::removeUserPreference(const std::string &name) {
  auto value = m_values[name];
  if (!value.isNull()) {
    // TODO: removeUserPreference
    //_values.erase(it);
  }
}

void Preferences::subscribe(const std::function<void(const std::string &)> &function) {
  m_functions.emplace_back(function);
}

ngf::GGPackValue Preferences::getUserPreferenceCore(const std::string &name,
                                                    const ngf::GGPackValue &defaultValue) const {
  auto value = m_values[name];
  if (!value.isNull()) {
    return value;
  }
  return defaultValue;
}

ngf::GGPackValue Preferences::getTempPreferenceCore(const std::string &name,
                                                    const ngf::GGPackValue &defaultValue) const {
  auto value = m_tempValues[name];
  if (!value.isNull()) {
    return value;
  }
  return defaultValue;
}

template<>
int Preferences::fromGGPackValue<int>(const ngf::GGPackValue &value) {
  return value.getInt();
}

template<>
bool Preferences::fromGGPackValue<bool>(const ngf::GGPackValue &value) {
  return value.getInt() != 0;
}

template<>
std::string Preferences::fromGGPackValue<std::string>(const ngf::GGPackValue &value) {
  return value.getString();
}

template<>
float Preferences::fromGGPackValue<float>(const ngf::GGPackValue &value) {
  return static_cast<float>(value.getDouble());
}

template<>
ngf::GGPackValue Preferences::fromGGPackValue<ngf::GGPackValue>(const ngf::GGPackValue &value) {
  return value;
}

template<>
ngf::GGPackValue Preferences::toGGPackValue<ngf::GGPackValue>(ngf::GGPackValue value) {
  return value;
}

template<>
ngf::GGPackValue Preferences::toGGPackValue<int>(int value) {
  return value;
}

template<>
ngf::GGPackValue Preferences::toGGPackValue<bool>(bool value) {
  return value ? 1 : 0;
}

template<>
ngf::GGPackValue Preferences::toGGPackValue<float>(float value) {
  return value;
}

template<>
ngf::GGPackValue Preferences::toGGPackValue<std::string>(std::string value) {
  return value;
}

template<>
void Preferences::setUserPreference(const std::string &name, bool value) {
  setUserPreference(name, value ? 1 : 0);
}

} // namespace ng
