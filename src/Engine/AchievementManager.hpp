#pragma once
#include <filesystem>
#include <string>
#include <ngf/IO/GGPackValue.h>
#include <engge/System/Logger.hpp>

namespace ng {
class AchievementManager final {
public:
  void load(const std::filesystem::path &path);
  void save(const std::filesystem::path &path);

  template<typename T>
  void setPrivatePreference(const std::string &name, T value) {
    trace("setPrivatePreference({},{})", name, value);
    m_value[name] = value;
  }

  [[nodiscard]] ngf::GGPackValue getPrivatePreference(const std::string &name) const;

private:
  ngf::GGPackValue m_value;
};
}
